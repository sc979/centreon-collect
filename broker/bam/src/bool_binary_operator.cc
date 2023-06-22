/*
** Copyright 2014-2016, 2021 Centreon
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

#include "com/centreon/broker/bam/bool_binary_operator.hh"
#include "com/centreon/broker/log_v2.hh"

using namespace com::centreon::broker::bam;

static constexpr double eps = 0.000001;

/**
 *  Default constructor.
 */
bool_binary_operator::bool_binary_operator()
    : _left_hard(0.0),
      _left_soft(0.0),
      _right_hard(0.0),
      _right_soft(0.0),
      _state_known(false),
      _in_downtime(false) {}

/**
 *  Notification of child update.
 *
 *  @param[in] child     Child that got updated.
 *  @param[out] visitor  Visitor.
 *
 *  @return              True if the values of this object were modified.
 */
bool bool_binary_operator::child_has_update(computable* child,
                                            io::stream* visitor) {
  (void)visitor;
  bool retval = false;

  // Check operation members values.
  if (child) {
    if (child == _left.get()) {
      double value_hard(_left->value_hard());
      double value_soft(_left->value_soft());
      if (std::abs(_left_hard - value_hard) > ::eps ||
          std::abs(_left_soft - value_soft) > ::eps) {
        SPDLOG_LOGGER_TRACE(log_v2::bam(),
                            "{}::child_has_update old_soft_left={} "
                            "old_hard_left={}, new soft_left={}, new "
                            "hard_left={}, soft_right={}, hard_right={}",
                            typeid(*this).name(), _left_soft, _left_hard,
                            value_soft, value_hard, _right_soft, _right_hard);
        _left_hard = value_hard;
        _left_soft = value_soft;
        retval = true;
      }
    } else if (child == _right.get()) {
      double value_hard(_right->value_hard());
      double value_soft(_right->value_soft());
      if (std::abs(_right_hard - value_hard) > ::eps ||
          std::abs(_right_soft - value_soft) > ::eps) {
        SPDLOG_LOGGER_TRACE(log_v2::bam(),
                            "{}::child_has_update old_soft_right={} "
                            "old_hard_right={}, new soft_right={}, new "
                            "hard_right={}, soft_left={}, hard_left={}",
                            typeid(*this).name(), _right_soft, _right_hard,
                            value_soft, value_hard, _left_soft, _left_hard);
        _right_hard = value_hard;
        _right_soft = value_soft;
        retval = true;
      }
    }
    return retval;
  }

  // Check known flag.
  bool known = state_known();
  if (_state_known != known) {
    _state_known = known;
    retval = true;
  }

  // Check downtime flag.
  bool in_dt = in_downtime();
  if (_in_downtime != in_dt) {
    _in_downtime = in_dt;
    retval = true;
  }

  return retval;
}

/**
 *  Set left member.
 *
 *  @param[in] left Left member of the boolean operator.
 */
void bool_binary_operator::set_left(std::shared_ptr<bool_value> const& left) {
  _left = left;
  _left_hard = _left->value_hard();
  _left_soft = _left->value_soft();
  _state_known = state_known();
  _in_downtime = in_downtime();
}

/**
 *  Set right member.
 *
 *  @param[in] right Right member of the boolean operator.
 */
void bool_binary_operator::set_right(std::shared_ptr<bool_value> const& right) {
  _right = right;
  _right_hard = _right->value_hard();
  _right_soft = _right->value_soft();
  _state_known = state_known();
  _in_downtime = in_downtime();
}

/**
 *  Get if the state is known, i.e has been computed at least once.
 *
 *  @return  True if the state is known.
 */
bool bool_binary_operator::state_known() const {
  bool retval =
      _left && _right && _left->state_known() && _right->state_known();
  log_v2::bam()->debug("BAM: bool binary operator: state known {}", retval);
  return retval;
}

/**
 *  Is this expression in downtime?
 *
 *  @return  True if this expression is in downtime.
 */
bool bool_binary_operator::in_downtime() const {
  return (_left && _left->in_downtime()) || (_right && _right->in_downtime());
}
