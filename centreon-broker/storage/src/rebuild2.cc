/*
** Copyright 2021 Centreon
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
**
** For more information : contact@centreon.com
*/

#include "com/centreon/broker/storage/rebuild2.hh"
#include "com/centreon/exceptions/msg_fmt.hh"

using namespace com::centreon::broker;
using namespace com::centreon::broker::storage;

/**
 *  Default constructor.
 */
rebuild2::rebuild2()
    : io::data(rebuild2::static_type()) {}

// Operations.
static io::data* new_rebuild2() {
  return new rebuild2;
}

static std::string serialize_rebuild2(const io::data& e) {
  std::string retval;
  auto r = static_cast<const rebuild2*>(&e);
  if (!r->obj.SerializeToString(&retval))
    throw com::centreon::exceptions::msg_fmt("Unable to serialize rebuild2 object");
  return retval;
}

io::event_info::event_operations const rebuild2::operations = {&new_rebuild2, &serialize_rebuild2};
