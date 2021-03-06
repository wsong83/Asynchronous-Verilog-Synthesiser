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
 * while statements
 * 03/04/2012   Wei Song
 *
 *
 */

#include "component.h"

using namespace netlist;
using std::ostream;
using std::string;
using boost::shared_ptr;
using shell::location;

netlist::WhileState::WhileState(const shared_ptr<Expression>& exp, const shared_ptr<Block>& body)
  : NetComp(NetComp::tWhile), exp(exp), body(body), named(false)
{
  body->elab_inparse();
}

netlist::WhileState::WhileState(const location& lloc, const shared_ptr<Expression>& exp, const shared_ptr<Block>& body)
  : NetComp(NetComp::tWhile, lloc), exp(exp), body(body), named(false)
{
  body->elab_inparse();
}

void netlist::WhileState::set_father(Block *pf) {
  if(father == pf) return;
  father= pf;
  exp->set_father(pf);
  body->set_father(pf);
}

ostream& netlist::WhileState::streamout(ostream& os, unsigned int indent) const {
  assert(exp);

  os << string(indent, ' ') << "while (" << *exp << ") ";
  body->streamout(os, indent, true);

  return os;
}

WhileState* netlist::WhileState::deep_copy(NetComp* bp) const {
  WhileState *rv;
  if(!bp) rv = new WhileState(loc);
  else    rv = static_cast<WhileState *>(bp); // C++ does not support multiple dispatch
  NetComp::deep_copy(rv);

  rv->name = name;
  rv->named = named;
  
  if(exp) rv->exp.reset(exp->deep_copy(NULL));
  if(body) rv->body.reset(body->deep_copy(NULL));

  return rv;
}

void netlist::WhileState::db_register(int) {
  if(exp) exp->db_register(1);
  if(body) body->db_register(1);
}

void netlist::WhileState::db_expunge() {
  if(exp) exp->db_expunge();
  if(body) body->db_expunge();
}

void netlist::WhileState::replace_variable(const VIdentifier& var, const Number& num) {
  exp->replace_variable(var, num);
  body->replace_variable(var, num);
}

void netlist::WhileState::replace_variable(const VIdentifier& var, const VIdentifier& nvar) {
  exp->replace_variable(var, nvar);
  body->replace_variable(var, nvar);
}

shared_ptr<Block> netlist::WhileState::unfold() {
  std::cout << "unfold() for while is not supported yet" << std::endl;
  return shared_ptr<Block>();
}
