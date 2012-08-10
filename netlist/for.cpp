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
 * for statements
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

netlist::ForState::ForState(
                            const shared_ptr<Assign>& init, 
                            const shared_ptr<Expression>& cond, 
                            const shared_ptr<Assign>& incr, 
                            const shared_ptr<Block>& body
                            )
  : NetComp(NetComp::tFor), init(init), cond(cond), incr(incr), body(body), named(false)
{
  body->elab_inparse();
}

netlist::ForState::ForState(
                            const location& lloc,
                            const shared_ptr<Assign>& init, 
                            const shared_ptr<Expression>& cond, 
                            const shared_ptr<Assign>& incr, 
                            const shared_ptr<Block>& body
                            )
  : NetComp(NetComp::tFor, lloc), init(init), cond(cond), incr(incr), body(body), named(false)
{
  body->elab_inparse();
}

void netlist::ForState::set_father(Block *pf) {
  if(father == pf) return;
  father = pf;
  name.set_father(pf);
  init->set_father(pf);
  cond->set_father(pf);
  incr->set_father(pf);
  body->set_father(pf);
}

bool netlist::ForState::check_inparse() {
  bool rv = true;
  rv &= init->check_inparse();
  rv &= cond->check_inparse();
  rv &= incr->check_inparse();
  rv &= body->check_inparse();
  return rv;
}

ostream& netlist::ForState::streamout(ostream& os, unsigned int indent) const {
  assert(init.use_count() != 0);

  os << string(indent, ' ') << "for (" << *init << "; " << *cond << "; " << *incr << ") ";
  body->streamout(os, indent, true);

  return os;
}

ForState* netlist::ForState::deep_copy() const {
  ForState* rv = new ForState(loc);
  rv->name = name;
  rv->named = named;
  
  if(init.use_count() != 0) rv->init.reset(init->deep_copy());
  if(cond.use_count() != 0) rv->cond.reset(cond->deep_copy());
  if(incr.use_count() != 0) rv->incr.reset(incr->deep_copy());
  if(body.use_count() != 0) rv->body.reset(body->deep_copy());

  return rv;
}

void netlist::ForState::db_register(int) {
  if(init.use_count() != 0) init->db_register(1);
  if(cond.use_count() != 0) cond->db_register(1);
  if(incr.use_count() != 0) incr->db_register(1);
  if(body.use_count() != 0) body->db_register(1);
}

void netlist::ForState::db_expunge() {
  if(init.use_count() != 0) init->db_expunge();
  if(cond.use_count() != 0) cond->db_expunge();
  if(incr.use_count() != 0) incr->db_expunge();
  if(body.use_count() != 0) body->db_expunge();
}

void netlist::ForState::set_always_pointer(SeqBlock *p) {
  if(body.use_count() != 0) body->set_always_pointer(p);
}
