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

#include <assert.h>
#include <stdlib.h>
#include "exceptions/basic.hh"
#include "tcp/acceptor.hh"
#include "tcp/stream.hh"

using namespace com::centreon::broker;
using namespace com::centreon::broker::tcp;

/**************************************
*                                     *
*           Private Methods           *
*                                     *
**************************************/

/**
 *  @brief Copy constructor.
 *
 *  Any call to this constructor will result in a call to abort().
 *
 *  @param[in] a Object to copy.
 */
acceptor::acceptor(acceptor const& a) : io::acceptor(a) {
  assert(false);
  abort();
}

/**
 *  @brief Assignment operator.
 *
 *  Any call to this method will result in a call to abort().
 *
 *  @param[in] a Object to copy.
 *
 *  @return This object.
 */
acceptor& acceptor::operator=(acceptor const& a) {
  (void)a;
  assert(false);
  abort();
  return (*this);
}

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 *  Default constructor.
 */
acceptor::acceptor() : _port(0) {}

/**
 *  Destructor.
 */
acceptor::~acceptor() {}

/**
 *  @brief Accept an incoming connection.
 *
 *  This method should not be called, as TCP directly handles connection
 *  acception.
 *
 *  @param[in] ptr Unused.
 */
void acceptor::accept(QSharedPointer<com::centreon::broker::io::stream> ptr) {
  (void)ptr;
  throw (exceptions::basic() << "attempt make a TCP acceptor accept " \
           "an incoming connection from a lower layer (this is a software bug)");
  return ;
}

/**
 *  Close the acceptor.
 */
void acceptor::close() {
  _socket.close();
  return ;
}

/**
 *  Set the port on which the acceptor will listen.
 *
 *  @param[in] port Port on which the acceptor will listen.
 */
void acceptor::listen_on(unsigned short port) {
  _port = port;
  // Listen on port.
  if (!_socket.listen(QHostAddress::Any, _port))
    throw (exceptions::basic() << "could not listen on port " << _port
             << ": " << _socket.errorString().toStdString().c_str());
  return ;
}

/**
 *  Start connection acception.
 */
QSharedPointer<io::stream> acceptor::open() {
  // Wait for incoming connections.
  if (!_socket.waitForNewConnection(-1))
    throw (exceptions::basic() << "could not accept incoming TCP client: "
             << _socket.errorString().toStdString().c_str());

  // Accept client.
  QSharedPointer<QTcpSocket> incoming(_socket.nextPendingConnection());
  if (incoming.isNull())
    throw (exceptions::basic() << "could not accept incoming TCP client: "
             << _socket.errorString().toStdString().c_str());

  // Forward object.
  if (!_down.isNull()) {
    QSharedPointer<io::stream> new_client(new stream(incoming));
    _down->accept(new_client);
  }

  return (QSharedPointer<io::stream>());
}
