/*
 * Copyright (c) 2012 Wei Song <songw@cs.man.ac.uk> 
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
 * Expressions
 * 08/02/2012   Wei Song
 *
 *
 */

#include<stack>
#include "component.h"

using namespace netlist;

netlist::Expression::Expression()
  : NetComp(tUnkown), valuable(false)
{}

netlist::Expression::Expression(const Number& exp) 
  : NetComp(tExp), valuable(exp.is_valuable())
{
  boost::shared_ptr<Number> num(new Number(exp));
  eqn.push_back(Operation(num));
}

bool netlist::Expression::is_valuable() const {
  return valuable;
}

Number netlist::Expression::get_value() const { 
  if(valuable &&                                   // valuable 
     1 == eqn.size() &&                            // and has only one element
     eqn.front().is_valuable() &&                  // and the element is valuable
     Operation::oNum == eqn.front().get_type())   // and it is a number
    return eqn.front().get_num(); 
  else
    return 0;
}

void netlist::Expression::reduce() {
  // state stack
  std::stack<boost::shared_ptr<expression_state> > m_stack;

  while(!eqn.empty()) {
    // fetch a new operation element
    Operation& it = eqn.front();
    eqn.pop_front();
    
    // check the operation
    if(it.get_type() <= Operation::oFun) {   // a prime
      if(m_stack.empty()) {     // only one element and it is a prime
        assert(eqn.empty());    // eqn should be empty
        eqn.push_back(it);
        valuable = it.is_valuable();
        return;
      } else {
        boost::shared_ptr<expression_state> m_state = m_stack.top();
        m_state->d[(m_state->opp)++].push_back(it);
        while(true) {
          if(m_state->ops == m_state->opp) { // ready for execute
            execute_operation(m_state->op.get_type(), m_state->d[0], m_state->d[3], m_state->d[2]);
            m_stack.pop();
            // recursive iterations
            if(m_stack.empty()) { // final
              assert(eqn.empty());    // eqn should be empty
              eqn.splice(eqn.end(), m_state->d[0]);
              valuable = eqn.front().is_valuable();
              //delete m_state;
              return;
            } else {
              boost::shared_ptr<expression_state> tmp = m_state;
              m_state = m_stack.top();
              m_state->d[m_state->opp].splice(m_state->d[m_state->opp].end(), tmp->d[0]);
              //delete tmp;
              (m_state->opp)++;
            }
          } else break;       // do nothing, proceed to the next element
        }
      }
    } else {                  // an operator
      boost::shared_ptr<expression_state> m_state;
      m_state->op = it;
      if(it.get_type() <= Operation::oUNxor) { // unary
          m_state->ops = 1;
      } else if(it.get_type() <= Operation::oLOr) { // two operands
        m_state->ops = 2;
      } else {
        m_state->ops = 3;
      }
      m_stack.push(m_state);
    }
  }

  // should not run to here
  assert(1 == 0);
}

void netlist::Expression::append(Operation::operation_t otype) {
  assert(tExp == ctype);     // this object whould be valid
  assert(otype >= Operation::oUPos && otype < Operation::oPower);

  // preparing operands
  this->reduce();

  // connect the equation
  this->eqn.push_front(Operation(otype));

  // try to reduce the final equation
  this->reduce();
}

void netlist::Expression::append(Operation::operation_t otype, Expression& d1) {
  assert(tExp == ctype);     // this object whould be valid
  assert(otype >= Operation::oPower && otype < Operation::oQuestion);

  // preparing operands
  this->reduce();
  d1.reduce();

  // connect the equation
  this->eqn.push_front(Operation(otype));
  this->eqn.splice(this->eqn.end(), d1.eqn);
  
  // try to reduce the final equation
  this->reduce();

}

void netlist::Expression::append(Operation::operation_t otype, Expression& d1, Expression& d2) {
  assert(tExp == ctype);     // this object whould be valid
  assert(otype >= Operation::oQuestion);

  // preparing operands
  this->reduce();
  d1.reduce();
  d2.reduce();

  this->eqn.push_front(Operation(otype));
  this->eqn.splice(this->eqn.end(), d1.eqn);
  this->eqn.splice(this->eqn.end(), d2.eqn);
  
  // try to reduce the final equation
  this->reduce();
}

bool netlist::Expression::operator== (const Expression& rhs) const {
  return false;
}

Expression netlist::operator+ (const Expression& lhs, const Expression& rhs) {
  return Expression();
}

Expression netlist::operator- (const Expression& lhs, const Expression& rhs) {
  return Expression();
}

std::ostream& netlist::Expression::streamout(std::ostream& os) const {
  os << "expression";
  return os;
}