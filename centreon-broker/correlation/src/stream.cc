/*
** Copyright 2015 Merethis
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

#include "com/centreon/broker/correlation/stream.hh"
#include "com/centreon/broker/exceptions/msg.hh"
#include "com/centreon/broker/io/exceptions/shutdown.hh"
#include "com/centreon/broker/persistent_cache.hh"
#include "com/centreon/broker/correlation/engine_state.hh"
#include "com/centreon/broker/correlation/node.hh"
#include "com/centreon/broker/correlation/parser.hh"
#include "com/centreon/broker/neb/host_status.hh"
#include "com/centreon/broker/neb/service_status.hh"
#include "com/centreon/broker/neb/log_entry.hh"
#include "com/centreon/broker/notification/acknowledgement.hh"
#include "com/centreon/broker/notification/downtime.hh"
#include "com/centreon/broker/notification/downtime_removed.hh"
#include "com/centreon/broker/multiplexing/publisher.hh"
#include "com/centreon/broker/logging/logging.hh"

extern unsigned int instance_id;

using namespace com::centreon::broker;
using namespace com::centreon::broker::correlation;

/**
 *  Constructor.
 *
 *  @param[in]      correlation_file  Correlation file.
 *  @param[int,out] cache             Persistent cache.
 */
stream::stream(
          QString const& correlation_file,
	  misc::shared_ptr<persistent_cache> cache)
  : _correlation_file(correlation_file),
    _process_out(true),
    _cache(cache) {
  // Create the engine started event.
  multiplexing::publisher pblsh;
  misc::shared_ptr<engine_state> es(new engine_state);
  es->instance_id = instance_id;
  es->started = true;
  pblsh.write(es);

  // Load the correlation.
  _load_correlation();
}

/**
 *  Destructor.
 */
stream::~stream() {
  try {
    multiplexing::publisher pblsh;
    misc::shared_ptr<engine_state> es(new engine_state);
    es->instance_id = instance_id;
    pblsh.write(es);
  } catch (std::exception const& e) {
    logging::error(logging::medium)
      << "correlator: error while trying to publish engine state: "
      << e.what();
  }
}

/**
 *  Set which data to process.
 *
 *  @param[in] in   Process in.
 *  @param[in] out  Process out.
 */
void stream::process(bool in, bool out) {
  bool was_processing(_process_out);
  _process_out = in || !out; // Only for immediate shutdown.
  if (was_processing && !_process_out)
    _save_persistent_cache();
  return ;
}

/**
 *  Read data from the stream.
 *
 *  @param[out] d  Unused.
 */
void stream::read(misc::shared_ptr<io::data>& d) {
  d.clear();
  throw (exceptions::msg()
	 << "correlation: cannot read from a stream. This is likely a "
	 << "software bug that you should report to Centreon Broker "
	 << "developers");
  return ;
}

/**
 *  Update the stream.
 */
void stream::update() {
  _save_persistent_cache();
  _load_correlation();
  return ;
}

/**
 *  Write data to the correlation stream.
 *
 *  @param[in] d  Multiplexed data.
 */
unsigned int stream::write(misc::shared_ptr<io::data> const& d) {
  // Check that data can be processed.
  if (!_process_out)
    throw (io::exceptions::shutdown(true, true)
	   << "correlation stream is shutdown");

  if (d.isNull())
    return (1);

  if (d->type() == neb::host_status::static_type()) {
    neb::host_status const& hs = d.ref_as<neb::host_status>();
    QPair<unsigned int, unsigned int> id(hs.host_id, 0);
    QMap<QPair<unsigned int, unsigned int>, node>::iterator found
      = _nodes.find(id);
    if (found != _nodes.end()) {
      multiplexing::publisher pblsh;
      found->manage_status(
        hs.last_hard_state,
        hs.last_hard_state_change,
        &pblsh);
    }
  }
  else if (d->type() == neb::service_status::static_type()) {
    neb::service_status const& ss = d.ref_as<neb::service_status>();
    QPair<unsigned int, unsigned int> id(ss.host_id, ss.service_id);
    QMap<QPair<unsigned int, unsigned int>, node>::iterator found
      = _nodes.find(id);
    if (found != _nodes.end()) {
      multiplexing::publisher pblsh;
      found->manage_status(
        ss.last_hard_state,
        ss.last_hard_state_change,
        &pblsh);
    }
  }
  else if (d->type() == notification::acknowledgement::static_type()) {
    notification::acknowledgement const& ack
      = d.ref_as<notification::acknowledgement>();
    QPair<unsigned int, unsigned int> id(ack.host_id, ack.service_id);
    QMap<QPair<unsigned int, unsigned int>, node>::iterator found
      = _nodes.find(id);
    if (found != _nodes.end()) {
      multiplexing::publisher pblsh;
      found->manage_ack(ack.entry_time, &pblsh);
    }
  }
  else if (d->type() == notification::downtime::static_type()) {
    notification::downtime const& dwn = d.ref_as<notification::downtime>();
    QPair<unsigned int, unsigned int> id(dwn.host_id, dwn.service_id);
    QMap<QPair<unsigned int, unsigned int>, node>::iterator found
      = _nodes.find(id);
    if (found != _nodes.end()) {
      multiplexing::publisher pblsh;
      found->manage_downtime(dwn.start_time, &pblsh);
    }
  }
  else if (d->type() == neb::log_entry::static_type()) {
    neb::log_entry const& entry = d.ref_as<neb::log_entry>();
    QPair<unsigned int, unsigned int> id(entry.host_id, entry.service_id);
    QMap<QPair<unsigned int, unsigned int>, node>::iterator found
      = _nodes.find(id);
    if (found != _nodes.end()) {
      multiplexing::publisher pblsh;
      found->manage_log(entry, &pblsh);
    }
  }
}

/**
 *  Load correlation from the configuration file and the persistent
 *  cache.
 */
void stream::_load_correlation() {
  parser p;
  p.parse(_correlation_file, _nodes);

  // No cache, nothing to do.
  if (_cache.isNull())
    return ;
}

/**
 *  Save content of the correlation memory in the persistent cache.
 */
void stream::_save_persistent_cache() {
  // No cache, nothing to do.
  if (_cache.isNull())
    return ;

}
