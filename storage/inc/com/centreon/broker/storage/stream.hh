/*
** Copyright 2011-2013 Merethis
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

#ifndef CCB_STORAGE_STREAM_HH
#  define CCB_STORAGE_STREAM_HH

#  include <list>
#  include <map>
#  include <memory>
#  include <QSqlDatabase>
#  include <QString>
#  include <utility>
#  include "com/centreon/broker/io/stream.hh"
#  include "com/centreon/broker/multiplexing/hooker.hh"
#  include "com/centreon/broker/namespace.hh"
#  include "com/centreon/broker/storage/rebuilder.hh"

CCB_BEGIN()

namespace          storage {
  /**
   *  @class stream stream.hh "com/centreon/broker/storage/stream.hh"
   *  @brief Storage stream.
   *
   *  Handle perfdata and insert proper informations in index_data and
   *  metrics table of a centstorage DB.
   */
  class            stream : public multiplexing::hooker {
  public:
                   stream(
                     QString const& storage_type,
                     QString const& storage_host,
                     unsigned short storage_port,
                     QString const& storage_user,
                     QString const& storage_password,
                     QString const& storage_db,
                     unsigned int queries_per_transaction,
                     unsigned int rrd_len,
                     time_t interval_length,
                     unsigned int rebuild_check_interval,
                     bool store_in_db = true,
                     bool check_replication = true,
                     bool insert_in_index_data = false);
                   stream(stream const& s);
                   ~stream();
    void           process(bool in = false, bool out = true);
    void           read(misc::shared_ptr<io::data>& d);
    void           starting();
    void           statistics(std::string& buffer) const;
    void           stopping();
    void           update();
    unsigned int   write(misc::shared_ptr<io::data> const& d);

  private:
    struct         index_info {
      QString      host_name;
      unsigned int index_id;
      unsigned int rrd_retention;
      QString      service_description;
      bool         special;
    };
    struct         metric_info {
      double       crit;
      double       crit_low;
      bool         crit_mode;
      double       max;
      unsigned int metric_id;
      double       min;
      unsigned int type;
      QString      unit_name;
      double       warn;
      double       warn_low;
      bool         warn_mode;
    };
    struct         metric_value {
      time_t       c_time;
      unsigned int metric_id;
      short        status;
      double       value;
    };

    stream&        operator=(stream const& s);
    void           _check_deleted_index();
    void           _clear_qsql();
    void           _delete_metrics(
                     std::list<unsigned long long> const& metrics_to_delete);
    unsigned int   _find_index_id(
                     unsigned int host_id,
                     unsigned int service_id,
                     QString const& host_name,
                     QString const& service_desc,
                     unsigned int* rrd_len = NULL);
    unsigned int   _find_metric_id(
                     unsigned int index_id,
                     QString metric_name,
		     QString const& unit_name,
		     double warn,
                     double warn_low,
                     bool warn_mode,
		     double crit,
                     double crit_low,
                     bool crit_mode,
		     double min,
		     double max,
                     unsigned int* type = NULL);
    void           _insert_perfdatas();
    void           _prepare();
    void           _rebuild_cache();
    void           _update_status(std::string const& status);

    std::map<std::pair<unsigned int, unsigned int>, index_info>
                   _index_cache;
    bool           _insert_in_index_data;
    time_t         _interval_length;
    std::map<std::pair<unsigned int, QString>, metric_info>
                   _metric_cache;
    std::list<metric_value>
                   _perfdata_queue;
    bool           _process_out;
    unsigned int   _queries_per_transaction;
    rebuilder      _rebuild_thread;
    unsigned int   _rrd_len;
    std::string    _status;
    mutable QMutex _statusm;
    bool           _store_in_db;
    unsigned int   _transaction_queries;
    std::auto_ptr<QSqlQuery>
                   _update_metrics;
    std::auto_ptr<QSqlDatabase>
                   _storage_db;
  };
}

CCB_END()

#endif // !CCB_STORAGE_STREAM_HH
