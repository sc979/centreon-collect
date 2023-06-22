/*
** Copyright 2014-2016 Centreon
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

#include "com/centreon/broker/bam/bool_equal.hh"
#include "com/centreon/broker/log_v2.hh"

#include <cmath>

using namespace com::centreon::broker::bam;

/**
 *  Get the hard value.
 *
 *  @return Evaluation of the expression with hard values.
 */
double bool_equal::value_hard() {
  bool retval = std::fabs(_left_hard - _right_hard) < COMPARE_EPSILON;
  log_v2::bam()->trace("BAM: bool_equal: {}", retval);
  return retval;
}

/**
 *  Get the soft value.
 *
 *  @return Evaluation of the expression with soft values.
 */
double bool_equal::value_soft() {
  bool retval = std::fabs(_left_soft - _right_soft) < COMPARE_EPSILON;
  log_v2::bam()->trace("BAM: soft bool_equal: {}", retval);
  return retval;
}
