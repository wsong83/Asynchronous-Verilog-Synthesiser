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

#include <algorithm>
#include <deque>
#include "component.h"

using namespace netlist;
using std::ostream;
using std::endl;
using std::pair;
using std::string;
using boost::shared_ptr;
using std::map;
using std::vector;
using std::deque;


netlist::Variable::Variable(const Port& p)
  : NetComp(tVariable, p.loc)
{
  vtype = TWire;
  name = VIdentifier(p.name.loc, p.name.name, p.name.get_range());
}

void netlist::Variable::set_value(const Number& num) {
  exp.reset(new Expression(num));
  update();
}

bool netlist::Variable::update() {
  assert(exp.use_count() != 0);
  exp->reduce();
  if(!exp->is_valuable()) return false;

  Number m = exp->get_value();
  map<unsigned int, VIdentifier *>::iterator it, end;
  for(it=fan[1].begin(), end=fan[1].end(); it != end; it++) {
    it->second->set_value(m);
  }
  return true;
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

bool netlist::Variable::check_inparse() {
  if(exp.use_count() != 0) return exp->check_inparse();
  else return true;
}

unsigned int netlist::Variable::get_id() {

  uid++;
  unsigned int rv = uid;
  
  if(rv - fan[0].size() - fan[1].size() > MAX_NUMBER_UNUSED_IN_MAP) { // clean the map
    deque<unsigned int> availID;
    map<unsigned int, VIdentifier *> newFan[2];
    unsigned int i;
    for(i=1; !fan[0].empty() | fan[1].empty(); i++) {
      if(!fan[0].count(i) && !fan[1].count(i)) { // id available
        availID.push_back(i);
      } else if(!availID.empty()) { // replace the fan with a new id
        unsigned int newid = availID.front();
        availID.pop_front();
        if(fan[0].count(i)) {
          newFan[0][newid] = fan[0][i];
          fan[0].erase(i);
          newFan[0][newid]->reset_uid(newid);
        }
        if(fan[1].count(i)) {
          newFan[1][newid] = fan[1][i];
          fan[1].erase(i);
          newFan[1][newid]->reset_uid(newid);
        }
      } else {                  // just move the fan to new fan
        if(fan[0].count(i)) {
          newFan[0][i] = fan[0][i];
          fan[0].erase(i);
        }
        if(fan[1].count(i)) {
          newFan[1][i] = fan[1][i];
          fan[1].erase(i);
        }
      }        
    }
    if(availID.empty()) uid = i;
    else uid = availID.front();
    rv = uid;
    fan[0] = newFan[0];
    fan[1] = newFan[1];
  }
  return rv;
}
    
Variable* netlist::Variable::deep_copy() const {
  Variable *rv = new Variable(loc, name, vtype);
  if(exp.use_count() != 0) rv->exp.reset(exp->deep_copy());

  // every time a variable is deep copied, all fan-in and -out connections are lost and need to regenerated
 
  rv->set_father(father);
  return rv;
}

void netlist::Variable::db_register(int iod) {
  if(exp.use_count() != 0) exp->db_register(1);
}

void netlist::Variable::db_expunge() {
  if(exp.use_count() != 0) exp->db_expunge();
}
