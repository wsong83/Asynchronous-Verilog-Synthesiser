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
#include "shell/env.h"
#include <algorithm>
#include <stack>
#include <boost/foreach.hpp>

using namespace netlist;
using std::ostream;
using std::string;
using boost::shared_ptr;
using shell::location;
using std::pair;
using std::stack;
using std::list;
using std::vector;
using std::for_each;

netlist::Expression::Expression(const Number& exp) 
  : NetComp(tExp)
{
  eqn.reset(new Operation(exp));
}

netlist::Expression::Expression(const location& lloc, const Number& exp) 
  : NetComp(tExp, lloc)
{
  eqn.reset(new Operation(exp));
}

netlist::Expression::Expression(const VIdentifier& id) 
  : NetComp(tExp)
{
  eqn.reset(new Operation(id));
}

netlist::Expression::Expression(const location& lloc, const VIdentifier& id) 
  : NetComp(tExp, lloc)
{
  eqn.reset(new Operation(id));
}

netlist::Expression::Expression(const shared_ptr<Concatenation>& con) 
  : NetComp(tExp)
{
  eqn.reset(new Operation(con));
}

netlist::Expression::Expression(const location& lloc, const shared_ptr<Concatenation>& con) 
  : NetComp(tExp, lloc)
{
  eqn.reset(new Operation(con));
}

netlist::Expression::Expression(const shared_ptr<LConcatenation>& con)
  : NetComp(tExp)
{
  eqn.reset(new Operation(con));
}

netlist::Expression::Expression(const location& lloc, const shared_ptr<LConcatenation>& con)
  : NetComp(tExp, lloc)
{
  eqn.reset(new Operation(con));
}

netlist::Expression::~Expression() {}

bool netlist::Expression::is_valuable() const {
  return eqn->is_valuable();
}

Number netlist::Expression::get_value() const { 
  assert(eqn->is_valuable());
  return eqn->get_num();
}

bool netlist::Expression::is_valuable() const {
  if(child.use_count() != 0) return child->is_valuable();
  else return false;
}

Number netlist::Expression::get_value() const {
  if(child.use_count() != 0) {
    assert(child->is_valuable());
    return child->get_valuable();
  } else return Number();
}  

void netlist::Expression::reduce() {
  assert(eqn.use_count() != 0);
  eqn->reduce();
}

void netlist::Expression::db_register(int iod) {
  eqn->db_register(iod);
}

void netlist::Expression::db_expunge() {
  eqn->db_expunge();
}

void netlist::Expression::append(Operation::operation_t otype) {
  assert(tExp == ctype);     // this object whould be valid
  assert(otype >= Operation::oUPos && otype < Operation::oPower);
  assert(eqn.use_count() != 0);

  eqn.reset(new Operation(otype, eqn));
}

void netlist::Expression::append(Operation::operation_t otype, Expression& d1) {
  assert(tExp == ctype);     // this object whould be valid
  assert(otype >= Operation::oPower && otype < Operation::oQuestion);
  assert(eqn.use_count() != 0);

  eqn.reset(new Operation(otype, eqn, d1.eqn));
}

void netlist::Expression::append(Operation::operation_t otype, Expression& d1, Expression& d2) {
  assert(tExp == ctype);     // this object whould be valid
  assert(otype >= Operation::oQuestion);
  assert(eqn.use_count() != 0);

  eqn.reset(new Operation(otype, eqn, d1.eqn, d2.eqn));
}

void netlist::Expression::concatenate(const Expression& rhs) {
  assert(eqn->is_valuable() && rhs.eqn->is_valuable());
  eqn->get_num().concatenate(rhs.eqn->get_num());
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

ostream& netlist::Expression::streamout(ostream& os, unsigned int indent) const {
  os << string(indent, ' ') << *eqn;
  return os;
}

void netlist::Expression::set_father(Block *pf) {
  if(father == pf) return;
  father = pf;
  BOOST_FOREACH(shared_ptr<Operation>& it, eqn)
    it->set_father(pf);
}

bool netlist::Expression::check_inparse() {
  bool rv = true;
  BOOST_FOREACH(shared_ptr<Operation>& it, eqn)
    rv &= it->check_inparse();
  return rv;
}

Expression* netlist::Expression::deep_copy() const {
  Expression* rv = new Expression();
  rv->loc = loc;
  rv->valuable = this->valuable;
  BOOST_FOREACH(const shared_ptr<Operation>& it, eqn)
    rv->eqn.push_back(shared_ptr<Operation>(it->deep_copy()));
  return rv;
}

bool netlist::Expression::elaborate(elab_result_t &result, const ctype_t mctype, const vector<NetComp *>& fp) {
  bool rv = true;
  result = ELAB_Normal;
  
  // resolve all operation if possible
  assert(eqn.use_count() != 0);
  eqn->elaborate(result);
  if(!rv) return false;

  // try to reduce the expression
  eqn->reduce();

  // type specific check
  switch(mctype) {
  case tCaseItem: {
    // for a case item, it must be const
    if(!eqn->is_valuable()) {
      G_ENV->error(loc, "ELAB-CASE-3", toString(*this));
      rv = false;
    }
    break;
  }
  default:
    ;
  }

  return rv;
}
