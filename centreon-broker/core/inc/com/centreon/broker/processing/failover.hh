/*
** Copyright 2011-2012 Merethis
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

#ifndef CCB_PROCESSING_FAILOVER_HH
#  define CCB_PROCESSING_FAILOVER_HH

#  include <limits.h>
#  include <QMutex>
#  include <QReadWriteLock>
#  include <QThread>
#  include <time.h>
#  include "com/centreon/broker/io/endpoint.hh"
#  include "com/centreon/broker/io/stream.hh"
#  include "com/centreon/broker/misc/shared_ptr.hh"
#  include "com/centreon/broker/processing/feeder.hh"

namespace                com {
  namespace              centreon {
    namespace            broker {
      // Forward declaration.
      namespace          stats {
        class            worker;
      }

      namespace          processing {
        /**
         *  @class failover failover.hh "com/centreon/broker/processing/failover.hh"
         *  @brief Failover thread.
         *
         *  Thread that provide failover on inputs or outputs.
         */
        class            failover : public QThread, public io::stream {
          Q_OBJECT
          friend class   stats::worker;

         public:
                         failover(bool is_out);
                         failover(failover const& f);
                         ~failover();
          failover&      operator=(failover const& f);
          time_t         get_buffering_timeout() const throw ();
          double         get_event_processing_speed() const throw ();
          time_t         get_last_event() const throw ();
          time_t         get_read_timeout() const throw ();
          time_t         get_retry_interval() const throw ();
          void           process(bool in = false, bool out = false);
          misc::shared_ptr<io::data>
                         read();
          misc::shared_ptr<io::data>
                         read(time_t timeout);
          void           run();
          void           set_buffering_timeout(time_t secs);
          void           set_endpoint(
                           misc::shared_ptr<io::endpoint> endp);
          void           set_failover(
                           misc::shared_ptr<processing::failover> fo);
          void           set_name(QString const& name);
          void           set_read_timeout(time_t read_timeout);
          void           set_retry_interval(time_t retry_interval);
          bool           wait(unsigned long time = ULONG_MAX);
          void           write(misc::shared_ptr<io::data> d);

          static time_t const
                         event_window_length = 30;

         signals:
          void           exception_caught();
          void           initial_lock();

         private:
          // Data that doesn't require locking.
          volatile time_t _buffering_timeout;
          misc::shared_ptr<io::endpoint>
                         _endpoint;
          unsigned int   _events[event_window_length];
          misc::shared_ptr<failover>
                         _failover;
          bool           _initial;
          bool           _is_out;
          time_t         _last_connect_attempt;
          time_t         _last_connect_success;
          QString        _last_error;
          time_t         _last_event;
          QString        _name;
          time_t         _read_timeout;
          volatile time_t _retry_interval;

          // Retained data.
          misc::shared_ptr<io::data>
                         _data;
          mutable QMutex _datam;

          // Exit flag.
          volatile bool  _immediate;
          volatile bool  _should_exit;
          mutable QMutex _should_exitm;

          // Stream locking.
          mutable QReadWriteLock _fromm;
          mutable QReadWriteLock _tom;
        };
      }
    }
  }
}

#endif // !CCB_PROCESSING_FAILOVER_HH
