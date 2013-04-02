/*
** Copyright 2011-2013 Merethis
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

#include "com/centreon/broker/exceptions/msg.hh"
#include "com/centreon/broker/ndo/acceptor.hh"
#include "com/centreon/broker/ndo/connector.hh"
#include "com/centreon/broker/ndo/factory.hh"

using namespace com::centreon::broker;
using namespace com::centreon::broker::ndo;

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 *  Default constructor.
 */
factory::factory() {}

/**
 *  Copy constructor.
 *
 *  @param[in] f Object to copy.
 */
factory::factory(factory const& f) : io::factory(f) {}

/**
 *  Destructor.
 */
factory::~factory() {}

/**
 *  Assignment operator.
 *
 *  @param[in] f Object to copy.
 *
 *  @return This object.
 */
factory& factory::operator=(factory const& f) {
  io::factory::operator=(f);
  return (*this);
}

/**
 *  Clone the factory object.
 *
 *  @return Clone of this factory.
 */
io::factory* factory::clone() const {
  return (new factory(*this));
}

/**
 *  Check if a configuration supports this protocol.
 *
 *  @param[in] cfg       Object configuration.
 *  @param[in] is_input  true if endpoint must act as event source.
 *  @param[in] is_output true if endpoint must act as event destination.
 *
 *  @return true if the configuration has this protocol.
 */
bool factory::has_endpoint(
                config::endpoint& cfg,
                bool is_input,
                bool is_output) const {
  (void)is_input;
  (void)is_output;
  QMap<QString, QString>::const_iterator it(cfg.params.find("protocol"));
  return ((it != cfg.params.end()) && (it.value() == "ndo"));
}

/**
 *  Create a new endpoint from a configuration.
 *
 *  @param[in]  cfg Endpoint configuration.
 *  @param[in]  is_input    true if endpoint must act as event source.
 *  @param[in]  is_output   true if endpoint must act as event destination.
 *  @param[in]  temporary   Temporary stream to write data when memory
 *                          queue is full.
 *  @param[out] is_acceptor Set to true if the endpoint is an acceptor.
 *
 *  @return Endpoint matching configuration.
 */
io::endpoint* factory::new_endpoint(
                         config::endpoint& cfg,
                         bool is_input,
                         bool is_output,
                         io::endpoint const* temporary,
                         bool& is_acceptor) const {
  (void)cfg;
  io::endpoint* retval(NULL);
  if (is_acceptor)
    retval = new ndo::acceptor(is_output, temporary);
  else
    retval = new ndo::connector(is_input, is_output);
  return (retval);
}
