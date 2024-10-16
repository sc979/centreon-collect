/*
** Copyright 2022 Centreon
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

#ifndef CCB_BAM_BA_BEST_HH
#define CCB_BAM_BA_BEST_HH

#include "com/centreon/broker/bam/ba.hh"

namespace com::centreon::broker {

namespace bam {
// Forward declaration.
class kpi;

/**
 *  @class ba ba.hh "com/centreon/broker/bam/ba.hh"
 *  @brief Business activity.
 *
 *  Represents a BA that gets computed every time an impact change
 *  of value.
 */
class ba_best : public ba {
  state _computed_soft_state = state_critical;
  state _computed_hard_state = state_critical;

  void _open_new_event(io::stream* visitor, short service_hard_state);
  void _compute_inherited_downtime(io::stream* visitor);

  void _commit_initial_events(io::stream* visitor);

 protected:
  bool _apply_impact(kpi* kpi_ptr, impact_info& impact) override;
  void _unapply_impact(kpi* kpi_ptr, impact_info& impact) override;
  void _recompute() override;

 public:
  ba_best(uint32_t id,
          uint32_t host_id,
          uint32_t service_id,
          bool generate_virtual_status = true);
  state get_state_hard() const override;
  state get_state_soft() const override;
  std::string get_output() const override;
  std::string get_perfdata() const override;
};
}  // namespace bam

}

#endif  // !CCB_BAM_BA_BEST_HH
