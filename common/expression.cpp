/*
 * Copyright (c) 2011 Wei Song <songw@cs.man.ac.uk> 
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
 * 08/02/2011   Wei Song
 *
 *
 */

#include "expression.h"

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

int netlist::Expression::get_value() const { 
  if(valuable &&                                   // valuable 
     1 == eqn.size() &&                            // and has only one element
     eqn.front().is_valuable() &&                  // and the element is valuable
     Operation::oNum == eqn.front().get_type())   // and it is a number
    return eqn.front().get_num()->get_value(); 
  else
    return 0;
}

void netlist::Expression::append(Operation::operation_t otype) {
  assert(tExp == ctype);     // this object whould be valid
  assert(otype >= Operation::oUPos && otype < Operation::oPower);

  // preparing operands
  this->execute();

  // connect the equation
  this->eqn.push_front(Operation(otype));

  // try to reduce the final equation
  this->execute();
}

void netlist::Expression::execute() {
  std::list<Operation> m_eqn;   // the new equation
  stack<Operation&> m_stack;
  enum state_t {sInit, sOp, sD1, sD2, sD3};
  state_t m_state = sInit;
  Operation *pd1, *pd2, *pd3;
  bool m_valuable = true;       // assuming reductable
  bool m_reduce_again = false;

  while(!eqn.empty) {
    // move an operation from old to new equation, then check the operation according to last state
    m_eqn.splice(m_eqn.end(), eqn, eqn.begin());
    while(!reduce_again) {
      switch(m_state) {
      case sInit:
        if(m_eqn.back().get_type() <= Operation::oFun) { // already reduced
          valuable = m_eqn.back().is_valuable();
          assert(eqn.empty());    // it should be empty now
          m_reduce_again = false;
          break;
        } else {                  // operator
          m_state = sOp;
          m_stack.push(m_eqn.back());
          m_reduce_again = false;
          break;
        }
    case sOp:
      if(m_eqn.back().get_type() <= Operation::oFun) { // first operand is a number/variable
        if(m_stack.top().get_type() <= Operation::oUNxor) { // unary operation
          
    case sD1:
    case sD2:
    case sD3:
    default:
    }
  }

}

void netlist::Expression::append(Operation::operation_t otype, Expression& d1) {
  assert(tExp == ctype);     // this object whould be valid
  assert(otype >= Operation::oPower && otype < Operation::oQuestion);

  // preparing operands
  this->execute();
  d1.execute();

  // connect the equation
  this->eqn.push_front(Operation(otype));
  this->eqn.splice(this->eqn.end(), d1.eqn);
  
  // try to reduce the final equation
  this->execute();

}

void netlist::Expression::append(Operation::operation_t otype, Expression& d1, Expression& d2) {
  assert(tExp == ctype);     // this object whould be valid
  assert(otype >= Operation::oQuestion);

  // preparing operands
  this->execute();
  d1.execute();
  d2.execute();

  this->eqn.push_front(Operation(otype));
  this->eqn.splice(this->eqn.end(), d1.eqn);
  this->eqn.splice(this->eqn.end(), d2.eqn);
  
  // try to reduce the final equation
  this->execute();
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
