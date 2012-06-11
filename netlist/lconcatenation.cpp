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

#include <algorithm>
#include "component.h"

using namespace netlist;
using std::ostream;
using std::string;
using std::vector;
using boost::shared_ptr;
using std::list;
using shell::location;
using std::for_each;

netlist::LConcatenation::LConcatenation(shared_ptr<Concatenation>& con)
  : NetComp(tLConcatenation, con->loc), valid(false)
{
  con->reduce();
  list<shared_ptr<ConElem> >::iterator it, end;
  for( it = con->data.begin(), end = con->data.end(); it != end; it++) {
    if(0 != (*it)->con.size()) break; // the concatenation contain sub-concatenations
    if((*it)->exp->size() != 1) break; // the expression ia still complex
    if((*it)->exp->front()->get_type() != Operation::oVar) break; // wrong type
    if((*it)->exp->front()->get_var().get_type() != tVarName) break; // wrong type
    data.push_back((*it)->exp->front()->get_var());
  }
  if(it == end) valid = true;
}

netlist::LConcatenation::LConcatenation(const location& lloc, shared_ptr<Concatenation>& con)
  : NetComp(tLConcatenation, lloc), valid(false)
{
  con->reduce();
  list<shared_ptr<ConElem> >::iterator it, end;
  for( it = con->data.begin(), end = con->data.end(); it != end; it++) {
    if(0 != (*it)->con.size()) break; // the concatenation contain sub-concatenations
    if((*it)->exp->size() != 1) break; // the expression ia still complex
    if((*it)->exp->front()->get_type() != Operation::oVar) break; // wrong type
    if((*it)->exp->front()->get_var().get_type() != tVarName) break; // wrong type
    data.push_back((*it)->exp->front()->get_var());
  }
  if(it == end) valid = true;
}

netlist::LConcatenation::LConcatenation(const VIdentifier& id)
  : NetComp(tLConcatenation, id.loc), valid(true) { data.push_back(id); }

netlist::LConcatenation::LConcatenation(const location& lloc, const VIdentifier& id)
  : NetComp(tLConcatenation, lloc), valid(true) { data.push_back(id); }

void netlist::LConcatenation::set_father(Block *pf) {
  if(father == pf) return;
  father = pf;
  list<VIdentifier>::iterator it, end;
  for(it=data.begin(), end=data.end(); it!=end; it++)
    it->set_father(pf);
}

bool netlist::LConcatenation::check_inparse() {
  bool rv = true;
  list<VIdentifier>::iterator it, end;
  for(it=data.begin(), end=data.end(); it!=end; it++)
    rv &= it->check_inparse();
  return rv;
}

ostream& netlist::LConcatenation::streamout(ostream& os, unsigned int indent) const {
  assert(valid);

  os << string(indent, ' ');

  if(1 == data.size()) os << data.front();
  else {
    list<VIdentifier>::const_iterator it, end;
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

LConcatenation* netlist::LConcatenation::deep_copy() const {
  LConcatenation* rv = new LConcatenation();
  rv->loc = loc;
  rv->valid = valid;
  for_each(data.begin(), data.end(), [&rv](const VIdentifier& m) {
      VIdentifier* mp = m.deep_copy();
      rv->data.push_back(*mp);
      delete mp;
    });
  
  return rv;
}

void netlist::LConcatenation::db_register(int iod) {
  for_each(data.begin(), data.end(), [](VIdentifier& m) {m.db_register(0);});
}

void netlist::LConcatenation::db_expunge() {
  for_each(data.begin(), data.end(), [](VIdentifier& m) {m.db_expunge();});
}

bool netlist::LConcatenation::elaborate(const ctype_t mctype, const vector<NetComp *>& fp) {
  bool rv = true;

  assert(valid && data.size() > 0);

  for_each(data.begin(), data.end(), [&rv, &fp](VIdentifier& m) {
      rv &= m.elaborate(tLConcatenation, fp);
    });

  return rv;
}

void netlist::LConcatenation::set_always_pointer(SeqBlock *p) {
  for_each(data.begin(), data.end(), [&p](VIdentifier& m) {m.set_always_pointer(p); });
}
