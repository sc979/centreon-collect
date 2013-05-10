/*
** Copyright 2011-2013 Merethis
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

#ifndef CCE_CONFIGURATION_HOSTDEPENDENCY_HH
#  define CCE_CONFIGURATION_HOSTDEPENDENCY_HH

#  include "com/centreon/engine/configuration/object.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace                  configuration {
  class                    hostdependency
    : public object {
  public:
                           hostdependency();
                           hostdependency(hostdependency const& right);
                           ~hostdependency() throw ();
    hostdependency&        operator=(hostdependency const& right);
    bool                   operator==(
                             hostdependency const& right) const throw ();
    bool                   operator!=(
                             hostdependency const& right) const throw ();
    /*
    std::string const&     dependency_period() const throw ();
    std::list<std::string> const&
                           dependent_hostgroups() const throw ();
    std::list<std::string> const&
                           dependent_hosts() const throw ();
    unsigned int           execution_failure_options() const throw ();
    std::list<std::string> const&
                           hostgroups() const throw ();
    std::list<std::string> const&
                           hosts() const throw ();
    bool                   inherits_parent() const throw ();
    unsigned int           notification_failure_options() const throw ();
    */

    bool                   parse(
                             std::string const& key,
                             std::string const& value);

  private:
    void                   _set_dependency_period(std::string const& value);
    void                   _set_dependent_hostgroups(std::string const& value);
    void                   _set_dependent_hosts(std::string const& value);
    void                   _set_execution_failure_options(std::string const& value);
    void                   _set_hostgroups(std::string const& value);
    void                   _set_hosts(std::string const& value);
    void                   _set_inherits_parent(bool value);
    void                   _set_notification_failure_options(std::string const& value);

    std::string            _dependency_period;
    std::list<std::string> _dependent_hostgroups;
    std::list<std::string> _dependent_hosts;
    unsigned int           _execution_failure_options;
    std::list<std::string> _hostgroups;
    std::list<std::string> _hosts;
    bool                   _inherits_parent;
    unsigned int           _notification_failure_options;
  };
}

CCE_END()

#endif // !CCE_CONFIGURATION_HOSTDEPENDENCY_HH

