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
 * If statements
 * 02/04/2012   Wei Song
 *
 *
 */

#include "component.h"

using namespace netlist;
using std::ostream;
using std::string;
using boost::shared_ptr;
using boost::static_pointer_cast;
using shell::location;

netlist::IfState::IfState(
                          const shared_ptr<Expression>& exp, 
                          const shared_ptr<Block>& m_ifcase, 
                          const shared_ptr<Block>& m_elsecase
                          )
  : NetComp(NetComp::tIf), exp(exp), ifcase(m_ifcase), elsecase(m_elsecase), named(false)
{
  ifcase->elab_inparse();
  elsecase->elab_inparse();
}

netlist::IfState::IfState(
                          const location& lloc,
                          const shared_ptr<Expression>& exp, 
                          const shared_ptr<Block>& m_ifcase, 
                          const shared_ptr<Block>& m_elsecase
                          )
  : NetComp(NetComp::tIf, lloc), exp(exp), ifcase(m_ifcase), elsecase(m_elsecase), named(false)
{
  ifcase->elab_inparse();
  elsecase->elab_inparse();
}

netlist::IfState::IfState(
                          const shared_ptr<Expression>& exp, 
                          const shared_ptr<Block>& m_ifcase
                          )
  : NetComp(NetComp::tIf), exp(exp),  ifcase(m_ifcase), named(false)
{
  ifcase->elab_inparse();
}

netlist::IfState::IfState(
                          const location& lloc,
                          const shared_ptr<Expression>& exp, 
                          const shared_ptr<Block>& m_ifcase
                          )
  : NetComp(NetComp::tIf, lloc), exp(exp),  ifcase(m_ifcase), named(false)
{
  ifcase->elab_inparse();
}

ostream& netlist::IfState::streamout(ostream& os, unsigned int indent) const {
  return streamout(os, indent, false);
}

void netlist::IfState::set_father(Block *pf) {
  if(father == pf) return;
  father = pf;
  exp->set_father(pf);
  ifcase->set_father(pf);
  if(elsecase.use_count() != 0) elsecase->set_father(pf);
}

bool netlist::IfState::check_inparse() {
  bool rv = true;
  rv &= exp->check_inparse();
  rv &= ifcase->check_inparse();
  if(elsecase.use_count() != 0) rv &= elsecase->check_inparse();
  return rv;
}

ostream& netlist::IfState::streamout(ostream& os, unsigned int indent, bool fl_prefix) const {
  assert(exp.use_count() != 0);

  if(!fl_prefix) os << string(indent, ' ');
  
  os << "if (" << *exp << ") ";
  ifcase->streamout(os, indent, true);

  if(elsecase.use_count() == 0) return os;

  os << string(indent, ' ') << "else ";
  if(!elsecase->is_blocked() && (elsecase->front()->get_type() == NetComp::tIf)) {
    static_pointer_cast<IfState>(elsecase->front())->streamout(os, indent, true);
  } else
    elsecase->streamout(os, indent, true);
  
  return os;

}

IfState* netlist::IfState::deep_copy() const {
  IfState* rv = new IfState(loc);
  rv->name = name;
  rv->named = named;
  
  if(exp.use_count() != 0) rv->exp.reset(exp->deep_copy());
  if(ifcase.use_count() != 0) rv->ifcase.reset(ifcase->deep_copy());
  if(elsecase.use_count() != 0) rv->elsecase.reset(elsecase->deep_copy());

  rv->set_father(father);
  return rv;
}

void netlist::IfState::db_register(int iod) {
  if(exp.use_count() != 0) exp->db_register(1);
  if(ifcase.use_count() != 0) ifcase->db_register(1);
  if(elsecase.use_count() != 0) elsecase->db_register(1);
}

void netlist::IfState::db_expunge() {
  if(exp.use_count() != 0) exp->db_expunge();
  if(ifcase.use_count() != 0) ifcase->db_expunge();
  if(elsecase.use_count() != 0) elsecase->db_expunge();
}
