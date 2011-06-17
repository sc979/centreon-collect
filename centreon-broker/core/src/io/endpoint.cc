/*
** Copyright 2011 Merethis
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

#include "com/centreon/broker/io/endpoint.hh"

using namespace com::centreon::broker::io;

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 *  Default constructor.
 */
endpoint::endpoint() {}

/**
 *  Copy constructor.
 *
 *  @param[in] e Object to copy.
 */
endpoint::endpoint(endpoint const& e) : _from(e._from) {}

/**
 *  Destructor.
 */
endpoint::~endpoint() {}

/**
 *  Assignment operator.
 *
 *  @param[in] e Object to copy.
 *
 *  @return This object.
 */
endpoint& endpoint::operator=(endpoint const& e) {
  _from = e._from;
  return (*this);
}

/**
 *  Set the lower layer endpoint object of this endpoint.
 *
 *  @param[in] endp Lower layer endpoint object.
 */
void endpoint::from(QSharedPointer<endpoint> endp) {
  _from = endp;
  return ;
}
