/**
* Copyright 2009-2013 Centreon
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* For more information : contact@centreon.com
*/

#include "com/centreon/broker/tls/connector.hh"

#include "com/centreon/broker/log_v2.hh"
#include "com/centreon/broker/tls/internal.hh"
#include "com/centreon/broker/tls/params.hh"
#include "com/centreon/broker/tls/stream.hh"
#include "com/centreon/exceptions/msg_fmt.hh"

using namespace com::centreon::broker;
using namespace com::centreon::broker::tls;
using namespace com::centreon::exceptions;

/**************************************
 *                                     *
 *           Public Methods            *
 *                                     *
 **************************************/

/**
 *  Default constructor
 *
 *  @param[in] cert Certificate.
 *  @param[in] key  Key file.
 *  @param[in] ca   Trusted CA's certificate.
 */
connector::connector(std::string const& cert,
                     std::string const& key,
                     std::string const& ca,
                     std::string const& tls_hostname)
    : io::endpoint(false, {}),
      _ca(ca),
      _cert(cert),
      _key(key),
      _tls_hostname(tls_hostname) {}

/**
 *  Connect to the remote TLS peer.
 *
 *  @return New connected stream.
 */
std::shared_ptr<io::stream> connector::open() {
  // First connect the lower layer.
  std::shared_ptr<io::stream> lower(_from->open());
  if (lower)
    return open(std::move(lower));
  return nullptr;
}

/**
 *  Overload of open, using base stream.
 *
 *  @param[in] lower Open stream.
 *
 *  @return Encrypted stream.
 */
std::shared_ptr<io::stream> connector::open(std::shared_ptr<io::stream> lower) {
  std::shared_ptr<io::stream> u;
  if (lower) {
    int ret;
    // Load parameters.
    params p(params::CLIENT);
    p.set_cert(_cert, _key);
    p.set_trusted_ca(_ca);
    p.set_tls_hostname(_tls_hostname);
    p.load();

    gnutls_session_t* session(new gnutls_session_t);
    try {
      // Initialize the TLS session
      log_v2::tls()->debug("TLS: initializing session");
#ifdef GNUTLS_NONBLOCK
      ret = gnutls_init(session, GNUTLS_CLIENT | GNUTLS_NONBLOCK);
#else
      ret = gnutls_init(session, GNUTLS_CLIENT);
#endif  // GNUTLS_NONBLOCK
      if (ret != GNUTLS_E_SUCCESS) {
        log_v2::tls()->error("TLS: cannot initialize session: {}",
                             gnutls_strerror(ret));
        throw msg_fmt("TLS: cannot initialize session: {} ",
                      gnutls_strerror(ret));
      }

      // Apply TLS parameters to the current session.
      p.apply(*session);

      // Create stream object.
      u.reset(new stream(session));
    } catch (...) {
      gnutls_deinit(*session);
      delete session;
      throw;
    }
    u->set_substream(lower);

    // Bind the TLS session with the stream from the lower layer.
#if GNUTLS_VERSION_NUMBER < 0x020C00
    gnutls_transport_set_lowat(*session, 0);
#endif  // GNU TLS < 2.12.0
    gnutls_transport_set_pull_function(*session, pull_helper);
    gnutls_transport_set_push_function(*session, push_helper);
    gnutls_transport_set_ptr(*session, u.get());

    // Perform the TLS handshake.
    log_v2::tls()->debug("TLS: performing handshake");
    do {
      ret = gnutls_handshake(*session);
    } while (GNUTLS_E_AGAIN == ret || GNUTLS_E_INTERRUPTED == ret);
    if (ret != GNUTLS_E_SUCCESS) {
      log_v2::tls()->error("TLS: handshake failed: {}", gnutls_strerror(ret));
      throw msg_fmt("TLS: handshake failed: {}", gnutls_strerror(ret));
    }

    log_v2::tls()->debug("TLS: successful handshake");
    gnutls_protocol_t prot = gnutls_protocol_get_version(*session);
    gnutls_cipher_algorithm_t ciph = gnutls_cipher_get(*session);
    log_v2::tls()->debug("TLS: protocol and cipher  {} {} used",
                         gnutls_protocol_get_name(prot),
                         gnutls_cipher_get_name(ciph));

    // Check certificate if necessary.
    p.validate_cert(*session);
  }

  return u;
}
