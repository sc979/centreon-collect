/*
** Copyright 2011,2015 Merethis
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

#include "com/centreon/broker/correlation/node.hh"

using namespace com::centreon::broker;

/**
 *  Check that node is properly default constructed.
 *
 *  @return 0 on success.
 */
int main() {
  // Object.
  correlation::node n;

  // Check default construction.
  return (!n.get_children().empty()
          || !n.get_dependeds().empty()
          || !n.get_dependencies().empty()
          || (n.host_id != 0)
          || n.in_downtime
          || n.my_issue.get()
          || !n.get_parents().empty()
          || (n.service_id != 0)
          || (n.current_state != 0));
}
