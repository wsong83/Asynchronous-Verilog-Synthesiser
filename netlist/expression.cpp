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
  eqn.reset(new Operation(lloc, exp));
}

netlist::Expression::Expression(const VIdentifier& id) 
  : NetComp(tExp)
{
  eqn.reset(new Operation(id));
}

netlist::Expression::Expression(const location& lloc, const VIdentifier& id) 
  : NetComp(tExp, lloc)
{
  eqn.reset(new Operation(lloc, id));
}

netlist::Expression::Expression(const shared_ptr<Concatenation>& con) 
  : NetComp(tExp)
{
  eqn.reset(new Operation(con));
}

netlist::Expression::Expression(const location& lloc, const shared_ptr<Concatenation>& con) 
  : NetComp(tExp, lloc)
{
  eqn.reset(new Operation(lloc, con));
}

netlist::Expression::Expression(const shared_ptr<LConcatenation>& con)
  : NetComp(tExp)
{
  eqn.reset(new Operation(con));
}

netlist::Expression::Expression(const location& lloc, const shared_ptr<LConcatenation>& con)
  : NetComp(tExp, lloc)
{
  eqn.reset(new Operation(lloc, con));
}

netlist::Expression::~Expression() {}

bool netlist::Expression::is_valuable() const {
  assert(eqn.use_count() != 0);
  return eqn->is_valuable();
}

Number netlist::Expression::get_value() const {
  assert(eqn.use_count() != 0);
  assert(eqn->is_valuable());
  return eqn->get_num();
}

bool netlist::Expression::is_singular() const {
  assert(eqn.use_count() != 0);
  return ((eqn->get_type() > Operation::oNULL) && (eqn->get_type() <= Operation::oFun));
}

const Operation& netlist::Expression::get_op() const {
  assert(eqn.use_count() != 0);
  return *eqn;
}

Operation& netlist::Expression::get_op() {
  assert(eqn.use_count() != 0);
  return *eqn;
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

unsigned int netlist::Expression::get_width() {
  if(width) return width;
  assert(eqn.use_count() != 0);
  width = eqn->get_width();
  return width;
}

void netlist::Expression::set_width(const unsigned int& w) {
  if(width == w) return;
  assert(eqn.use_count() != 0);
  // there is no need to make sure w <= width as it may happen this way
  // such as assign a[3:0] = 1'b1;
  eqn->set_width(w);
  width = w;
  return;
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
  assert(eqn.use_count() != 0);
  eqn->set_father(pf);
}

bool netlist::Expression::check_inparse() {
  bool rv = true;
  assert(eqn.use_count() != 0);
  rv &= eqn->check_inparse();
  return rv;
}

Expression* netlist::Expression::deep_copy() const {
  Expression* rv = new Expression();
  rv->loc = loc;
  rv->width = width;
  assert(eqn.use_count() != 0);
  rv->eqn = shared_ptr<Operation>(eqn->deep_copy());
  return rv;
}

bool netlist::Expression::elaborate(elab_result_t &result, const ctype_t mctype, const vector<NetComp *>&) {
  bool rv = true;
  result = ELAB_Normal;
  
  // try to reduce the expression
  assert(eqn.use_count() != 0);
  eqn->reduce();

  eqn->elaborate(result);
  if(!rv) return false;

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
