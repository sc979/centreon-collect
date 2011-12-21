/*
** Copyright 2011 Merethis
**
** This file is part of Centreon Connector SSH.
**
** Centreon Connector SSH is free software: you can redistribute it
** and/or modify it under the terms of the GNU Affero General Public
** License as published by the Free Software Foundation, either version
** 3 of the License, or (at your option) any later version.
**
** Centreon Connector SSH is distributed in the hope that it will be
** useful, but WITHOUT ANY WARRANTY; without even the implied warranty
** of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** Affero General Public License for more details.
**
** You should have received a copy of the GNU Affero General Public
** License along with Centreon Connector SSH. If not, see
** <http://www.gnu.org/licenses/>.
*/

#ifndef CCC_SSH_SESSIONS_HH_
# define CCC_SSH_SESSIONS_HH_

# include <map>
# include "com/centreon/connector/ssh/credentials.hh"
# include "com/centreon/connector/ssh/namespace.hh"

CCCS_BEGIN()

// Forward declaration.
class              session;

/**
 *  @class sessions sessions "com/centreon/connector/ssh/sessions.hh"
 *  @brief Session manager.
 *
 *  Manage sessions.
 */
class              sessions {
public:
                   ~sessions();
  session*&        operator[](credentials const& c);
  std::map<credentials, session*>::iterator
                   begin();
  bool             empty() const;
  std::map<credentials, session*>::iterator
                   end();
  void             erase(credentials const& key);
  void             erase(std::map<credentials, session*>::iterator it);
  static sessions& instance();
  unsigned int     size() const;

private:
                   sessions();
                   sessions(sessions const& s);
  sessions&        operator=(sessions const& s);

  std::map<credentials, session*>
                   _set;
};

CCCS_END()

#endif // !CCCS_SESSIONS_HH
