/*
** Copyright 2009-2013 Merethis
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

#ifndef CCB_NEB_HOST_SERVICE_HH
#  define CCB_NEB_HOST_SERVICE_HH

#  include <QString>
#  include "com/centreon/broker/namespace.hh"

CCB_BEGIN()

namespace         neb {
  /**
   *  @class host_service host_service.hh "com/centreon/broker/neb/host_service.hh"
   *  @brief Common class to host and service.
   *
   *  This class holds data members common to host and service
   *  classes.
   *
   *  @see host
   *  @see service
   */
  class           host_service {
  public:
                  host_service();
                  host_service(host_service const& hs);
    virtual       ~host_service();
    host_service& operator=(host_service const& hs);

    QString       action_url;
    bool          check_freshness;
    bool          default_active_checks_enabled;
    bool          default_event_handler_enabled;
    bool          default_failure_prediction;
    bool          default_flap_detection_enabled;
    bool          default_notifications_enabled;
    bool          default_passive_checks_enabled;
    bool          default_process_perf_data;
    QString       display_name;
    double        first_notification_delay;
    double        freshness_threshold;
    double        high_flap_threshold;
    QString       icon_image;
    QString       icon_image_alt;
    double        low_flap_threshold;
    QString       notes;
    QString       notes_url;
    double        notification_interval;
    QString       notification_period;
    bool          notify_on_downtime;
    bool          notify_on_flapping;
    bool          notify_on_recovery;
    bool          retain_nonstatus_information;
    bool          retain_status_information;

  private:
    void          _internal_copy(host_service const& hs);
  };
}

CCB_END()

#endif // !CCB_NEB_HOST_SERVICE_HH
