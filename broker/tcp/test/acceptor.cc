/**
 * Copyright 2011 - 2019 Centreon (https://www.centreon.com/)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For more information : contact@centreon.com
 *
 */

#include <fmt/format.h>
#include <gtest/gtest.h>

#include "com/centreon/broker/tcp/acceptor.hh"

#include <nlohmann/json.hpp>

#include "com/centreon/broker/io/raw.hh"
#include "com/centreon/broker/log_v2.hh"
#include "com/centreon/broker/pool.hh"
#include "com/centreon/broker/tcp/connector.hh"
#include "com/centreon/broker/tcp/tcp_async.hh"
#include "com/centreon/exceptions/msg_fmt.hh"

using namespace com::centreon::broker;
using namespace com::centreon::exceptions;

extern std::shared_ptr<asio::io_context> g_io_context;

const static std::string test_addr("127.0.0.1");
constexpr static uint16_t test_port(4444);
static tcp::tcp_config::pointer test_conf(
    std::make_shared<tcp::tcp_config>(test_addr, test_port));
static tcp::tcp_config::pointer test_conf2(
    std::make_shared<tcp::tcp_config>(test_addr, 4141));

class TcpAcceptor : public ::testing::Test {
 public:
  void SetUp() override {
    log_v2::tcp()->set_level(spdlog::level::debug);
    g_io_context->restart();
    pool::load(g_io_context, 0);
    tcp::tcp_async::load();
  }

  void TearDown() override {
    log_v2::tcp()->info("TCP TearDown");
    tcp::tcp_async::instance().stop_timer();
    tcp::tcp_async::unload();
    pool::unload();
  }
};

static auto try_connect =
    [](tcp::connector& con) -> std::shared_ptr<io::stream> {
  std::shared_ptr<io::stream> u;
  while (!u) {
    try {
      u = con.open();
    } catch (...) {
    }
  }
  return u;
};

TEST_F(TcpAcceptor, BadPort) {
  if (getuid() != 0) {
    tcp::tcp_config::pointer conf(std::make_shared<tcp::tcp_config>("", 2));
    tcp::acceptor acc(conf);
    ASSERT_THROW(acc.open(), std::exception);
  }
}

TEST_F(TcpAcceptor, NoConnector) {
  tcp::acceptor acc(test_conf);

  ASSERT_EQ(acc.open(), std::shared_ptr<io::stream>());
}

TEST_F(TcpAcceptor, Nominal) {
  std::thread cbd([] {
    std::unique_ptr<tcp::acceptor> a(
        std::make_unique<tcp::acceptor>(test_conf2));
    std::unique_ptr<io::endpoint> endp(a.release());

    /* Nominal case, cbd is acceptor and read on the socket */
    std::shared_ptr<io::stream> u_cbd;
    do {
      u_cbd = endp->open();
    } while (!u_cbd);

    std::shared_ptr<io::data> data_read;
    while (!data_read ||
           std::static_pointer_cast<io::raw>(data_read)->size() < 10000)
      ASSERT_NO_THROW(u_cbd->read(data_read, static_cast<time_t>(-1)));

    std::vector<char> vec(
        std::static_pointer_cast<io::raw>(data_read)->get_buffer());
    std::string result(vec.begin(), vec.end());
    char cc = 'A';
    std::string wanted;
    for (int i = 0; i < 10000; i++) {
      wanted += cc;
      if (++cc == 'z') {
        wanted += "\n";
        cc = 'A';
      }
    }
    ASSERT_EQ(wanted, result);
  });

  std::thread centengine([] {
    std::unique_ptr<tcp::connector> c(new tcp::connector(test_conf2));
    std::unique_ptr<io::endpoint> endp(c.release());

    /* Nominal case, centengine is connector and write on the socket */
    std::shared_ptr<io::stream> u_centengine;
    do {
      u_centengine = endp->open();
    } while (!u_centengine);

    std::shared_ptr<io::raw> data_write{new io::raw()};
    std::string cc("A");
    for (int i = 0; i < 10000; i++) {
      data_write->append(cc);
      if (++(cc[0]) == 'z') {
        data_write->append(std::string("\n"));
        cc = "A";
      }
    }
    u_centengine->write(data_write);
    int retry = 10;
    while (retry-- && u_centengine->flush() == 0)
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
  });

  centengine.join();
  cbd.join();
}

TEST_F(TcpAcceptor, QuestionAnswer) {
  constexpr int rep = 100;
  std::mutex cbd_m;
  std::condition_variable cbd_cv;
  bool cbd_finished = false;

  std::thread cbd([&cbd_m, &cbd_cv, &cbd_finished] {
    std::unique_ptr<io::endpoint> endp(
        std::make_unique<tcp::acceptor>(test_conf2));

    /* Nominal case, cbd is acceptor and read on the socket */
    std::shared_ptr<io::stream> u_cbd;
    do {
      u_cbd = endp->open();
    } while (!u_cbd);

    std::shared_ptr<io::data> data_read;
    std::shared_ptr<io::raw> data_write;
    bool val;
    for (int i = 0; i < rep; i++) {
      val = false;
      std::string wanted(fmt::format("Question{}", i));
      while (!val || !data_read ||
             std::static_pointer_cast<io::raw>(data_read)->size() <
                 wanted.size()) {
        val = u_cbd->read(data_read, static_cast<time_t>(0));
      }

      std::vector<char> vec(
          std::static_pointer_cast<io::raw>(data_read)->get_buffer());
      std::string result(vec.begin(), vec.end());

      ASSERT_EQ(wanted, result);

      data_write = std::make_shared<io::raw>();
      std::string text(fmt::format("Answer{}", i));
      std::string cc("a");
      for (auto c : text) {
        cc[0] = c;
        data_write->append(cc);
      }
      u_cbd->write(data_write);
    }
    int retry = 10;
    while (retry-- && u_cbd->flush() == 0)
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::unique_lock<std::mutex> lock(cbd_m);
    cbd_cv.wait(lock, [&cbd_finished] { return cbd_finished; });
  });

  std::thread centengine([&cbd_cv, &cbd_finished] {
    std::unique_ptr<tcp::connector> c(new tcp::connector(test_conf2));
    std::unique_ptr<io::endpoint> endp(c.release());

    std::shared_ptr<io::stream> u_centengine;
    do {
      u_centengine = endp->open();
    } while (!u_centengine);

    std::shared_ptr<io::data> data_read;
    std::shared_ptr<io::raw> data_write;
    bool val;
    for (int i = 0; i < rep; i++) {
      data_write = std::make_shared<io::raw>();
      std::string text(fmt::format("Question{}", i));
      std::string cc("a");
      for (auto c : text) {
        cc[0] = c;
        data_write->append(cc);
      }
      u_centengine->write(data_write);

      val = false;
      std::string wanted(fmt::format("Answer{}", i));
      while (!val || !data_read ||
             std::static_pointer_cast<io::raw>(data_read)->size() <
                 wanted.size())
        val = u_centengine->read(data_read, static_cast<time_t>(0));

      std::vector<char> vec(
          std::static_pointer_cast<io::raw>(data_read)->get_buffer());
      std::string result(vec.begin(), vec.end());

      ASSERT_EQ(wanted, result);
    }
    cbd_finished = true;
    cbd_cv.notify_all();
  });

  centengine.join();
  cbd.join();
}

TEST_F(TcpAcceptor, MultiNominal) {
  constexpr size_t nb_poller(10);
  std::mutex cbd_m;
  std::unique_lock<std::mutex> lock(cbd_m);
  std::condition_variable cbd_cv;
  bool cbd_finished = false;

  std::thread cbd([nb_poller, &cbd_finished, &cbd_m, &cbd_cv] {
    char cc = 'A';
    std::string wanted;
    for (int i = 0; i < 10000; i++) {
      wanted += cc;
      if (++cc == 'z') {
        wanted += "\n";
        cc = 'A';
      }
    }

    std::vector<std::string> data(nb_poller);
    {
      std::vector<std::shared_ptr<io::stream>> u_cbd(nb_poller);
      std::unique_ptr<io::endpoint> endp{
          std::make_unique<tcp::acceptor>(test_conf2)};

      /* Nominal case, cbd is acceptor and read on the socket */
      bool cont = true;

      while (cont) {
        std::cout << "Open stream\n";
        auto u(endp->open());
        if (u) {
          std::cout << "Open stream done\n";
          for (auto it = u_cbd.begin(); it != u_cbd.end(); ++it) {
            if (!*it) {
              *it = std::move(u);
              break;
            }
          }
        }
        std::cout << "before reading\n";
        cont = false;
        for (size_t i = 0; i < nb_poller; i++) {
          std::cout << "READING i = " << i << "\n";
          auto& u = u_cbd[i];
          if (u) {
            std::shared_ptr<io::data> d;
            /* The read function does not wait... */
            u->read(d, static_cast<time_t>(0));
            std::vector<char> vec(
                std::static_pointer_cast<io::raw>(d)->get_buffer());
            data[i].insert(data[i].end(), vec.begin(), vec.end());
            if (data[i].size() < wanted.size()) {
              std::cout << "not finished to fill stream...\n";
              cont = true;
            }
          } else {
            std::cout << "not finished...\n";
            cont = true;
            break;
          }
        }
        std::cout << "after reading, cont = " << cont << "\n";
      }

      std::cout << "Checking...\n";
      ASSERT_EQ(u_cbd.size(), nb_poller);
    }

    for (auto d : data)
      ASSERT_EQ(wanted, d);

    std::lock_guard<std::mutex> lock(cbd_m);
    cbd_finished = true;
    cbd_cv.notify_all();
  });

  std::vector<std::thread> pollers;

  for (size_t i = 0; i < nb_poller; i++) {
    pollers.emplace_back([&cbd_finished, &cbd_m, &cbd_cv] {
      std::unique_ptr<io::endpoint> endp{
          std::make_unique<tcp::connector>(test_conf2)};

      /* Nominal case, centengine is connector and write on the socket */
      std::shared_ptr<io::stream> u_centengine;
      do {
        u_centengine = endp->open();
      } while (!u_centengine);

      std::shared_ptr<io::raw> data_write{new io::raw()};
      std::string cc("A");
      for (int i = 0; i < 10000; i++) {
        data_write->append(cc);
        if (++(cc[0]) == 'z') {
          data_write->append(std::string("\n"));
          cc = "A";
        }
      }
      u_centengine->write(data_write);
      int retry = 10;
      while (retry-- && u_centengine->flush() == 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      std::unique_lock<std::mutex> lock(cbd_m);
      cbd_cv.wait(lock, [&cbd_finished] { return cbd_finished; });
    });
  }

  cbd_cv.wait(lock, [&cbd_finished] { return cbd_finished; });
  lock.unlock();

  std::cout << "Stopping pollers\n";
  for (auto& p : pollers) {
    p.join();
    std::cout << "Done.\n";
  }
  std::cout << "All pollers stopped.\n";

  cbd.join();
}

TEST_F(TcpAcceptor, NominalReversed) {
  std::mutex cbd_m;
  std::condition_variable cbd_cv;
  bool cbd_finished = false;

  std::thread centengine([&cbd_m, &cbd_cv, &cbd_finished] {
    std::unique_ptr<tcp::connector> c(new tcp::connector(test_conf2));
    std::unique_ptr<io::endpoint> endp(c.release());

    std::shared_ptr<io::stream> u_centengine;
    do {
      try {
        u_centengine = endp->open();
      } catch (const std::exception& e) {
        std::cout << '.';
      }
    } while (!u_centengine);
    std::cout << '\n';

    std::shared_ptr<io::raw> data_write{new io::raw()};
    std::string cc("A");
    for (int i = 0; i < 10000; i++) {
      data_write->append(cc);
      if (++(cc[0]) == 'z') {
        data_write->append(std::string("\n"));
        cc = "A";
      }
    }
    u_centengine->write(data_write);
    int retry = 10;
    while (retry-- && u_centengine->flush() == 0)
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::unique_lock<std::mutex> lck(cbd_m);
    cbd_cv.wait(lck, [&cbd_finished] { return cbd_finished; });
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  std::thread cbd([&cbd_m, &cbd_finished, &cbd_cv] {
    std::unique_ptr<io::endpoint> endp(
        std::make_unique<tcp::acceptor>(test_conf2));

    std::shared_ptr<io::stream> u_cbd;
    do {
      u_cbd = endp->open();
    } while (!u_cbd);

    std::shared_ptr<io::data> data_read;
    while (!data_read ||
           std::static_pointer_cast<io::raw>(data_read)->size() < 10000)
      u_cbd->read(data_read, static_cast<time_t>(-1));

    std::vector<char> vec(
        std::static_pointer_cast<io::raw>(data_read)->get_buffer());
    std::string result(vec.begin(), vec.end());
    char cc = 'A';
    std::string wanted;
    for (int i = 0; i < 10000; i++) {
      wanted += cc;
      if (++cc == 'z') {
        wanted += "\n";
        cc = 'A';
      }
    }
    ASSERT_EQ(wanted, result);
    std::lock_guard<std::mutex> lck(cbd_m);
    cbd_finished = true;
    cbd_cv.notify_all();
  });

  cbd.join();
  centengine.join();
}

TEST_F(TcpAcceptor, OnePeer) {
  std::thread centengine([] {
    std::unique_ptr<tcp::acceptor> a(
        std::make_unique<tcp::acceptor>(test_conf2));
    std::unique_ptr<io::endpoint> endp(a.release());

    std::shared_ptr<io::stream> u_centengine;
    do {
      u_centengine = endp->open();
    } while (!u_centengine);

    std::shared_ptr<io::raw> data_write{new io::raw()};
    std::string cc("A");
    for (int i = 0; i < 10000; i++) {
      data_write->append(cc);
      if (++(cc[0]) == 'z') {
        data_write->append(std::string("\n"));
        cc = "A";
      }
    }
    u_centengine->write(data_write);
    int retry = 10;
    while (retry-- && u_centengine->flush() == 0)
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
  });

  std::thread cbd([] {
    std::unique_ptr<tcp::connector> c(new tcp::connector(test_conf2));
    std::unique_ptr<io::endpoint> endp(c.release());

    std::shared_ptr<io::stream> u_cbd;
    do {
      u_cbd = endp->open();
    } while (!u_cbd);

    std::shared_ptr<io::data> data_read;
    while (!data_read ||
           std::static_pointer_cast<io::raw>(data_read)->size() < 10000)
      u_cbd->read(data_read, static_cast<time_t>(-1));

    std::vector<char> vec(
        std::static_pointer_cast<io::raw>(data_read)->get_buffer());
    std::string result(vec.begin(), vec.end());
    char cc = 'A';
    std::string wanted;
    for (int i = 0; i < 10000; i++) {
      wanted += cc;
      if (++cc == 'z') {
        wanted += "\n";
        cc = 'A';
      }
    }
    ASSERT_EQ(wanted, result);
  });

  cbd.join();
  centengine.join();
}

TEST_F(TcpAcceptor, OnePeerReversed) {
  std::thread cbd([] {
    std::unique_ptr<tcp::connector> c(new tcp::connector(test_conf2));
    std::unique_ptr<io::endpoint> endp(c.release());

    std::shared_ptr<io::stream> u_cbd;
    do {
      try {
        u_cbd = endp->open();
      } catch (const std::exception& e) {
        std::cout << '.';
      }
    } while (!u_cbd);
    std::cout << '\n';

    std::shared_ptr<io::data> data_read;
    while (!data_read ||
           std::static_pointer_cast<io::raw>(data_read)->size() < 10000)
      u_cbd->read(data_read, static_cast<time_t>(-1));

    std::vector<char> vec(
        std::static_pointer_cast<io::raw>(data_read)->get_buffer());
    std::string result(vec.begin(), vec.end());
    char cc = 'A';
    std::string wanted;
    for (int i = 0; i < 10000; i++) {
      wanted += cc;
      if (++cc == 'z') {
        wanted += "\n";
        cc = 'A';
      }
    }
    ASSERT_EQ(wanted, result);
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  std::thread centengine([] {
    std::unique_ptr<tcp::acceptor> a(
        std::make_unique<tcp::acceptor>(test_conf2));
    std::unique_ptr<io::endpoint> endp(a.release());

    std::shared_ptr<io::stream> u_centengine;
    do {
      u_centengine = endp->open();
    } while (!u_centengine);

    std::shared_ptr<io::raw> data_write{new io::raw()};
    std::string cc("A");
    for (int i = 0; i < 10000; i++) {
      data_write->append(cc);
      if (++(cc[0]) == 'z') {
        data_write->append(std::string("\n"));
        cc = "A";
      }
    }
    u_centengine->write(data_write);
    int retry = 10;
    while (retry-- && u_centengine->flush() == 0)
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
  });

  centengine.join();
  cbd.join();
}

TEST_F(TcpAcceptor, MultiOnePeer) {
  const int nb_steps = 5;

  std::thread centengine([] {
    std::unique_ptr<tcp::acceptor> a(
        std::make_unique<tcp::acceptor>(test_conf2));
    std::unique_ptr<io::endpoint> endp(a.release());

    std::shared_ptr<io::stream> u_centengine;

    int i = 0;
    while (i < nb_steps) {
      if (u_centengine) {
        std::shared_ptr<io::data> data_read;
        while (!data_read ||
               std::static_pointer_cast<io::raw>(data_read)->size() == 0) {
          try {
            u_centengine->read(data_read, static_cast<time_t>(0));
          } catch (const std::exception& e) {
            u_centengine.reset();
          }
        }

        std::vector<char> vec(
            std::static_pointer_cast<io::raw>(data_read)->get_buffer());
        std::string result(vec.begin(), vec.end());
        ASSERT_EQ(vec.size(), 7u);
        ASSERT_EQ(result, "Hello1!");
        std::shared_ptr<io::raw> data_write = std::make_shared<io::raw>();
        data_write->append(std::string("Hello2!"));
        u_centengine->write(data_write);
        int retry = 10;
        while (retry-- && u_centengine->flush() == 0)
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
        u_centengine.reset();
        i++;
      } else
        u_centengine = endp->open();
    }
  });

  /* We start nb_steps instances of cbd one after the other. Each time, it
   * simulates a negotiation with the centengine instance */
  for (int i = 0; i < nb_steps; i++) {
    std::thread cbd([] {
      std::unique_ptr<tcp::connector> c(new tcp::connector(test_conf2));
      std::unique_ptr<io::endpoint> endp(c.release());

      std::shared_ptr<io::stream> u_cbd;
      do {
        u_cbd = endp->open();
      } while (!u_cbd);

      std::shared_ptr<io::raw> data_write = std::make_shared<io::raw>();
      data_write->append(std::string("Hello1!"));
      ASSERT_NO_THROW(u_cbd->write(data_write));

      std::shared_ptr<io::data> data_read;
      while (!data_read ||
             std::static_pointer_cast<io::raw>(data_read)->size() == 0) {
        ASSERT_NO_THROW(u_cbd->read(data_read, static_cast<time_t>(-1)));
      }

      std::vector<char> vec(
          std::static_pointer_cast<io::raw>(data_read)->get_buffer());
      std::string result(vec.begin(), vec.end());
      ASSERT_EQ(result, "Hello2!");
    });

    cbd.join();
  }
  centengine.join();
}

TEST_F(TcpAcceptor, NominalRepeated) {
  const int nb_steps = 5;

  std::thread centengine([] {
    std::unique_ptr<tcp::connector> c(new tcp::connector(test_conf2));
    std::unique_ptr<io::endpoint> endp(c.release());

    std::shared_ptr<io::stream> u_centengine;

    int i = 0;
    do {
      u_centengine = endp->open();
    } while (!u_centengine);

    while (i < nb_steps) {
      std::cout << "engine 0 " << i << "\n";
      std::shared_ptr<io::data> data_read;
      while (!data_read ||
             std::static_pointer_cast<io::raw>(data_read)->size() == 0) {
        std::cout << "engine 1 " << i << "\n";
        try {
          u_centengine->read(data_read, static_cast<time_t>(-1));
        } catch (const std::exception& e) {
          do {
            u_centengine = endp->open();
          } while (!u_centengine);
        }
      }
      std::cout << "engine 2 " << i << "\n";

      std::vector<char> vec(
          std::static_pointer_cast<io::raw>(data_read)->get_buffer());
      std::string result(vec.begin(), vec.end());
      ASSERT_EQ(result, "Hello1!");
      std::cout << "engine 3 " << i << "\n";
      std::shared_ptr<io::raw> data_write = std::make_shared<io::raw>();
      data_write->append(std::string("Hello2!"));
      u_centengine->write(data_write);
      std::cout << "engine 4 " << i << "\n";
      i++;
    }
    int retry = 10;
    while (retry-- && u_centengine->flush() == 0)
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
  });

  /* We start nb_steps instances of cbd one after the other. Each time, it
   * simulates a negotiation with the centengine instance */
  for (int i = 0; i < nb_steps; i++) {
    std::thread cbd([i] {
      std::cout << "cbd  " << i << "\n";
      std::unique_ptr<tcp::acceptor> a(
          std::make_unique<tcp::acceptor>(test_conf2));
      std::cout << "cbd1 " << i << "\n";
      std::cout << "cbd2 " << i << "\n";
      std::cout << "cbd3 " << i << "\n";
      std::unique_ptr<io::endpoint> endp(a.release());
      std::cout << "cbd4 " << i << "\n";

      std::shared_ptr<io::stream> u_cbd;
      do {
        u_cbd = endp->open();
      } while (!u_cbd);

      std::cout << "cbd5 " << i << "\n";
      std::shared_ptr<io::raw> data_write = std::make_shared<io::raw>();
      data_write->append(std::string("Hello1!"));
      ASSERT_NO_THROW(u_cbd->write(data_write));

      std::shared_ptr<io::data> data_read;
      while (!data_read ||
             std::static_pointer_cast<io::raw>(data_read)->size() == 0) {
        try {
          u_cbd->read(data_read, static_cast<time_t>(-1));
        } catch (const std::exception& e) {
          std::cout << "EXCEPTION DURING READING: " << e.what() << "\n";
          break;
        }
      }

      std::vector<char> vec(
          std::static_pointer_cast<io::raw>(data_read)->get_buffer());
      std::string result(vec.begin(), vec.end());
      ASSERT_EQ(result, "Hello2!");
    });

    cbd.join();
  }
  centengine.join();
}

TEST_F(TcpAcceptor, Wait2Connect) {
  tcp::acceptor acc(test_conf2);
  int i = 0;
  std::shared_ptr<io::stream> st;

  std::thread t{[&] {
    std::this_thread::sleep_for(std::chrono::milliseconds{2050});
    tcp::connector con(test_conf2);
    std::shared_ptr<io::stream> str{try_connect(con)};
  }};

  while (!st) {
    std::cout << "TRY " << i << std::endl;
    try {
      st = acc.open();
    } catch (std::exception const& e) {
      std::cout << std::this_thread::get_id() << "EXCEPTION: " << e.what()
                << std::endl;
    }
    i++;
  }
  t.join();
  ASSERT_GT(i, 0);
}

TEST_F(TcpAcceptor, Simple) {
  tcp::acceptor acc(test_conf);
  std::condition_variable cv;
  std::mutex m;
  bool finish = false;

  std::thread t([&] {
    tcp::connector con(test_conf);
    std::shared_ptr<io::stream> str{try_connect(con)};
    std::shared_ptr<io::raw> data{std::make_shared<io::raw>()};
    std::shared_ptr<io::data> data_read;
    data->append(std::string("TEST\n"));
    str->write(data);
    int retry = 10;
    while (retry-- && str->flush() == 0)
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::unique_lock<std::mutex> lock(m);
    cv.wait(lock, [&finish] { return finish; });
  });

  {
    std::lock_guard<std::mutex> lock(m);
    std::shared_ptr<io::stream> io;
    for (;;) {
      io = acc.open();
      if (io)
        break;
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    std::shared_ptr<io::raw> data{std::make_shared<io::raw>()};
    std::shared_ptr<io::data> data_read;

    while (!io->read(data_read, -1)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      std::cout << std::this_thread::get_id() << "loop... ";
    }

    std::vector<char> vec{
        std::static_pointer_cast<io::raw>(data_read)->get_buffer()};
    std::string str{vec.begin(), vec.end()};
    ASSERT_TRUE(str == "TEST\n");
    data->append(std::string("TEST\n"));
    io->write(data);
  }
  finish = true;
  cv.notify_one();
  t.join();
}

TEST_F(TcpAcceptor, Multiple) {
  tcp::acceptor acc(test_conf);

  {
    std::thread t{[] {
      tcp::connector con(test_conf);
      std::shared_ptr<io::stream> str{try_connect(con)};
      std::shared_ptr<io::raw> data{new io::raw()};
      std::shared_ptr<io::data> data_read;
      data->append(std::string("TEST\n"));
      str->write(data);
      str->read(data_read, -1);
    }};
    std::shared_ptr<io::stream> io;
    for (;;) {
      io = acc.open();
      if (io)
        break;
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    std::shared_ptr<io::raw> data{new io::raw()};
    std::shared_ptr<io::data> data_read;
    io->read(data_read);

    std::vector<char> vec{
        std::static_pointer_cast<io::raw>(data_read)->get_buffer()};
    std::string str{vec.begin(), vec.end()};
    ASSERT_TRUE(str == "TEST\n");

    data->append(std::string("TEST\n"));
    io->write(data);

    t.join();
  }
  {
    std::thread t{[] {
      tcp::connector con(test_conf);
      std::shared_ptr<io::stream> str{try_connect(con)};
      std::shared_ptr<io::raw> data{new io::raw()};
      std::shared_ptr<io::data> data_read;
      data->append(std::string("TEST\n"));
      str->write(data);
      str->read(data_read, -1);
    }};
    std::shared_ptr<io::stream> io;
    for (;;) {
      io = acc.open();
      if (io)
        break;
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    std::shared_ptr<io::raw> data{new io::raw()};
    std::shared_ptr<io::data> data_read;
    io->read(data_read);

    std::vector<char> vec{
        std::static_pointer_cast<io::raw>(data_read)->get_buffer()};
    std::string str{vec.begin(), vec.end()};
    ASSERT_TRUE(str == "TEST\n");

    data->append(std::string("TEST\n"));
    io->write(data);

    t.join();
  }
}

TEST_F(TcpAcceptor, BigSend) {
  tcp::acceptor acc(test_conf);

  std::thread t{[] {
    tcp::connector con(test_conf);
    std::shared_ptr<io::stream> str{try_connect(con)};
    std::shared_ptr<io::raw> data{new io::raw()};
    std::shared_ptr<io::data> data_read;
    for (int i = 0; i < 1024; i++) {
      data->append(std::string("0123456789"));
    }
    data->append(std::string("01234"));
    str->write(data);
    str->read(data_read, -1);
  }};
  std::shared_ptr<io::stream> io;
  for (;;) {
    io = acc.open();
    if (io)
      break;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
  std::shared_ptr<io::raw> data{new io::raw()};
  std::shared_ptr<io::data> data_read;
  io->read(data_read, time(nullptr) + 5);

  std::vector<char> vec{
      std::static_pointer_cast<io::raw>(data_read)->get_buffer()};
  std::string str{vec.begin(), vec.end()};

  data->append(std::string("TEST\n"));
  io->write(data);

  ASSERT_TRUE(str.length() == 10245);

  t.join();
}

TEST_F(TcpAcceptor, CloseRead) {
  tcp::acceptor acc(test_conf);

  std::thread t{[&] {
    {
      tcp::connector con(test_conf);
      std::shared_ptr<io::stream> str{try_connect(con)};
      std::shared_ptr<io::raw> data{new io::raw()};
      std::shared_ptr<io::data> data_read;
      data->append(std::string("0"));
      str->write(data);
      int retry = 10;
      while (retry-- && str->flush() == 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }};
  std::shared_ptr<io::stream> io;
  for (;;) {
    io = acc.open();
    if (io)
      break;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
  std::shared_ptr<io::raw> data{new io::raw()};
  std::shared_ptr<io::data> data_read;

  t.join();
  for (;;) {
    try {
      io->read(data_read, -1);
    } catch (msg_fmt const& ex) {
      break;
    }
  }
}

TEST_F(TcpAcceptor, ChildsAndStats) {
  tcp::acceptor acc(test_conf);

  acc.add_child("child1");
  acc.add_child("child2");
  acc.add_child("child3");
  acc.remove_child("child2");

  nlohmann::json obj;
  acc.stats(obj);
  ASSERT_EQ(obj.dump(), "{\"peers\":\"2: child1, child3\"}");
}

TEST_F(TcpAcceptor, QuestionAnswerMultiple) {
  constexpr int nb_connections = 5;
  constexpr int rep = 100;
  std::vector<std::thread> cbd, centengine;

  for (int i = 0; i < nb_connections; i++) {
    cbd.emplace_back([i] {
      tcp::tcp_config::pointer conf(
          std::make_shared<tcp::tcp_config>("", 4141 + i));
      std::unique_ptr<tcp::acceptor> a(std::make_unique<tcp::acceptor>(conf));
      std::unique_ptr<io::endpoint> endp(a.release());

      /* Nominal case, cbd is acceptor and read on the socket */
      std::shared_ptr<io::stream> u_cbd;
      do {
        u_cbd = endp->open();
      } while (!u_cbd);

      std::shared_ptr<io::data> data_read;
      std::shared_ptr<io::raw> data_write;
      bool val;
      for (int i = 0; i < rep; i++) {
        val = false;
        std::string wanted(fmt::format("Question{}", i));
        while (!val || !data_read ||
               std::static_pointer_cast<io::raw>(data_read)->size() <
                   wanted.size()) {
          val = u_cbd->read(data_read, static_cast<time_t>(0));
        }

        std::vector<char> vec(
            std::static_pointer_cast<io::raw>(data_read)->get_buffer());
        std::string result(vec.begin(), vec.end());

        ASSERT_EQ(wanted, result);

        data_write = std::make_shared<io::raw>();
        std::string text(fmt::format("Answer{}", i));
        std::string cc("a");
        for (auto c : text) {
          cc[0] = c;
          data_write->append(cc);
        }
        u_cbd->write(data_write);
      }
      int retry = 10;
      while (retry-- && u_cbd->flush() == 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });

    centengine.emplace_back([i] {
      tcp::tcp_config::pointer conf(
          std::make_shared<tcp::tcp_config>("", 4141 + i));
      std::unique_ptr<tcp::connector> c(new tcp::connector(conf));
      std::unique_ptr<io::endpoint> endp(c.release());

      std::shared_ptr<io::stream> u_centengine;
      do {
        u_centengine = endp->open();
      } while (!u_centengine);

      std::shared_ptr<io::data> data_read;
      std::shared_ptr<io::raw> data_write;
      bool val;
      for (int i = 0; i < rep; i++) {
        data_write = std::make_shared<io::raw>();
        std::string text(fmt::format("Question{}", i));
        std::string cc("a");
        for (auto c : text) {
          cc[0] = c;
          data_write->append(cc);
        }
        u_centengine->write(data_write);

        val = false;
        std::string wanted(fmt::format("Answer{}", i));
        while (!val || !data_read ||
               std::static_pointer_cast<io::raw>(data_read)->size() <
                   wanted.size())
          val = u_centengine->read(data_read, static_cast<time_t>(0));

        std::vector<char> vec(
            std::static_pointer_cast<io::raw>(data_read)->get_buffer());
        std::string result(vec.begin(), vec.end());

        ASSERT_EQ(wanted, result);
      }
    });
  }

  for (int i = 0; i < nb_connections; i++) {
    centengine[i].join();
    cbd[i].join();
  }
}

TEST_F(TcpAcceptor, MultipleBigSend) {
  tcp::acceptor acc(test_conf);
  constexpr int32_t nb_packet = 10;
  constexpr int32_t len = 10024;

  std::thread t{[] {
    tcp::connector con(test_conf);
    std::shared_ptr<io::stream> str{try_connect(con)};
    std::shared_ptr<io::data> data_read;
    for (int k = 0; k < nb_packet; k++) {
      std::shared_ptr<io::raw> data = std::make_shared<io::raw>();
      for (int i = 0; i < len; i++) {
        std::string a(10, '0' + k);
        data->append(a);
      }
      str->write(data);
    }
    str->read(data_read, -1);
  }};
  std::shared_ptr<io::stream> io;
  for (;;) {
    io = acc.open();
    if (io)
      break;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
  std::shared_ptr<io::data> data;
  std::shared_ptr<io::raw> data_read = std::make_shared<io::raw>();

  do {
    io->read(data, time(nullptr) + 5);
    std::vector<char> vec{
        std::static_pointer_cast<io::raw>(data)->get_buffer()};
    data_read->append(vec);
  } while (data_read->size() < nb_packet * len * 10);
  std::string str{data_read->get_buffer().begin(),
                  data_read->get_buffer().end()};

  int k = -1;
  for (int i = 0; i < nb_packet * len * 10; i++) {
    if (i % (len * 10) == 0)
      k++;
    ASSERT_EQ(str[i], '0' + k);
  }
  ASSERT_TRUE(str.length() == nb_packet * len * 10);

  std::static_pointer_cast<io::raw>(data)->get_buffer().clear();
  std::static_pointer_cast<io::raw>(data)->append(std::string("TEST\n"));
  io->write(data);

  t.join();
}
