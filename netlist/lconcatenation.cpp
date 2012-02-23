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
 * left-side concatenation, only to be the target of an assignment
 * 23/02/2012   Wei Song
 *
 *
 */

#include "component.h"

using namespace netlist;

netlist::LConcatenation::LConcatenation(shared_ptr<Concatenation> con)
  : valid(false)
{
  list<ConElem>::const_iterator it, end;
  for( it = con->data.begin(), end = con->data.end(); it != end; it++) {
    if(0 == it->exp.use_count()) break; // the concatenation contain sub-contenations
    if(it->exp->eqn.size() != 1) break; // the expression ia still complex
    if(it->exp->eqn.front().get_type() != Operation::oVar) break; // wrong type
    if(it->exp->eqn.front().get_var()->get_type() != NetComp::tVarName) break; // wrong type
    data.push_back(static_pointer_cast<VIdentifier>(it->exp->eqn.front().get_var()));
  }
  if(it == end) valid = true;
}

netlist::LConcatenation::LConcatenation(shared_ptr<VIdentifier> id)
  : valid(true) { data.push_back(id); }


ostream& netlist::LConcatenation::streamout(ostream& os) const {
  assert(valid);

  if(1 == data.size()) os << *data.front();
  else {
    list<shared_ptr<VIdentifier> >::const_iterator it, end;
    it=data.begin();
    end=data.end();
    os << "{";
    while(true) {
      os << *it;
      it++;
      if(it != end)
	os << ",";
      else {
	os << "}";
	break;
      }
    }
  }
  return os;
}
