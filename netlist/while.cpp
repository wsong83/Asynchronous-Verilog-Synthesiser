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
 * while statements
 * 03/04/2012   Wei Song
 *
 *
 */

#include "component.h"

using namespace netlist;

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
  assert(exp.use_count() != 0);

  os << string(indent, ' ') << "while (" << *exp << ") ";
  body->streamout(os, indent, true);

  return os;
}
