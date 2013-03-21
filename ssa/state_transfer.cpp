/*
 * Copyright (c) 2011-2013 Wei Song <songw@cs.man.ac.uk> 
 *    Advanced Processor Technologies Group, School of Computer Science
 *    University of Manchester, Manchester M13 9PL UK
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

/* 
 * Record of a state transfer condition and its target state
 * 19/03/2013   Wei Song
 *
 *
 */

#include "netlist/component.h"
#include "state_transfer.hpp"
#include <boost/foreach.hpp>

using namespace SSA;
using namespace netlist;
using boost::shared_ptr;
using std::pair;
using std::map;
using std::string;

SSA::StateTransfer::StateTransfer()
  : type(SSA_CONST), cname("state") {}


SSA::StateTransfer::StateTransfer(const string& cn)
  : type(SSA_CONST), cname(cn) {}

SSA::StateTransfer::StateTransfer(const string& cn, st_type_t tt, const Number& nstate, 
                                  const shared_ptr<Expression>& expr)
  : type(tt), condition(expr->deep_copy()), cname(cn)
{
  switch(tt) {
  case SSA_CONST: next_state = nstate; break;
  case SSA_DELTA: next_delta = nstate; break;
  default:
    assert(0 == "wrong state transfer type");
  }
}

SSA::StateTransfer::StateTransfer(const string& cn, shared_ptr<Expression> next_expr, 
                                  const shared_ptr<Expression>& expr) 
  : type(SSA_EXP), condition(expr->deep_copy()), cname(cn)
{
  next_exp.reset(next_expr->deep_copy());
}

void SSA::StateTransfer::set_next_state(const Number& nstate) {
  type = SSA_CONST;
  next_state = nstate;
}

void SSA::StateTransfer::set_next_delta(const Number& ndelta) {
  type = SSA_DELTA;
  next_delta = ndelta;  
}

void SSA::StateTransfer::set_next_exp(shared_ptr<Expression> exp) {
  type = SSA_EXP;
  next_exp.reset(exp->deep_copy());
}

shared_ptr<Expression> SSA::StateTransfer::get_next_exp() const {
  return shared_ptr<Expression>(next_exp->deep_copy());
}

// core function, to calculate the next state and for state traversal
pair<bool, Number> SSA::StateTransfer::get_next_state(const SSA_ENV& senv, const Number& cstate) const{
  pair<bool, Number> rv(false, Number()); // return value
  // get a copy of the condition expression
  shared_ptr<Expression> m_exp(condition->deep_copy());
  // replace all variables in the environment
  BOOST_FOREACH(SSA_ENV_TYPE r, senv) {
    m_exp->replace_variable(r.first, r.second);
  }
  m_exp->reduce();
  
  if(!m_exp->is_valuable() || !m_exp->get_value().is_true())
    return rv;
  
  switch(type) {
  case SSA_CONST: rv.first = true; rv.second = next_state; break;
  case SSA_DELTA: rv.first = true; rv.second = cstate + next_delta; break;
  case SSA_EXP: {
    // copy next_exp
    m_exp.reset(next_exp->deep_copy());
    // replace environment
    BOOST_FOREACH(SSA_ENV_TYPE r, senv) {
      m_exp->replace_variable(r.first, r.second);
    }
    // replace current state
    m_exp->replace_variable(VIdentifier(cname), cstate);
    if(m_exp->is_valuable() && m_exp->get_value().is_true()) {
      rv.first = true; rv.second = m_exp->get_value();
    }
    break;
  }
  default:
    assert(0 == "wrong state transfer type");
  }
  return rv;
}


void SSA::StateTransfer::append_condition(const Operation& op, shared_ptr<Expression> exp) {
  condition->append(op.get_type(), *exp);
}

std::ostream& SSA::StateTransfer::streamout (std::ostream& os) const {
  switch(type) {
  case SSA_CONST: os << "<" << next_state; break;
  case SSA_DELTA: os << "<" << cname << " + " << next_delta; break;
  case SSA_EXP:   os << next_exp; break;
  default: assert(0 == "wrong state transfer type");
  }
  os << ", " << condition << ">";
  return os;
}
