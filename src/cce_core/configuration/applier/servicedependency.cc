/*
** Copyright 2011-2013,2017-2019 Centreon
**
** This file is part of Centreon Engine.
**
** Centreon Engine is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License version 2
** as published by the Free Software Foundation.
**
** Centreon Engine is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Centreon Engine. If not, see
** <http://www.gnu.org/licenses/>.
*/

#include "com/centreon/engine/configuration/applier/servicedependency.hh"
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/object.hh"
#include "com/centreon/engine/configuration/servicedependency.hh"
#include "com/centreon/exceptions/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"

using namespace com::centreon::exceptions;
using namespace com::centreon::engine::configuration;

/**
 *  Default constructor.
 */
applier::servicedependency::servicedependency() {}

/**
 *  Destructor.
 */
applier::servicedependency::~servicedependency() throw() {}

/**
 *  Add new service dependency.
 *
 *  @param[in] obj  The new servicedependency to add into the monitoring
 *                  engine.
 */
void applier::servicedependency::add_object(
    configuration::servicedependency const& obj) {
  // Check service dependency.
  if (obj.hosts().size() != 1 || !obj.hostgroups().empty() ||
      obj.service_description().size() != 1 || !obj.servicegroups().empty() ||
      obj.dependent_hosts().size() != 1 ||
      !obj.dependent_hostgroups().empty() ||
      obj.dependent_service_description().size() != 1 ||
      !obj.dependent_servicegroups().empty())
    throw error(
        "Could not create service dependency with multiple (dependent) hosts / "
        "host groups / services / service groups");

  if (obj.dependency_type() !=
          configuration::servicedependency::execution_dependency &&
      obj.dependency_type() !=
          configuration::servicedependency::notification_dependency)
    throw error(
        "Could not create unexpanded dependency of service '{}' of host '{}' "
        "on service '{}' of host '{}'",
        obj.dependent_service_description().front(),
        obj.dependent_hosts().front(),
        obj.service_description().front(),
        obj.hosts().front());

  // Logging.
  logger(logging::dbg_config, logging::more)
      << "Creating new service dependency of service '"
      << obj.dependent_service_description().front() << "' of host '"
      << obj.dependent_hosts().front() << "' on service '"
      << obj.service_description().front() << "' of host '"
      << obj.hosts().front() << "'.";

  // Add dependency to the global configuration set.
  config->servicedependencies().insert(obj);

  std::shared_ptr<engine::servicedependency> sd;

  if (obj.dependency_type() ==
      configuration::servicedependency::execution_dependency)
    // Create execution dependency.
    sd = std::make_shared<engine::servicedependency>(
        obj.dependent_hosts().front(),
        obj.dependent_service_description().front(),
        obj.hosts().front(),
        obj.service_description().front(),
        dependency::execution,
        obj.inherits_parent(),
        static_cast<bool>(obj.execution_failure_options() &
                          configuration::servicedependency::ok),
        static_cast<bool>(obj.execution_failure_options() &
                          configuration::servicedependency::warning),
        static_cast<bool>(obj.execution_failure_options() &
                          configuration::servicedependency::unknown),
        static_cast<bool>(obj.execution_failure_options() &
                          configuration::servicedependency::critical),
        static_cast<bool>(obj.execution_failure_options() &
                          configuration::servicedependency::pending),
        obj.dependency_period());
  else
    // Create notification dependency.
    sd = std::make_shared<engine::servicedependency>(
        obj.dependent_hosts().front(),
        obj.dependent_service_description().front(),
        obj.hosts().front(),
        obj.service_description().front(),
        dependency::notification,
        obj.inherits_parent(),
        static_cast<bool>(obj.notification_failure_options() &
                          configuration::servicedependency::ok),
        static_cast<bool>(obj.notification_failure_options() &
                          configuration::servicedependency::warning),
        static_cast<bool>(obj.notification_failure_options() &
                          configuration::servicedependency::unknown),
        static_cast<bool>(obj.notification_failure_options() &
                          configuration::servicedependency::critical),
        static_cast<bool>(obj.notification_failure_options() &
                          configuration::servicedependency::pending),
        obj.dependency_period());

  // Add new items to the global list.
  engine::servicedependency::servicedependencies.insert(
      {{sd->get_dependent_hostname(), sd->get_dependent_service_description()},
       sd});

  // Notify event broker.
  timeval tv(get_broker_timestamp(nullptr));
  broker_adaptive_dependency_data(
      NEBTYPE_SERVICEDEPENDENCY_ADD, NEBFLAG_NONE, NEBATTR_NONE, sd.get(), &tv);
}

/**
 *  Expand service dependencies.
 *
 *  @param[in,out] s  Configuration being applied.
 */
void applier::servicedependency::expand_objects(configuration::state& s) {
  // Browse all dependencies.
  configuration::set_servicedependency expanded;
  for (configuration::set_servicedependency::const_iterator
           it_dep(s.servicedependencies().begin()),
       end_dep(s.servicedependencies().end());
       it_dep != end_dep;
       ++it_dep) {
    // Expand service dependency instances.
    if (it_dep->hosts().size() != 1 || !it_dep->hostgroups().empty() ||
        it_dep->service_description().size() != 1 ||
        !it_dep->servicegroups().empty() ||
        it_dep->dependent_hosts().size() != 1 ||
        !it_dep->dependent_hostgroups().empty() ||
        it_dep->dependent_service_description().size() != 1 ||
        !it_dep->dependent_servicegroups().empty() ||
        it_dep->dependency_type() ==
            configuration::servicedependency::unknown_type) {
      // Expand depended services.
      std::set<std::pair<std::string, std::string> > depended_services;
      _expand_services(it_dep->hosts(),
                       it_dep->hostgroups(),
                       it_dep->service_description(),
                       it_dep->servicegroups(),
                       s,
                       depended_services);

      // Expand dependent services.
      std::set<std::pair<std::string, std::string> > dependent_services;
      _expand_services(it_dep->dependent_hosts(),
                       it_dep->dependent_hostgroups(),
                       it_dep->dependent_service_description(),
                       it_dep->dependent_servicegroups(),
                       s,
                       dependent_services);

      // Browse all depended and dependent services.
      for (std::set<std::pair<std::string, std::string> >::const_iterator
               it1(depended_services.begin()),
           end1(depended_services.end());
           it1 != end1;
           ++it1)
        for (std::set<std::pair<std::string, std::string> >::const_iterator
                 it2(dependent_services.begin()),
             end2(dependent_services.end());
             it2 != end2;
             ++it2)
          for (int i(0); i < 2; ++i) {
            // Create service dependency instance.
            configuration::servicedependency sdep(*it_dep);
            sdep.hostgroups().clear();
            sdep.hosts().clear();
            sdep.hosts().push_back(it1->first);
            sdep.servicegroups().clear();
            sdep.service_description().clear();
            sdep.service_description().push_back(it1->second);
            sdep.dependent_hostgroups().clear();
            sdep.dependent_hosts().clear();
            sdep.dependent_hosts().push_back(it2->first);
            sdep.dependent_servicegroups().clear();
            sdep.dependent_service_description().clear();
            sdep.dependent_service_description().push_back(it2->second);
            if (i == 0) {
              sdep.dependency_type(
                  configuration::servicedependency::execution_dependency);
              sdep.notification_failure_options(0);
            } else {
              sdep.dependency_type(
                  configuration::servicedependency::notification_dependency);
              sdep.execution_failure_options(0);
            }

            // Insert new service dependency. We do not need to expand it
            // because no expansion is made on 1->1 dependency.
            expanded.insert(sdep);
          }
    }
    // Insert dependency if already good to go.
    else
      expanded.insert(*it_dep);
  }

  // Set expanded service dependencies in configuration state.
  s.servicedependencies().swap(expanded);
}

/**
 *  @brief Modify service dependency.
 *
 *  Service dependencies cannot be defined with anything else than their
 *  full content. Therefore no modification can occur.
 *
 *  @param[in] obj  Unused.
 */
void applier::servicedependency::modify_object(
    configuration::servicedependency const& obj) {
  (void)obj;
  throw error(
      "Could not modify a service dependency: service dependency objects can "
      "only be added or removed, this is likely a software bug that you should "
      "report to Centreon Engine developers");
}

/**
 *  Remove old service dependency.
 *
 *  @param[in] obj  The service dependency to remove from the monitoring
 *                  engine.
 */
void applier::servicedependency::remove_object(
    configuration::servicedependency const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
      << "Removing a service dependency.";

  // Find service dependency.
  servicedependency_mmap::iterator it(
      engine::servicedependency::servicedependencies_find(obj.key()));
  if (it != engine::servicedependency::servicedependencies.end()) {
    // Notify event broker.
    timeval tv(get_broker_timestamp(nullptr));
    broker_adaptive_dependency_data(NEBTYPE_SERVICEDEPENDENCY_DELETE,
                                    NEBFLAG_NONE,
                                    NEBATTR_NONE,
                                    it->second.get(),
                                    &tv);

    // Remove service dependency from its list.
    engine::servicedependency::servicedependencies.erase(it);
  }

  // Remove dependency from the global configuration set.
  config->servicedependencies().erase(obj);
}

/**
 *  Resolve a servicedependency.
 *
 *  @param[in] obj  Servicedependency object.
 */
void applier::servicedependency::resolve_object(
    configuration::servicedependency const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
      << "Resolving a service dependency.";

  // Find service dependency.
  servicedependency_mmap::iterator it(
      engine::servicedependency::servicedependencies_find(obj.key()));
  if (engine::servicedependency::servicedependencies.end() == it)
    throw error("Cannot resolve non-existing service dependency");

  // Resolve service dependency.
  it->second->resolve(config_warnings, config_errors);
}

/**
 *  Expand services.
 *
 *  @param[in]     hst      Hosts.
 *  @param[in]     hg       Host groups.
 *  @param[in]     svc      Service descriptions.
 *  @param[in]     sg       Service groups.
 *  @param[in,out] s        Configuration state.
 *  @param[out]    expanded Expanded services.
 */
void applier::servicedependency::_expand_services(
    std::list<std::string> const& hst,
    std::list<std::string> const& hg,
    std::list<std::string> const& svc,
    std::list<std::string> const& sg,
    configuration::state& s,
    std::set<std::pair<std::string, std::string> >& expanded) {
  // Expanded hosts.
  std::set<std::string> all_hosts;

  // Base hosts.
  all_hosts.insert(hst.begin(), hst.end());

  // Host groups.
  for (std::list<std::string>::const_iterator it(hg.begin()), end(hg.end());
       it != end;
       ++it) {
    // Find host group.
    configuration::set_hostgroup::iterator it_group(s.hostgroups_find(*it));
    if (it_group == s.hostgroups().end())
      throw error("Could not resolve host group '{}'", *it);

    // Add host group members.
    all_hosts.insert(it_group->members().begin(), it_group->members().end());
  }

  // Hosts * services.
  for (std::set<std::string>::const_iterator it_host(all_hosts.begin()),
       end_host(all_hosts.end());
       it_host != end_host;
       ++it_host)
    for (std::list<std::string>::const_iterator it_service(svc.begin()),
         end_service(svc.end());
         it_service != end_service;
         ++it_service)
      expanded.insert({*it_host, *it_service});

  // Service groups.
  for (std::list<std::string>::const_iterator it(sg.begin()), end(sg.end());
       it != end;
       ++it) {
    // Find service group.
    configuration::set_servicegroup::iterator it_group(
        s.servicegroups_find(*it));
    if (it_group == s.servicegroups().end())
      throw error("Could not resolve service group '{}'", *it);

    // Add service group members.
    for (set_pair_string::const_iterator it_member(it_group->members().begin()),
         end_member(it_group->members().end());
         it_member != end_member;
         ++it_member)
      expanded.insert(*it_member);
  }
}
