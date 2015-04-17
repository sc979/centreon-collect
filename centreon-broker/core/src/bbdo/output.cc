/*
** Copyright 2013-2015 Merethis
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

#include <arpa/inet.h>
#include <cstdio>
#include <memory>
#include <stdint.h>
#include "com/centreon/broker/bbdo/internal.hh"
#include "com/centreon/broker/bbdo/output.hh"
#include "com/centreon/broker/exceptions/msg.hh"
#include "com/centreon/broker/io/event_info.hh"
#include "com/centreon/broker/io/exceptions/shutdown.hh"
#include "com/centreon/broker/io/raw.hh"
#include "com/centreon/broker/logging/logging.hh"
#include "com/centreon/broker/mapping/entry.hh"
#include "com/centreon/broker/io/events.hh"

using namespace com::centreon::broker;
using namespace com::centreon::broker::bbdo;

/**************************************
*                                     *
*          Static Functions           *
*                                     *
**************************************/

/**
 *  Get a boolean from an object.
 */
static void get_boolean(
              io::data const& t,
              mapping::entry const& member,
              QByteArray& buffer) {
  char c(member.get_bool(t) ? 1 : 0);
  buffer.append(&c, 1);
  return ;
}

/**
 *  Get a double from an object.
 */
static void get_double(
              io::data const& t,
              mapping::entry const& member,
              QByteArray& buffer) {
  char str[32];
  size_t strsz(snprintf(str, sizeof(str), "%f", member.get_double(t)) + 1);
  if (strsz > sizeof(str))
    strsz = sizeof(str);
  buffer.append(str, strsz);
  return ;
}

/**
 *  Get an integer from an object.
 */
static void get_integer(
              io::data const& t,
              mapping::entry const& member,
              QByteArray& buffer) {
  uint32_t value(htonl(member.get_int(t)));
  buffer.append(
           static_cast<char*>(static_cast<void*>(&value)),
           sizeof(value));
  return ;
}

/**
 *  Get a short from an object.
 */
static void get_short(
              io::data const& t,
              mapping::entry const& member,
              QByteArray& buffer) {
  uint16_t value(htons(member.get_short(t)));
  buffer.append(
           static_cast<char*>(static_cast<void*>(&value)),
           sizeof(value));
  return ;
}

/**
 *  Get a string from an object.
 */
static void get_string(
              io::data const& t,
              mapping::entry const& member,
              QByteArray& buffer) {
  QByteArray tmp(member.get_string(t).toUtf8());
  buffer.append(tmp.constData(), tmp.size() + 1);
  return ;
}

/**
 *  Get a timestamp from an object.
 */
static void get_timestamp(
              io::data const& t,
              mapping::entry const& member,
              QByteArray& buffer) {
  uint64_t ts(member.get_time(t).get_time_t());
  uint32_t high(htonl(ts / (1ll << 32)));
  uint32_t low(htonl(ts % (1ll << 32)));
  buffer.append(
           static_cast<char*>(static_cast<void*>(&high)),
           sizeof(high));
  buffer.append(
           static_cast<char*>(static_cast<void*>(&low)),
           sizeof(low));
  return ;
}

/**
 *  Get an unsigned integer from an object.
 */
static void get_uint(
              io::data const& t,
              mapping::entry const& member,
              QByteArray& buffer) {
  uint32_t value(htonl(member.get_uint(t)));
  buffer.append(
           static_cast<char*>(static_cast<void*>(&value)),
           sizeof(value));
  return ;
}

/**
 *  Serialize an event in the BBDO protocol.
 *
 *  @param[in] e  Event to serialize.
 *
 *  @return Serialized event.
 */
static io::raw* serialize(io::data const& e) {
  // Get event info (mapping).
  io::event_info const*
    info(io::events::instance().get_event_info(e.type()));
  if (info) {
    // Serialization buffer.
    std::auto_ptr<io::raw> buffer(new io::raw);
    QByteArray& data(*buffer);

    // Reserve space for the BBDO header.
    unsigned int beginning(data.size());
    data.resize(data.size() + BBDO_HEADER_SIZE);
    *(static_cast<uint32_t*>(static_cast<void*>(
                               data.data() + data.size())) - 1)
      = htonl(e.type());

    // Serialize source and destination.
    {
      uint32_t source_id(htonl(e.source_id));
      uint32_t destination_id(htonl(e.destination_id));
      data.append(
               static_cast<char*>(static_cast<void*>(&source_id)),
               sizeof(source_id));
      data.append(
               static_cast<char*>(static_cast<void*>(&destination_id)),
               sizeof(destination_id));
    }

    // Serialize properties of the object.
    for (mapping::entry const* current_entry(info->get_mapping());
         !current_entry->is_null();
         ++current_entry) {
      // Skip entries that should not be serialized.
      if (current_entry->get_serialize())
        switch (current_entry->get_type()) {
        case mapping::source::BOOL:
          get_boolean(e, *current_entry, *buffer);
          break ;
        case mapping::source::DOUBLE:
          get_double(e, *current_entry, *buffer);
          break ;
        case mapping::source::INT:
          get_integer(e, *current_entry, *buffer);
          break ;
        case mapping::source::SHORT:
          get_short(e, *current_entry, *buffer);
          break ;
        case mapping::source::STRING:
          get_string(e, *current_entry, *buffer);
          break ;
        case mapping::source::TIME:
          get_timestamp(e, *current_entry, *buffer);
          break ;
        case mapping::source::UINT:
          get_uint(e, *current_entry, *buffer);
          break ;
        default:
          throw (exceptions::msg() << "BBDO: invalid mapping for object"
                 << " of type '" << info->get_name() << "': "
                 << current_entry->get_type()
                 << " is not a known type ID");
        }

      // Packet splitting.
      if (static_cast<unsigned int>(data.size())
          >= (beginning + BBDO_HEADER_SIZE + 0xFFFF)) {
        // Set size.
        *(static_cast<uint16_t*>(static_cast<void*>(
                                   data.data() + beginning)) + 1)
          = 0xFFFF;

        // Set checksum.
        uint16_t chksum(qChecksum(
                          data.data() + beginning + 2,
                          BBDO_HEADER_SIZE - 2));
        *static_cast<uint16_t*>(static_cast<void*>(
                                  data.data() + beginning))
          = htons(chksum);

        // Create new header.
        beginning += BBDO_HEADER_SIZE + 0xFFFF;
        char header[BBDO_HEADER_SIZE];
        memset(header, 0, sizeof(header));
        *static_cast<uint32_t*>(static_cast<void*>(header + 4))
          = htonl(e.type());
        data.insert(beginning, header, sizeof(header));
      }
    }

    // Set (last) packet size.
    *(static_cast<uint16_t*>(static_cast<void*>(
                               data.data() + beginning)) + 1)
      = htons(data.size() - beginning - BBDO_HEADER_SIZE);

    // Checksum.
    uint16_t chksum(qChecksum(
                      data.data() + beginning + 2,
                      BBDO_HEADER_SIZE - 2));
    *static_cast<uint16_t*>(static_cast<void*>(data.data() + beginning))
      = htons(chksum);

    return (buffer.release());
  }
  else
    logging::info(logging::high)
      << "BBDO: cannot serialize event of ID " << e.type()
      << ": event was not registered and will therefore be ignored";

  return (NULL);
}

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 *  Default constructor.
 */
output::output() : _process_out(true) {}

/**
 *  Copy constructor.
 *
 *  @param[in] other  Object to copy.
 */
output::output(output const& other)
  : io::stream(other), _process_out(other._process_out) {}

/**
 *  Destructor.
 */
output::~output() {}

/**
 *  Assignment operator.
 *
 *  @param[in] other  Object to copy.
 *
 *  @return This object.
 */
output& output::operator=(output const& other) {
  if (this != &other) {
    io::stream::operator=(other);
    _process_out = other._process_out;
  }
  return (*this);
}

/**
 *  Enable or disable output processing.
 *
 *  @param[in] in  Unused.
 *  @param[in] out Set to true to enable output processing.
 */
void output::process(bool in, bool out) {
  (void)in;
  _process_out = out;
  return ;
}

/**
 *  Read data.
 *
 *  @param[out] d Cleared.
 */
void output::read(misc::shared_ptr<io::data>& d) {
  d.clear();
  throw (exceptions::msg()
         << "BBDO: attempt to read from an output object");
  return ;
}

/**
 *  Get statistics.
 *
 *  @param[out] tree Output tree.
 */
void output::statistics(io::properties& tree) const {
  if (!_to.isNull())
    _to->statistics(tree);
  return ;
}

/**
 *  Send an event.
 *
 *  @param[in] e Event to send.
 *
 *  @return Number of events acknowledged.
 */
unsigned int output::write(misc::shared_ptr<io::data> const& e) {
  // Check if data should be processed.
  if (!_process_out)
    throw (io::exceptions::shutdown(true, !_process_out)
           << "BBDO: output stream is shutdown");

  // Check if data exists.
  if (!e.isNull()) {
    misc::shared_ptr<io::raw> serialized(serialize(*e));
    if (serialized.data())
      _to->write(serialized);
  }
  else
    _to->write(e);

  return (1);
}
