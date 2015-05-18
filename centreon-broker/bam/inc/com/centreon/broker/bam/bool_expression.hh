/*
** Copyright 2014 Merethis
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

#ifndef CCB_BAM_BOOL_EXPRESSION_HH
#  define CCB_BAM_BOOL_EXPRESSION_HH

#  include "com/centreon/broker/bam/computable.hh"
#  include "com/centreon/broker/io/stream.hh"
#  include "com/centreon/broker/namespace.hh"

CCB_BEGIN()

namespace            bam {
  // Forward declaration.
  class              bool_value;

  /**
   *  @class bool_expression bool_expression.hh "com/centreon/broker/bam/bool_expression.hh"
   *  @brief Boolean expression.
   *
   *  Stores and entire boolean expression made of multiple boolean
   *  operations and evaluate them to match the kpi interface.
   */
  class              bool_expression : public computable {
  public:
                     bool_expression();
                     bool_expression(bool_expression const& other);
                     ~bool_expression();
    bool_expression& operator=(bool_expression const& other);
    bool             child_has_update(
                       computable* child,
                       io::stream* visitor = NULL);
    short            get_state() const;
    bool             state_known() const;
    void             set_expression(
                       misc::shared_ptr<bool_value> const& expression);
    void             set_id(unsigned int id);
    void             set_impact_if(bool impact_if);
    void             visit(io::stream* visitor);

  private:
    void             _internal_copy(bool_expression const& right);

    misc::shared_ptr<bool_value>
                     _expression;
    unsigned int     _id;
    bool             _impact_if;
  };
}

CCB_END()

#endif // !CCB_BAM_BOOL_EXPRESSION_HH
