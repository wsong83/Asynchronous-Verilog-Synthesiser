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
 * Register
 * 27/02/2012   Wei Song
 *
 *
 */

#include "component.h"

using namespace netlist;


netlist::Variable::Variable(const Port& p)
  : NetComp(tVariable, p.loc)
{
  if(p.is_in()) vtype = TWire;
  else          vtype = TReg;

  name = VIdentifier(p.name.loc, p.name.name, p.get_range());
}

void netlist::Variable::set_value(const Number& num) {
  exp.reset(new Expression(num));
  update();
}

void netlist::Variable::update() {
  assert(exp.use_count() != 0);
  exp->reduce();
  assert(exp->is_valuable());   // must be valuable right now

  Number m = exp->get_value();
  map<unsigned int, VIdentifier *>::iterator it, end;
  for(it=fan[1].begin(), end=fan[1].end(); it != end; it++) {
    it->second->set_value(m);
  }
}

void netlist::Variable::set_father(Block *pf) {
  if(father == pf) return;
  father = pf;
  if(exp.use_count() != 0) exp->set_father(pf);
}

ostream& netlist::Variable::streamout(ostream& os, unsigned int indent) const {

  os << string(indent, ' ');

  switch(vtype) {
  case TWire:   os << "wire ";      break;
  case TReg:    os << "reg ";       break;
  case TParam:  os << "parameter "; break;
  case TGenvar: os << "genvar ";    break;
  default: assert(0 == "uninitialized variable!");
  }

  vector<shared_ptr<Range> >::const_iterator it, end;

  vector<shared_ptr<Range> > rm = name.get_range();
  for(it=rm.begin(), end=rm.end(); it != end; it++) {
    if((*it)->is_dim()) continue;
    os << "[" << **it;
    if((*it)->is_single())
      os << ":" << **it;
    os << "] ";
  }
  os << name.name;
  rm = name.get_range();
  for(it=rm.begin(), end=rm.end(); it != end; it++) {
    if(!(*it)->is_dim()) break;
    os << "[" << **it;
    if((*it)->is_single())
      os << ":" << **it;
    os << "]";
  }

  if(exp.use_count() != 0) {
    os << " = " << *exp;
  }

  os << ";" << endl;

  return os;

}

unsigned int netlist::Variable::get_id(int iod) {
  assert(iod < 2 && iod >= 0);

  uid[iod]++;
  unsigned int rv = uid[iod];
  
  if(rv - fan[iod].size() > MAX_NUMBER_UNUSED_IN_MAP) { // clean the map
    unsigned int index = 1;
    map<unsigned int, VIdentifier *> new_map;
    map<unsigned int, VIdentifier *>::iterator it, end, cur;
    for(it = fan[iod].begin(), end = fan[iod].end(); it != end; index++) {
      it->second->reset_uid(index);
      cur = it; it++;
      VIdentifier * vp = cur->second;
      fan[iod].erase(cur);
      fan[iod].insert(it, pair<unsigned int, VIdentifier *>(index, vp));
    }
     
    rv = uid[iod] = index;
  }

  return rv;
}
    
