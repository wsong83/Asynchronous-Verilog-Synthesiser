/*
 * Copyright (c) 2011-2012 Wei Song <songw@cs.man.ac.uk> 
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

#include "component.h"

using namespace netlist;

netlist::Expression::Expression(const Number& exp) 
  : NetComp(tExp), valuable(exp.is_valuable())
{
  eqn.push_back(shared_ptr<Operation>( new Operation(exp)));
}

netlist::Expression::Expression(const location& lloc, const Number& exp) 
  : NetComp(tExp, lloc), valuable(exp.is_valuable())
{
  eqn.push_back(shared_ptr<Operation>( new Operation(exp)));
}

netlist::Expression::Expression(const VIdentifier& id) 
  : NetComp(tExp), valuable(false)
{
  eqn.push_back(shared_ptr<Operation>( new Operation(id)));
}

netlist::Expression::Expression(const location& lloc, const VIdentifier& id) 
  : NetComp(tExp, lloc), valuable(false)
{
  eqn.push_back(shared_ptr<Operation>( new Operation(id)));
}

netlist::Expression::Expression(const shared_ptr<Concatenation>& con) 
  : NetComp(tExp), valuable(false)
{
  eqn.push_back(shared_ptr<Operation>( new Operation(con)));
}

netlist::Expression::Expression(const location& lloc, const shared_ptr<Concatenation>& con) 
  : NetComp(tExp, lloc), valuable(false)
{
  eqn.push_back(shared_ptr<Operation>( new Operation(con)));
}

netlist::Expression::Expression(const shared_ptr<LConcatenation>& con)
  : NetComp(tExp), valuable(false)
{
  eqn.push_back(shared_ptr<Operation>(new Operation(con)));
}

netlist::Expression::Expression(const location& lloc, const shared_ptr<LConcatenation>& con)
  : NetComp(tExp, lloc), valuable(false)
{
  eqn.push_back(shared_ptr<Operation>(new Operation(con)));
}

bool netlist::Expression::is_valuable() const {
  return valuable;
}

Number netlist::Expression::get_value() const { 
  if(valuable &&                                   // valuable 
     1 == eqn.size() &&                            // and has only one element
     eqn.front()->is_valuable() &&                  // and the element is valuable
     Operation::oNum == eqn.front()->get_type())   // and it is a number
      return eqn.front()->get_num();
  else
    return 0;
}

void netlist::Expression::reduce() {
  // state stack
  stack<shared_ptr<expression_state> > m_stack;

  if(valuable) return;          // fast return path

  while(!eqn.empty()) {
    // fetch a new operation element
    shared_ptr<Operation> m = eqn.front();
    eqn.pop_front();
    
    // check the operation
    if(m->get_type() <= Operation::oFun) {   // a prime
      if(m_stack.empty()) {     // only one element and it is a prime
        assert(eqn.empty());    // eqn should be empty
        m->reduce();
        eqn.push_back(m);
        valuable = m->is_valuable();
        return;
      } else {
        shared_ptr<expression_state> m_state = m_stack.top();
        m->reduce();
        m_state->d[(m_state->opp)++].push_back(m);
        while(true) {
          if(m_state->ops == m_state->opp) { // ready for execute
            execute_operation(m_state->op->get_type(), m_state->d[0], m_state->d[1], m_state->d[2]);
            m_stack.pop();
            // recursive iterations
            if(m_stack.empty()) { // final
              assert(eqn.empty());    // eqn should be empty
              eqn.splice(eqn.end(), m_state->d[0]);
              valuable = eqn.size() == 1 && eqn.front()->is_valuable();
              return;
            } else {
              shared_ptr<expression_state> tmp = m_state;
              m_state = m_stack.top();
              m_state->d[m_state->opp].splice(m_state->d[m_state->opp].end(), tmp->d[0]);
              (m_state->opp)++;
            }
          } else break;       // do nothing, proceed to the next element
        }
      }
    } else {                  // an operator
      shared_ptr<expression_state> m_state(new expression_state);
      m_state->op = m;
      if(m->get_type() <= Operation::oUNxor) { // unary
          m_state->ops = 1;
      } else if(m->get_type() <= Operation::oLOr) { // two operands
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

void netlist::Expression::db_register(int iod) {
  list<shared_ptr<Operation> >::iterator it, end;
  for(it = eqn.begin(), end = eqn.end(); it != end; it++)
    (*it)->db_register(iod);
}

void netlist::Expression::db_expunge() {
  list<shared_ptr<Operation> >::iterator it, end;
  for(it = eqn.begin(), end = eqn.end(); it != end; it++)
    (*it)->db_expunge();
}

void netlist::Expression::append(Operation::operation_t otype) {
  assert(tExp == ctype);     // this object whould be valid
  assert(otype >= Operation::oUPos && otype < Operation::oPower);

  // preparing operands
  this->reduce();

  // connect the equation
  this->eqn.push_front(shared_ptr<Operation>(new Operation(otype)));
  this->valuable = false;

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
  this->eqn.push_front(shared_ptr<Operation>( new Operation(otype)));
  this->eqn.splice(this->eqn.end(), d1.eqn);
  this->valuable = false;
  
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

  this->eqn.push_front(shared_ptr<Operation>( new Operation(otype)));
  this->eqn.splice(this->eqn.end(), d1.eqn);
  this->eqn.splice(this->eqn.end(), d2.eqn);
  this->valuable = false;
  
  // try to reduce the final equation
  this->reduce();
}

void netlist::Expression::concatenate(const Expression& rhs) {
  assert(eqn.size() == 1 &&
	 eqn.front()->get_type() == Operation::oNum &&
         rhs.eqn.size() == 1 &&
	 rhs.eqn.front()->get_type() == Operation::oNum);

  eqn.front()->get_num().concatenate(rhs.eqn.front()->get_num());
}

Expression& netlist::operator+ (Expression& lhs, Expression& rhs) {
  lhs.append(Operation::oAdd, rhs);
  return lhs;
}

Expression& netlist::operator- (Expression& lhs, Expression& rhs) {
  lhs.append(Operation::oMinus, rhs);
  return lhs;  
}

bool netlist::operator== (const Expression& lhs, const Expression& rhs) {
  if(lhs.is_valuable() && rhs.is_valuable() && lhs.get_value() == rhs.get_value())
    return true;
  else
    return false;
}

namespace netlist{
  inline void 
  expression_pop_operators(
                           ostream& os, 
                           stack<pair<shared_ptr<Operation>,unsigned int> >& m_stack,
                           shared_ptr<Operation>& op,
                           unsigned int& op_cnt) {
    while(true) {
      if(!m_stack.empty()) { 
        op = m_stack.top().first; 
        op_cnt = m_stack.top().second + 1; 
        m_stack.pop();
      } else { 
        op.reset(); 
        op_cnt = 0;
        return;
      }
      
      if(op->get_type() <= Operation::oUNxor) { // unary
        continue;
      } else if (op->get_type() < Operation::oQuestion) { // two operands
        if(op_cnt == 2) {
          if(!m_stack.empty() && op->get_type() >= m_stack.top().first->get_type()+10)
            os << ")";
          continue;
        } else {                  // must be 1
          os << *op;
          return;
        }
      } else {                    // must be ?:
        if(op_cnt == 3) {
          if(!m_stack.empty() && op->get_type() >= m_stack.top().first->get_type()+10)
            os << ")";
          continue;
        } else if (op_cnt == 2) {
          os << " : ";
          return;
        } else {                  // must be 1
          os << " ? ";
          return;
        }
      }   
    }
  }
}

ostream& netlist::Expression::streamout(ostream& os, unsigned int indent) const {
  list<shared_ptr<Operation> >::const_iterator it, end;
  stack<pair<shared_ptr<Operation>,unsigned int> > m_stack;
  shared_ptr<Operation> c, op;
  unsigned int op_cnt = 0;

  os << string(indent, ' ');

  for(it=eqn.begin(), end=eqn.end(); it!=end; it++) {
    c = *it;
    if(c->get_type() <= Operation::oFun) { // data
      if(op.use_count() != 0) {
        if(op->get_type() <= Operation::oUNxor) { // unary operation always add parenthesis
          os << *c ;
          expression_pop_operators(os, m_stack, op, op_cnt);
        } else if(op->get_type() < Operation::oQuestion) { // two operands
          if(op_cnt == 0) { 	// first operand
            os << *c << *op;
            op_cnt = 1;
          } else {
            os << *c;

            if(!m_stack.empty() && op->get_type() >= m_stack.top().first->get_type()+10)
              os << ")";

            expression_pop_operators(os, m_stack, op, op_cnt);
          } 
        } else {		// ?
          if(op_cnt == 0) { 	// first operand
            os << *c << " ? ";
            op_cnt = 1;
          } else if(op_cnt == 1){
            os << *c << " : ";
            op_cnt = 2;
          } else {
            os << *c;

            if(!m_stack.empty() && op->get_type() >= m_stack.top().first->get_type()+10)
              os << ")";

            expression_pop_operators(os, m_stack, op, op_cnt);
          } 
        }
      } else {
        os << *c;
      }
    } else {			// must be an operator
      if(op.use_count() != 0) m_stack.push(pair<shared_ptr<Operation>,unsigned int>(op,op_cnt));
      op = c;
      op_cnt = 0;
      if(!m_stack.empty() && op->get_type() >= m_stack.top().first->get_type()+10)
        os << "(";
      if(op->get_type() <=  Operation::oUNxor)
        os << *op;
    }
  }
  
  return os;
}

void netlist::Expression::set_father(Block *pf) {
  if(father == pf) return;
  father = pf;
  list<shared_ptr<Operation> >::iterator it, end;
  for(it=eqn.begin(),end=eqn.end(); it!=end; it++)
    (*it)->set_father(pf);
}

bool netlist::Expression::check_inparse() {
  bool rv = true;
  list<shared_ptr<Operation> >::iterator it, end;
  for(it=eqn.begin(),end=eqn.end(); it!=end; it++)
    rv &= (*it)->check_inparse();
  return rv;
}

Expression* netlist::Expression::deep_copy() const {
  Expression* rv = new Expression();
  rv->loc = loc;
  rv->valuable = this->valuable;
  
  list<shared_ptr<Operation> >::const_iterator it, end;
  for(it=this->eqn.begin(), end=this->eqn.end(); it!=end; it++)
    rv->eqn.push_back(shared_ptr<Operation>((*it)->deep_copy()));

  rv->set_father(father);
  return rv;
}
