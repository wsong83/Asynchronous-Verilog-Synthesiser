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

netlist::WhileState::WhileState(const shared_ptr<Expression>& exp, const shared_ptr<SeqBlock>& body)
  : exp(exp)
{
  if(body->is_named()) {
    statements.push_back(static_pointer_cast<NetComp>(body));
  } else {
    statements = body->statements;
  }
}

ostream& netlist::WhileState::streamout(ostream& os, unsigned int indent) const {
  assert(exp.use_count() != 0);

  os << string(indent, ' ') << "while (" << *exp << ") ";
  
  if(statements.size() == 1) {
    if(statements.front()->get_type() == NetComp::tSeqBlock)
      static_pointer_cast<SeqBlock>(statements.front())->streamout(os, indent, true);
    else {
      os << endl;
      statements.front()->streamout(os, indent+2);
      if(statements.front()->get_type() == NetComp::tAssign) os << ";" << endl;
    }
  } else {
    os << "begin" << endl;
    list<shared_ptr<NetComp> >::const_iterator it, end;
    for(it=statements.begin(), end=statements.end(); it!=end; it++) {
      (*it)->streamout(os, indent+2);
      if((*it)->get_type() == NetComp::tAssign) os << ";" << endl;
    }
    os << "end" << endl;
  }
  
  return os;
}
