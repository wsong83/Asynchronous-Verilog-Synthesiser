/*
 * Copyright (c) 2012-2014 Wei Song <songw@cs.man.ac.uk> 
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
#include "sdfg/rtree.hpp"
#include <algorithm>
#include <stack>
#include <set>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

using namespace netlist;
using std::ostream;
using std::string;
using boost::shared_ptr;
using shell::location;
using std::pair;
using std::map;
using std::stack;
using std::list;
using std::vector;
using std::for_each;

netlist::Expression::Expression() : NetComp(tExp) {}

netlist::Expression::Expression(const shell::location& lloc) : NetComp(tExp, lloc) {}

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

netlist::Expression::Expression(const location& lloc, const shared_ptr<FuncCall>& fc)
  : NetComp(tExp, lloc)
{
  eqn.reset(new Operation(lloc, fc));
}

netlist::Expression::Expression(shared_ptr<Operation> e) 
  : NetComp(tExp, e->loc)
{
  eqn = e;
}

netlist::Expression::~Expression() {}

bool netlist::Expression::is_valuable() const {
  assert(eqn);
  return eqn->is_valuable();
}

Number netlist::Expression::get_value() const {
  assert(eqn);
  assert(eqn->is_valuable());
  return eqn->get_num();
}

bool netlist::Expression::is_variable() const {
  return eqn->get_type() == Operation::oVar;
}

VIdentifier netlist::Expression::get_variable() const {
  return eqn->get_var();
}

bool netlist::Expression::is_singular() const {
  assert(eqn);
  return ((eqn->get_type() > Operation::oNULL) && (eqn->get_type() <= Operation::oFun));
}

const Operation& netlist::Expression::get_op() const {
  assert(eqn);
  return *eqn;
}

Operation& netlist::Expression::get_op() {
  assert(eqn);
  return *eqn;
}

void netlist::Expression::reduce() {
  assert(eqn);
  eqn->reduce();
}

void netlist::Expression::db_register(int iod) {
  eqn->db_register(iod);
}

void netlist::Expression::db_expunge() {
  eqn->db_expunge();
}

Expression* netlist::Expression::append(Operation::operation_t otype) {
  assert(tExp == ctype);     // this object whould be valid
  assert(otype >= Operation::oUPos && otype < Operation::oPower);
  assert(eqn);

  eqn.reset(new Operation(otype, eqn));
  return this;
}

Expression* netlist::Expression::append(Operation::operation_t otype, Expression& d1) {
  assert(tExp == ctype);     // this object whould be valid
  assert(otype >= Operation::oPower && otype < Operation::oQuestion);
  assert(eqn);

  eqn.reset(new Operation(otype, eqn, d1.eqn));
  return this;
}

Expression* netlist::Expression::append(Operation::operation_t otype, Expression& d1, Expression& d2) {
  assert(tExp == ctype);     // this object whould be valid
  assert(otype >= Operation::oQuestion);
  assert(eqn);

  eqn.reset(new Operation(otype, eqn, d1.eqn, d2.eqn));
  return this;
}

Expression* netlist::Expression::concatenate(const Expression& rhs) {
  assert(eqn->is_valuable() && rhs.eqn->is_valuable());
  eqn->get_num().concatenate(rhs.eqn->get_num());
  return this;
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
  assert(eqn);
  eqn->set_father(pf);
}

Expression* netlist::Expression::deep_copy(NetComp* bp) const {
  Expression *rv;
  if(!bp) rv = new Expression();
  else    rv = static_cast<Expression *>(bp); // C++ does not support multiple dispatch
  NetComp::deep_copy(rv);
  assert(eqn);
  rv->eqn = shared_ptr<Operation>(eqn->deep_copy(NULL));
  return rv;
}

SDFG::RTree netlist::Expression::get_rtree() const {
  return eqn->get_rtree();
}

unsigned int netlist::Expression::get_width() const {
  return eqn->get_width();
}

void netlist::Expression::replace_variable(const VIdentifier& var, const Number& num) {
  eqn->replace_variable(var, num);
}

void netlist::Expression::replace_variable(const VIdentifier& var, const VIdentifier& nvar) {
  eqn->replace_variable(var, nvar);
}

void netlist::Expression::replace_variable(const VIdentifier& var, shared_ptr<Expression> rexp) {
  eqn->replace_variable(var, rexp);
}

//pair<bool, list<SSA_CONDITION_TYPE> >
void
netlist::Expression::extract_ssa_condition( const VIdentifier& sname) const {
  bool extractable = false;
  list<OpPair> conditions = eqn->breakToCases();

  // debug
  //std::cout << "SSA Conditions for \"" << sname << "\"" << std::endl;
  BOOST_FOREACH(OpPair opair, conditions) {
    if(opair.first)
      std::cout << "[" << *(opair.first) << "] " << *(opair.second) << std::endl;
    else
      std::cout << "[] " << *(opair.second) << std::endl;
  }
  std::cout << std::endl;
  // a lot to be implemented here
}
