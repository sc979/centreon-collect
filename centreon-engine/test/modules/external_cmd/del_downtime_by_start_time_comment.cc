/*
** Copyright 2011 Merethis
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

#include <QDebug>
#include <exception>
#include "error.hh"
#include "commands.hh"
#include "globals.hh"
#include "error.hh"

/**
 *  Run del_downtime_by_start_time_comment test.
 */
static void check_del_downtime_by_start_time_comment() {
  init_object_skiplists();

  host* hst = add_host("name", NULL, NULL, "localhost", NULL, 0, 0.0, 0.0, 42,
                       0, 0, 0, 0, 0, 0.0, 0.0, NULL, 0, NULL, 0, 0, NULL, 0,
                       0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, 0, NULL,
                       NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0.0, 0.0,
                       0.0, 0, 0, 0, 0, 0);
  if (!hst)
    throw (engine_error() << "create host failed.");

  next_downtime_id = 1;
  unsigned long downtime_id;
  scheduled_downtime_list = NULL;
  if (add_new_host_downtime("name",
                            time(NULL),
                            NULL,
                            "data",
                            1317196300,
                            time(NULL),
                            0,
                            0,
                            0,
                            &downtime_id) != OK)
    throw (engine_error() << "create downtime failed.");

  char const* cmd("[1317196300] DEL_DOWNTIME_BY_START_TIME_COMMENT;1317196300;data");
  process_external_command(cmd);

  if (scheduled_downtime_list)
    throw (engine_error() << "del_downtime_by_start_time_comment failed.");

  delete[] hst->name;
  delete[] hst->display_name;
  delete[] hst->alias;
  delete[] hst->address;
  delete hst;

  free_object_skiplists();
}

/**
 *  Check processing of del_downtime_by_start_time_comment works.
 */
int main(void) {
  try {
    check_del_downtime_by_start_time_comment();
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    return (1);
  }
  return (0);
}
