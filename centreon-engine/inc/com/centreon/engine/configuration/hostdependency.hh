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

#  include "com/centreon/engine/configuration/group.hh"
#  include "com/centreon/engine/configuration/object.hh"
#  include "com/centreon/engine/configuration/opt.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace                  configuration {
  class                    hostdependency
    : public object {
  public:
    enum                   action_on {
      none = 0,
      up = (1 << 0),
      down = (1 << 1),
      unreachable = (1 << 2),
      pending = (1 << 3)
    };

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

    std::size_t            id() const throw ();
    void                   merge(object const& obj);
    bool                   parse(
                             std::string const& key,
                             std::string const& value);

  private:
    bool                   _set_dependency_period(std::string const& value);
    bool                   _set_dependent_hostgroups(std::string const& value);
    bool                   _set_dependent_hosts(std::string const& value);
    bool                   _set_execution_failure_options(std::string const& value);
    bool                   _set_hostgroups(std::string const& value);
    bool                   _set_hosts(std::string const& value);
    bool                   _set_inherits_parent(bool value);
    bool                   _set_notification_failure_options(std::string const& value);

    std::string            _dependency_period;
    group                  _dependent_hostgroups;
    group                  _dependent_hosts;
    opt<unsigned int>      _execution_failure_options;
    group                  _hostgroups;
    group                  _hosts;
    opt<bool>              _inherits_parent;
    opt<unsigned int>      _notification_failure_options;
  };
}

CCE_END()

#endif // !CCE_CONFIGURATION_HOSTDEPENDENCY_HH
