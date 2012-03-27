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
 * Case statements
 * 26/03/2012   Wei Song
 *
 *
 */

#include "component.h"

using namespace netlist;

ostream& netlist::CaseItem::streamout (ostream& os, unsigned int indent) const {
  // show the expressions
  if(is_default()) {
    os << string(indent, ' ');    // show the indent for each line
    os << "default: ";
  } else if(exps.size() == 1) {
    os << string(indent, ' ');    // show the indent for each line
    os << exps.front() << ": ";
  } else {
    unsigned int ncase = exps.size(); // number of cases in the item
    list<Expression>::const_iterator it = exps.begin();
    for(unsigned int i=0; i< ncase-1; i++) {
      os << string(indent, ' ');    // show the indent for each line
      os << *it << "," << endl;
      it++;
    }
    os << *it << ": ";
  }
  
  // the body part
  if(statements.size() == 0) {
  } else if(statements.size() == 1) {
    os << statements.front();   // this may have problem for named block in the future!!! Now just ignore it
  } else {
    os << "begin" << endl;
    list<NetComp>::const_iterator it, end;
    for(it=statements.begin(), end=statements.end(); it!=end; it++) {
      it->streamout(os, indent+2);
    }
    os << string(indent, ' ') << "end";
  }
  return os;
}

void netlist::CaseItem::add_statements(SeqBlock& body) {
  if(body.is_named()) {
    statements.push_back(body);
  } else {
    statements.splice(statements.end(), body.statements);
  }
}

ostream& netlist::CaseState::streamout (ostream& os, unsigned int indent) const {
  os << string(indent, ' ') << "case(" << exp << ")" << endl;
  list<CaseItem>::const_iterator it, end;
  for(it=cases.begin(), end=cases.end(); it!=end; it++) {
    it->streamout(os, indent+2);
    os << endl;
  }
  os << string(indent, ' ') << "endcase" << endl;

  return os;
}