/*
** Copyright 2012 Merethis
**
** This file is part of Centreon Connector Perl.
**
** Centreon Connector Perl is free software: you can redistribute it
** and/or modify it under the terms of the GNU Affero General Public
** License as published by the Free Software Foundation, either version
** 3 of the License, or (at your option) any later version.
**
** Centreon Connector Perl is distributed in the hope that it will be
** useful, but WITHOUT ANY WARRANTY; without even the implied warranty
** of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** Affero General Public License for more details.
**
** You should have received a copy of the GNU Affero General Public
** License along with Centreon Connector Perl. If not, see
** <http://www.gnu.org/licenses/>.
*/

#include "com/centreon/process.hh"
#include "test/connector/binary.hh"

using namespace com::centreon;

/**
 *  Check that connector exits when stdin is closed.
 *
 *  @return 0 on success.
 */
int main() {
  // Process.
  process p;
  p.with_stdin(true);
  p.exec(CONNECTOR_PERL_BINARY);
  p.with_stdin(false);

  // Wait for process termination.
  int retval;
  int exitcode;
  if (!p.wait(5000, &exitcode)) {
    p.terminate();
    p.wait();
    retval = 1;
  }
  else
    retval = (exitcode != 0);
  return (retval);
}
