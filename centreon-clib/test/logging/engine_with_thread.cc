/*
** Copyright 2011-2013 Merethis
**
** This file is part of Centreon Clib.
**
** Centreon Clib is free software: you can redistribute it
** and/or modify it under the terms of the GNU Affero General Public
** License as published by the Free Software Foundation, either version
** 3 of the License, or (at your option) any later version.
**
** Centreon Clib is distributed in the hope that it will be
** useful, but WITHOUT ANY WARRANTY; without even the implied warranty
** of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** Affero General Public License for more details.
**
** You should have received a copy of the GNU Affero General Public
** License along with Centreon Clib. If not, see
** <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <memory>
#include <sstream>
#include "com/centreon/concurrency/locker.hh"
#include "com/centreon/concurrency/mutex.hh"
#include "com/centreon/concurrency/thread.hh"
#include "com/centreon/exceptions/basic.hh"
#include "com/centreon/logging/engine.hh"
#include "./backend_test.hh"

using namespace com::centreon::concurrency;
using namespace com::centreon::logging;

/**
 *  @class writter
 *  @brief little implementation of thread to test logging engine.
 */
class                writter : public thread {
public:
                     writter(unsigned int nb_write)
                       : _nb_write(nb_write) {}
                     ~writter() throw () {}
private:
  void               _run() {
    engine& e(engine::instance());
    for (unsigned int i(0); i < _nb_write; ++i) {
      std::ostringstream oss;
      oss << this << ":" << i;
      e.log(1, 0, oss.str().c_str(), oss.str().size());
    }
  }
  unsigned int       _nb_write;
};

/**
 *  Check add backend on to the logging engine.
 *
 *  @return 0 on success.
 */
int main() {
  static unsigned int const nb_writter(10);
  static unsigned int const nb_write(10);
  int retval;

  engine::load();
  try {
    engine& e(engine::instance());

    std::auto_ptr<backend_test> obj(new backend_test);
    e.add(obj.get(), 1, 0);

    std::vector<thread*> threads;
    for (unsigned int i(0); i < nb_writter; ++i)
      threads.push_back(new writter(nb_write));

    for (unsigned int i(0); i < nb_writter; ++i)
      threads[i]->exec();

    for (unsigned int i(0); i < nb_writter; ++i)
      threads[i]->wait();

    for (unsigned int i(0); i < nb_writter; ++i) {
      for (unsigned int j(0); j < nb_writter; ++j) {
        std::ostringstream oss;
        oss <<&threads[i] << ":" << j << "\n";
        if (!obj->data().find(oss.str()))
          throw (basic_error() << "pattern not found");
      }
      delete threads[i];
    }
    retval = 0;
  }
  catch (std::exception const& e) {
    std::cerr << "error: " << e.what() << std::endl;
    retval = 1;
  }
  engine::unload();
  return (retval);
}
