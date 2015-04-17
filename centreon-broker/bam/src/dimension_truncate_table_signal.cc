/*
** Copyright 2014-2015 Merethis
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

#include "com/centreon/broker/bam/dimension_truncate_table_signal.hh"
#include "com/centreon/broker/bam/internal.hh"
#include "com/centreon/broker/io/events.hh"

using namespace com::centreon::broker;
using namespace com::centreon::broker::bam;

/**
 *  Default constructor.
 */
dimension_truncate_table_signal::dimension_truncate_table_signal()
  : update_started(true) {}

/**
 *  Copy constructor.
 *
 *  @param[in] other  Object to copy.
 */
dimension_truncate_table_signal::dimension_truncate_table_signal(
    dimension_truncate_table_signal const& other)
  : io::data(other) {
  _internal_copy(other);
}

/**
 *  Destructor.
 */
dimension_truncate_table_signal::~dimension_truncate_table_signal() {}

/**
 *  Assignment operator.
 *
 *  @param[in] other  Object to copy.
 *
 *  @return This object.
 */
dimension_truncate_table_signal& dimension_truncate_table_signal::operator=(
                                                                    dimension_truncate_table_signal const& other) {
  if (this != &other) {
    io::data::operator=(other);
    _internal_copy(other);
  }
  return (*this);
}

/**
 *  Equality test operator.
 *
 *  @param[in] other  The object to test for equality.
 *
 *  @return  True if the two objects are equal.
 */
bool dimension_truncate_table_signal::operator==(
                                        dimension_truncate_table_signal const& other) const {
  return (update_started == other.update_started);
}

/**
 *  Get the event type.
 *
 *  @return Event type.
 */
unsigned int dimension_truncate_table_signal::type() const {
  return (dimension_truncate_table_signal::static_type());
}

/**
 *  Get the event type.
 *
 *  @return Event type.
 */
unsigned int dimension_truncate_table_signal::static_type() {
  return (io::events::data_type<io::events::bam,
                                bam::de_dimension_truncate_table_signal>::value);
}

/**
 *  Copy internal data members.
 *
 *  @param[in] other Object to copy.
 */
void dimension_truncate_table_signal::_internal_copy(
    dimension_truncate_table_signal const& other) {
  update_started = other.update_started;
  return ;
}

/**************************************
*                                     *
*           Static Objects            *
*                                     *
**************************************/

// Mapping.
mapping::entry const dimension_truncate_table_signal::entries[] = {
  mapping::entry(
    &bam::dimension_truncate_table_signal::update_started,
    "update_started"),
  mapping::entry()
};

// Operations.
static io::data* new_dimension_truncate_table_signal() {
  return (new dimension_truncate_table_signal);
}
io::event_info::event_operations const dimension_truncate_table_signal::operations = {
  &new_dimension_truncate_table_signal
};
