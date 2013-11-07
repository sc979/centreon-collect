/*
** Copyright 2013 Merethis
**
** This file is part of Centreon Broker.
**
** Centreon Broker is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License version 2
** as published by the Free Software Foundation.
**
** Centreon Broker is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Centreon Broker. If not, see
** <http://www.gnu.org/licenses/>.
*/

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <iterator>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <sstream>
#include <vector>
#include "com/centreon/broker/exceptions/msg.hh"
#include "test/cbd.hh"
#include "test/config.hh"
#include "test/engine.hh"
#include "test/external_command.hh"
#include "test/generate.hh"
#include "test/misc.hh"
#include "test/vars.hh"

using namespace com::centreon::broker;

#define DB_NAME "broker_correlation_to_sql"
#define HOST_COUNT 10
#define SERVICES_BY_HOST 2

/**
 *  Check that correlation is properly inserted in SQL database.
 *
 *  @return EXIT_SUCCESS on success.
 */
int main() {
  // Return value.
  int retval(EXIT_FAILURE);

  // Variables that need cleaning.
  std::list<host> hosts;
  std::list<service> services;
  std::string engine_config_path(tmpnam(NULL));
  std::string broker_config_path(tmpnam(NULL));
  std::string broker_correlation_path(tmpnam(NULL));
  std::string cbmod_config_path(tmpnam(NULL));
  std::string cbmod_correlation_path(tmpnam(NULL));
  external_command commander;
  engine daemon;
  cbd broker;

  // Information.
  std::cout << "base retention: " << cbmod_correlation_path << "\n"
            << "passive retention: " << broker_correlation_path << "\n";

  try {
    // Write cbmod configuration file.
    {
      std::ofstream ofs;
      ofs.open(
            cbmod_config_path.c_str(),
            std::ios_base::out | std::ios_base::trunc);
      if (ofs.fail())
        throw (exceptions::msg()
               << "cannot open cbmod configuration file '"
               << cbmod_config_path.c_str() << "'");
      ofs << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
        "<centreonbroker>\n"
        "  <include>" PROJECT_SOURCE_DIR "/test/cfg/broker_modules.xml</include>\n"
        "  <instance>42</instance>\n"
        "  <instance_name>MyBroker</instance_name>\n"
        "  <!--\n"
        "  <stats>\n"
        "    <fifo>/tmp/cbmod.fifo</fifo>\n"
        "  </stats>\n"
        "  <logger>\n"
        "    <type>file</type>\n"
        "    <name>cbmod.log</name>\n"
        "    <config>1</config>\n"
        "    <debug>1</debug>\n"
        "    <error>1</error>\n"
        "    <info>1</info>\n"
        "    <level>3</level>\n"
        "  </logger>\n"
        "  -->\n"
        "  <correlation>\n"
        "    <file>" PROJECT_SOURCE_DIR "/test/cfg/correlation_file.xml</file>\n"
        "    <retention>" << cbmod_correlation_path << "</retention>\n"
        "  </correlation>\n"
        "  <output>\n"
        "    <name>EngineToSQLUnitTest</name>\n"
        "    <type>sql</type>\n"
        "    <db_type>" DB_TYPE "</db_type>\n"
        "    <db_host>" DB_HOST "</db_host>\n"
        "    <db_port>" DB_PORT "</db_port>\n"
        "    <db_user>" DB_USER "</db_user>\n"
        "    <db_password>" DB_PASSWORD "</db_password>\n"
        "    <db_name>broker_correlation_to_sql</db_name>\n"
        "    <queries_per_transaction>0</queries_per_transaction>\n"
        "    <with_state_events>1</with_state_events>\n"
        "  </output>\n"
        "  <output>\n"
        "    <name>PassiveCorrelation</name>\n"
        "    <type>tcp</type>\n"
        "    <protocol>bbdo</protocol>\n"
        "    <read_timeout>1</read_timeout>\n"
        "    <port>5688</port>\n"
        "    <host>localhost</host>\n"
        "    <filters>\n"
        "      <category>correlation</category>\n"
        "    </filters>\n"
        "  </output>\n"
        "</centreonbroker>\n";
      ofs.close();
    }

    // Write cbd configuration file.
    {
      std::ofstream ofs;
      ofs.open(
            broker_config_path.c_str(),
            std::ios_base::out | std::ios_base::trunc);
      if (ofs.fail())
        throw (exceptions::msg()
               << "cannot open cbd configuration file '"
               << broker_config_path.c_str() << "'");
      ofs << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
        "<centreonbroker>\n"
        "  <include>" PROJECT_SOURCE_DIR "/test/cfg/broker_modules.xml</include>\n"
        "  <instance>24</instance>\n"
        "  <instance_name>MyBrokerPassive</instance_name>\n"
        "  <!--\n"
        "  <stats>\n"
        "    <fifo>/tmp/cbd.fifo</fifo>\n"
        "  </stats>\n"
        "  <logger>\n"
        "    <type>file</type>\n"
        "    <name>cbd.log</name>\n"
        "    <config>1</config>\n"
        "    <debug>1</debug>\n"
        "    <error>1</error>\n"
        "    <info>1</info>\n"
        "    <level>3</level>\n"
        "  </logger>\n"
        "  -->\n"
        "  <correlation>\n"
        "    <file>" PROJECT_SOURCE_DIR "/test/cfg/correlation_file.xml</file>\n"
        "    <retention>" << broker_correlation_path << "</retention>\n"
        "    <passive>1</passive>\n"
        "  </correlation>\n"
        "  <input>\n"
        "    <name>InputCorrelationPassiveModeUnitTest</name>\n"
        "    <type>tcp</type>\n"
        "    <port>5688</port>\n"
        "    <protocol>bbdo</protocol>\n"
        "  </input>\n"
        "</centreonbroker>\n";
      ofs.close();
    }

    // Prepare database.
    QSqlDatabase db(config_db_open(DB_NAME));

    // Prepare monitoring engine configuration parameters.
    generate_hosts(hosts, HOST_COUNT);
    for (std::list<host>::iterator it(hosts.begin()), end(hosts.end());
         it != end;
         ++it) {
      it->accept_passive_host_checks = 1;
      it->checks_enabled = 0;
      it->max_attempts = 1;
    }
    generate_services(services, hosts, SERVICES_BY_HOST);
    for (std::list<service>::iterator
           it(services.begin()),
           end(services.end());
         it != end;
         ++it) {
      it->accept_passive_service_checks = 1;
      it->checks_enabled = 0;
      it->max_attempts = 1;
    }
    commander.set_file(tmpnam(NULL));
    std::string additional_config;
    {
      std::ostringstream oss;
      oss << commander.get_engine_config()
          << "broker_module=" << CBMOD_PATH << " "
          << cbmod_config_path << "\n";
      additional_config = oss.str();
    }

    // Generate monitoring engine configuration files.
    config_write(
      engine_config_path.c_str(),
      additional_config.c_str(),
      &hosts,
      &services);

    // Start broker daemon.
    broker.set_config_file(broker_config_path);
    broker.start();
    sleep_for(2 * MONITORING_ENGINE_INTERVAL_LENGTH);

    // Start monitoring engine.
    std::string engine_config_file(engine_config_path);
    engine_config_file.append("/nagios.cfg");
    daemon.set_config_file(engine_config_file);
    daemon.start();

    // T1.
    time_t t1(time(NULL));

    // Let the daemon initialize.
    sleep_for(10 * MONITORING_ENGINE_INTERVAL_LENGTH);

    /* Test cases
    ** ----------
    **
    ** 1) UP -> DOWN
    ** 2) OK -> WARNING -> DOWNTIME
    ** 3) OK -> CRITICAL -> ACK (STICKY) -> WARNING
    ** 4) UP -> DOWNTIME -> UNREACHABLE -> ACK (STICKY) -> DOWN
    ** 5) OK -> CRITICAL -> ACK (STICKY).
    ** 6) OK -> WARNING -> ACK (NORMAL) -> DOWNTIME
    */

    // Step 1.
    {
      // Set hosts as OK.
      for (unsigned int i(1); i <= HOST_COUNT; ++i) {
        std::ostringstream cmd;
        cmd << "PROCESS_HOST_CHECK_RESULT;" << i << ";0;output1-" << i;
        commander.execute(cmd.str());
      }

      // Set services as OK.
      for (unsigned int i(0); i <= HOST_COUNT * SERVICES_BY_HOST; ++i) {
        unsigned int host_id((i / SERVICES_BY_HOST) + 1);
        unsigned int service_id(i + 1);
        std::ostringstream cmd;
        cmd << "PROCESS_SERVICE_CHECK_RESULT;" << host_id << ";"
            << service_id << ";0;output1-" << host_id << ";"
            << service_id;
        commander.execute(cmd.str());
      }
    }
    sleep_for(3 * MONITORING_ENGINE_INTERVAL_LENGTH);

    // T2.
    time_t t2(time(NULL));

    // Step 2.
    {
      commander.execute("PROCESS_HOST_CHECK_RESULT;1;2;output2-1");
      commander.execute("PROCESS_SERVICE_CHECK_RESULT;1;1;1;output2-1-1");
      commander.execute("PROCESS_SERVICE_CHECK_RESULT;1;2;2;output2-1-2");
      {
        std::ostringstream oss;
        oss << "SCHEDULE_HOST_DOWNTIME;2;" << t2 << ";" << (t2 + 3600)
            << ";1;0;3600;Merethis;Host #2 is going in downtime";
        commander.execute(oss.str());
      }
      commander.execute("PROCESS_SERVICE_CHECK_RESULT;2;3;2;output2-2-3");
      commander.execute("PROCESS_SERVICE_CHECK_RESULT;2;4;1;output2-2-4");
    }
    sleep_for(3 * MONITORING_ENGINE_INTERVAL_LENGTH);

    // T3.
    time_t t3(time(NULL));

    // Step 3.
    {
      {
        std::ostringstream oss;
        oss << "SCHEDULE_SVC_DOWNTIME;1;1;" << t3 << ";" << (t3 + 2000)
            << ";1;0;2000;Centreon;Service #1-#1 is going in downtime";
        commander.execute(oss.str());
      }
      commander.execute("ACKNOWLEDGE_SVC_PROBLEM;1;2;0;0;1;Broker;Ack SVC1-2");
      commander.execute("PROCESS_HOST_CHECK_RESULT;2;1;output3-2");
      commander.execute("ACKNOWLEDGE_SVC_PROBLEM;2;3;2;0;1;Engine;Ack SVC2-3");
      commander.execute("ACKNOWLEDGE_SVC_PROBLEM;2;4;0;0;1;foo;Ack SVC2-4");
    }
    sleep_for(3 * MONITORING_ENGINE_INTERVAL_LENGTH);

    // T4.
    time_t t4(time(NULL));

    // Step 4.
    {
      commander.execute("PROCESS_SVC_CHECK_RESULT;1;2;1;output4-1-2");
      commander.execute("ACKNOWLEDGE_HOST_PROBLEM;2;2;0;1;Centreon Map;Ack HST2");
      {
        std::ostringstream oss;
        oss << "SCHEDULE_SVC_DOWNTIME;2;4;" << t4 << ";" << (t4 + 1600)
            << ";0;0;1000;Merethis;Service #2-#4 is going in downtime";
        commander.execute(oss.str());
      }
    }
    sleep_for(3 * MONITORING_ENGINE_INTERVAL_LENGTH);

    // T5.
    time_t t5(time(NULL));

    // Step 5.
    {
      commander.execute("PROCESS_HOST_CHECK_RESULT;2;2;output5-2");
    }
    sleep_for(3 * MONITORING_ENGINE_INTERVAL_LENGTH);

    // Check host state events.
    {
      struct {
        unsigned int host_id;
        time_t       start_time_low;
        time_t       start_time_high;
        bool         end_time_is_null;
        time_t       end_time_low;
        time_t       end_time_high;
        short        state;
        bool         ack_time_is_null;
        time_t       ack_time_low;
        time_t       ack_time_high;
        bool         in_downtime;
      } const          entries[] = {
        /*
        ** Host 1.
        */
        // Start = PENDING.
        { 1, 0, 1, false, t1, t2, 4, true, 0, 0, false },
        // Step 1 = UP.
        { 1, t1, t2, false, t2, t3, 0, true, 0, 0, false },
        // Step 2 = UNREACHABLE.
        { 1, t2, t3, true, 0, 0, 2, true, 0, 0, false },

        /*
        ** Host 2.
        */
        // Start = PENDING.
        { 2, 0, 1, false, t1, t2, 4, true, 0, 0, false },
        // Step 1 = UP.
        { 2, t1, t2, false, t2, t3, 0, true, 0, 0, false },
        // Step 2 = DOWNTIME.
        { 2, t2, t3, false, t3, t4, 0, true, 0, 0, true },
        // Step 3 = DOWN, step 4 = ACK (STICKY).
        { 2, t3, t4, false, t4, t5, 1, false, t4, t5, true },
        // Step 5 = UNREACHABLE.
        { 2, t4, t5, true, 0, 0, 2, false, t4, t5, true },

        { 3, 0, 1, false, t1, t2, 4, true, 0, 0, false },
        { 3, t1, t2, true, 0, 0, 0, true, 0, 0, false },
        { 4, 0, 1, false, t1, t2, 4, true, 0, 0, false },
        { 4, t1, t2, true, 0, 0, 0, true, 0, 0, false },
        { 5, 0, 1, false, t1, t2, 4, true, 0, 0, false },
        { 5, t1, t2, true, 0, 0, 0, true, 0, 0, false },
        { 6, 0, 1, false, t1, t2, 4, true, 0, 0, false },
        { 6, t1, t2, true, 0, 0, 0, true, 0, 0, false },
        { 7, 0, 1, false, t1, t2, 4, true, 0, 0, false },
        { 7, t1, t2, true, 0, 0, 0, true, 0, 0, false },
        { 8, 0, 1, false, t1, t2, 4, true, 0, 0, false },
        { 8, t1, t2, true, 0, 0, 0, true, 0, 0, false },
        { 9, 0, 1, false, t1, t2, 4, true, 0, 0, false },
        { 9, t1, t2, true, 0, 0, 0, true, 0, 0, false },
        { 10, 0, 1, false, t1, t2, 4, true, 0, 0, false },
        { 10, t1, t2, true, 0, 0, 0, true, 0, 0, false }
      };

      // Get host state events.
      QSqlQuery q(db);
      if (!q.exec("SELECT host_id, start_time, end_time, state,"
                  "       ack_time, in_downtime"
                  " FROM hoststateevents"
                  " ORDER BY host_id, start_time"))
        throw (exceptions::msg() << "cannot get host state events: "
               << q.lastError().text());

      // Compare DB with expected content.
      for (unsigned int i(0);
           i < sizeof(entries) / sizeof(*entries);
           ++i) {
        // Get next entry.
        if (!q.next())
          throw (exceptions::msg()
                 << "not enough host state events in DB: got " << i
                 << ", expected "
                 << (sizeof(entries) / sizeof(*entries)));

        // Match entry.
        if ((q.value(0).toUInt() != entries[i].host_id)
            || (static_cast<time_t>(q.value(1).toLongLong())
                < entries[i].start_time_low)
            || (static_cast<time_t>(q.value(1).toLongLong())
                > entries[i].start_time_high)
            || (entries[i].end_time_is_null && !q.value(2).isNull())
            || (!entries[i].end_time_is_null
                && ((static_cast<time_t>(q.value(2).toLongLong())
                     < entries[i].end_time_low)
                    || (static_cast<time_t>(q.value(2).toLongLong())
                        > entries[i].end_time_high)))
            || (q.value(3).toInt() != entries[i].state)
            || (entries[i].ack_time_is_null && !q.value(4).isNull())
            || (!entries[i].ack_time_is_null
                && ((static_cast<time_t>(q.value(4).toLongLong())
                     < entries[i].ack_time_low)
                    || (static_cast<time_t>(q.value(4).toLongLong())
                        > entries[i].ack_time_high)))
            || (static_cast<bool>(q.value(5).toInt())
                != entries[i].in_downtime)) {
          exceptions::msg e;
          e << "invalid host state event entry #" << i
            << ": got (host id " << q.value(0).toUInt()
            << ", start time " << q.value(1).toUInt() << ", end time "
            << (q.value(2).isNull() ? "null" : q.value(2).toString())
            << ", state " << q.value(3).toInt() << ", ack time "
            << (q.value(4).isNull() ? "null" : q.value(4).toString())
            << ", in downtime " << q.value(5).toInt() << "), expected ("
            << entries[i].host_id << ", " << entries[i].start_time_low
            << ":" << entries[i].start_time_high << ", ";
          if (entries[i].end_time_is_null)
            e << "null";
          else
            e << entries[i].end_time_low << ":"
              << entries[i].end_time_high;
          e << ", " << entries[i].state << ", ";
          if (entries[i].ack_time_is_null)
            e << "null";
          else
            e << entries[i].ack_time_low << ":"
              << entries[i].ack_time_high;
          e << ", " << entries[i].in_downtime << ")";
          throw (e);
        }
      }

      // No more results.
      if (q.next())
        throw (exceptions::msg() << "too much host state events in DB");
    }

    // Check service state events.
    {
      struct {
        unsigned int host_id;
        unsigned int service_id;
        time_t       start_time_low;
        time_t       start_time_high;
        bool         end_time_is_null;
        time_t       end_time_low;
        time_t       end_time_high;
        short        state;
        bool         ack_time_is_null;
        time_t       ack_time_low;
        time_t       ack_time_high;
        bool         in_downtime;
      } const          entries[] = {
	/*
	** Service 1-1.
	*/
	// Start = UNKNOWN.
        { 1, 1, 0, 1, false, t1, t2, 3, true, 0, 0, false },
	// Step 1 = OK.
        { 1, 1, t1, t2, false, t2, t3, 0, true, 0, 0, false },
	// Step 2 = WARNING.
        { 1, 1, t2, t3, false, t3, t4, 3, true, 0, 0, false },
	// Step 3 = DOWNTIME.
	{ 1, 1, t3, t4, true, 0, 0, 3, true, 0, 0, true },

	/*
	** Service 1-2.
	*/
	// Start = UNKNOWN.
        { 1, 2, 0, 1, false, t1, t2, 3, true, 0, 0, false },
	// Step 1 = OK.
        { 1, 2, t1, t2, false, t2, t3, 0, true, 0, 0, false },
	// Step 2 = CRITICAL, step 3 = ACK (STICKY), step 4 = WARNING.
        { 1, 2, t2, t3, true, 0, 0, 3, false, t3, t4, false },

	/*
	** Service 2-3.
	*/
	// Start = UNKNOWN.
	{ 2, 3, 0, 1, false, t1, t2, 3, true, 0, 0, false },
	// Step 1 = OK.
	{ 2, 3, t1, t2, false, t2, t3, 0, true, 0, 0, false },
        // Step 2 = CRITICAL, step 3 = ACK (STICKY).
	{ 2, 3, t2, t3, true, 0, 0, 2, false, t3, t4, false },

	/*
	** Service 2-4.
	*/
	// Start = UNKNOWN.
	{ 2, 4, 0, 1, false, t1, t2, 3, true, 0, 0, false },
	// Step 1 = OK.
	{ 2, 4, t1, t2, false, t2, t3, 0, true, 0, 0, false },
	// Step 2 = WARNING, step 3 = ACK (NORMAL).
	{ 2, 4, t2, t3, false, t4, t5, 3, false, t3, t4, false },
	// Step 4 = DOWNTIME.
	{ 2, 4, t4, t5, true, 0, 0, 3, false, t4, t5, true },

	{ 3, 5, 0, 1, false, t1, t2, 3, true, 0, 0, false },
	{ 3, 5, t1, t2, true, 0, 0, 0, true, 0, 0, false },

	{ 3, 6, 0, 1, false, t1, t2, 3, true, 0, 0, false },
	{ 3, 6, t1, t2, true, 0, 0, 0, true, 0, 0, false },

	{ 4, 7, 0, 1, false, t1, t2, 3, true, 0, 0, false },
	{ 4, 7, t1, t2, true, 0, 0, 0, true, 0, 0, false },

	{ 4, 8, 0, 1, false, t1, t2, 3, true, 0, 0, false },
	{ 4, 8, t1, t2, true, 0, 0, 0, true, 0, 0, false },

	{ 5, 9, 0, 1, false, t1, t2, 3, true, 0, 0, false },
	{ 5, 9, t1, t2, true, 0, 0, 0, true, 0, 0, false },

	{ 5, 10, 0, 1, false, t1, t2, 3, true, 0, 0, false },
	{ 5, 10, t1, t2, true, 0, 0, 0, true, 0, 0, false },

	{ 6, 11, 0, 1, false, t1, t2, 3, true, 0, 0, false },
	{ 6, 11, t1, t2, true, 0, 0, 0, true, 0, 0, false },

	{ 6, 12, 0, 1, false, t1, t2, 3, true, 0, 0, false },
	{ 6, 12, t1, t2, true, 0, 0, 0, true, 0, 0, false },

	{ 7, 13, 0, 1, false, t1, t2, 3, true, 0, 0, false },
	{ 7, 13, t1, t2, true, 0, 0, 0, true, 0, 0, false },

	{ 7, 14, 0, 1, false, t1, t2, 3, true, 0, 0, false },
	{ 7, 14, t1, t2, true, 0, 0, 0, true, 0, 0, false },

	{ 8, 15, 0, 1, false, t1, t2, 3, true, 0, 0, false },
	{ 8, 15, t1, t2, true, 0, 0, 0, true, 0, 0, false },

	{ 8, 16, 0, 1, false, t1, t2, 3, true, 0, 0, false },
	{ 8, 16, t1, t2, true, 0, 0, 0, true, 0, 0, false },

	{ 9, 17, 0, 1, false, t1, t2, 3, true, 0, 0, false },
	{ 9, 17, t1, t2, true, 0, 0, 0, true, 0, 0, false },

	{ 9, 18, 0, 1, false, t1, t2, 3, true, 0, 0, false },
	{ 9, 18, t1, t2, true, 0, 0, 0, true, 0, 0, false },

	{ 10, 19, 0, 1, false, t1, t2, 3, true, 0, 0, false },
	{ 10, 19, t1, t2, true, 0, 0, 0, true, 0, 0, false },

	{ 10, 20, 0, 1, false, t1, t2, 3, true, 0, 0, false },
	{ 10, 20, t1, t2, true, 0, 0, 0, true, 0, 0, false }
      };

      // Get service state events.
      QSqlQuery q(db);
      if (!q.exec("SELECT host_id, service_id, start_time, end_time,"
                  "       state, ack_time, in_downtime"
                  " FROM servicestateevents"
                  " ORDER BY host_id, service_id, start_time"))
        throw (exceptions::msg() << "cannot get service state events: "
               << q.lastError().text());

      // Compare DB with expected content.
      for (unsigned int i(0);
           i < sizeof(entries) / sizeof(*entries);
           ++i) {
        // Get next entry.
        if (!q.next())
          throw (exceptions::msg()
                 << "not enough service state events in DB: got " << i
                 << ", expected "
                 << (sizeof(entries) / sizeof(*entries)));

        // Match entry.
        if ((q.value(0).toUInt() != entries[i].host_id)
            || (q.value(1).toUInt() != entries[i].service_id)
            || (static_cast<time_t>(q.value(2).toLongLong())
                < entries[i].start_time_low)
            || (static_cast<time_t>(q.value(2).toLongLong())
                > entries[i].start_time_high)
            || (entries[i].end_time_is_null && !q.value(3).isNull())
            || (!entries[i].end_time_is_null
                && ((static_cast<time_t>(q.value(3).toLongLong())
                     < entries[i].end_time_low)
                    || (static_cast<time_t>(q.value(3).toLongLong())
                        > entries[i].end_time_high)))
            || (q.value(4).toInt() != entries[i].state)
            || (entries[i].ack_time_is_null && !q.value(5).isNull())
            || (!entries[i].ack_time_is_null
                && ((static_cast<time_t>(q.value(5).toLongLong())
                     < entries[i].ack_time_low)
                    || (static_cast<time_t>(q.value(5).toLongLong())
                        > entries[i].ack_time_high)))
            || (static_cast<bool>(q.value(6).toInt())
                != entries[i].in_downtime)) {
          exceptions::msg e;
          e << "invalid service state event entry #" << i
            << ": got (host id " << q.value(0).toUInt()
            << ", service id " << q.value(1).toUInt() << ", start time "
            << q.value(2).toUInt() << ", end time "
            << (q.value(3).isNull() ? "null" : q.value(3).toString())
            << ", state " << q.value(4).toInt() << ", ack time "
            << (q.value(5).isNull() ? "null" : q.value(5).toString())
            << ", in downtime " << q.value(6).toInt() << "), expected ("
            << entries[i].host_id << ", " << entries[i].service_id
            << ", " << entries[i].start_time_low << ":"
            << entries[i].start_time_high << ", ";
          if (entries[i].end_time_is_null)
            e << "null";
          else
            e << entries[i].end_time_low << ":"
              << entries[i].end_time_high;
          e << ", " << entries[i].state << ", ";
          if (entries[i].ack_time_is_null)
            e << "null";
          else
            e << entries[i].ack_time_low << ":"
              << entries[i].ack_time_high;
          e << ", " << entries[i].in_downtime << ")";
          throw (e);
        }
      }

      // No more results.
      if (q.next())
        throw (exceptions::msg()
               << "too much service state events in DB");
    }

    // Stop daemons.
    daemon.stop();
    sleep_for(2 * MONITORING_ENGINE_INTERVAL_LENGTH);
    broker.stop();
    sleep_for(2 * MONITORING_ENGINE_INTERVAL_LENGTH);

    // Check passive correlation.
    std::ifstream active_correlation(cbmod_correlation_path.c_str());
    if (!active_correlation.is_open())
      throw (exceptions::msg() << "bad active correlation dump");
    std::ifstream passive_correlation(broker_correlation_path.c_str());
    if (!passive_correlation.is_open())
      throw (exceptions::msg() << "bad passive correlation dump");

    std::vector<std::string> active;
    while (!active_correlation.eof() && active_correlation.good()) {
      std::string line;
      std::getline(active_correlation, line);
      active.push_back(line);
    }
    std::vector<std::string> passive;
    while (!passive_correlation.eof() && passive_correlation.good()) {
      std::string line;
      std::getline(passive_correlation, line);
      passive.push_back(line);
    }

    if (active != passive)
      throw (exceptions::msg() << "active correlation and passive "
             "correlation are not equal");

    // Success.
    retval = EXIT_SUCCESS;
  }
  catch (std::exception const& e) {
    std::cerr << e.what() << std::endl;
  }
  catch (...) {
    std::cerr << "unknown exception" << std::endl;
  }

  // Cleanup.
  daemon.stop();
  broker.stop();
  config_remove(engine_config_path.c_str());
  ::remove(broker_config_path.c_str());
  ::remove(broker_correlation_path.c_str());
  ::remove(cbmod_config_path.c_str());
  ::remove(cbmod_correlation_path.c_str());
  //config_db_close(DB_NAME);
  free_hosts(hosts);
  free_services(services);

  return (retval);
}
