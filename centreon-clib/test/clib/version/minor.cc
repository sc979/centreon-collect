/*
** Copyright 2011-2013 Merethis
**
** This file is part of Centreon Clib.
**
** Centreon Clib is free software: you can redistribute it
** and/or modify it under the terms of the GNU Affero General Public
** License as published by the Free Software Foundation, either version
** 3 of the License, or (at your option) any later version.
**
** Centreon Clib is distributed in the hope that it will be
** useful, but WITHOUT ANY WARRANTY; without even the implied warranty
** of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** Affero General Public License for more details.
**
** You should have received a copy of the GNU Affero General Public
** License along with Centreon Clib. If not, see
** <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include "com/centreon/clib/version.hh"

using namespace com::centreon::clib;

/**
 *  Check that the version minor returned by the library matches the
 *  header.
 *
 *  @return 0 on success.
 */
int main() {
  // Check.
  int retval((version::get_minor() != version::minor)
             || (version::minor != CENTREON_CLIB_VERSION_MINOR));

  // Message.
  if (retval)
    std::cout << "Version minor mismatch" << std::endl
              << "  library returned "
              << version::get_minor() << std::endl
              << "  header returned  "
              << version::minor << std::endl
              << "  macro returned   "
              << CENTREON_CLIB_VERSION_MINOR << std::endl;
  else
    std::cout << "Version minor is consistent (" << version::minor
              << ")" << std::endl;

  // Return check result.
  return (retval);
}
