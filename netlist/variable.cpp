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
 * Register
 * 27/02/2012   Wei Song
 *
 *
 */

#include "component.h"
#include "shell/env.h"
#include <algorithm>
#include <deque>
#include <boost/foreach.hpp>

using namespace netlist;
using std::ostream;
using std::endl;
using std::pair;
using std::list;
using std::string;
using boost::shared_ptr;
using std::map;
using std::vector;
using std::deque;


netlist::Variable::Variable() 
  : NetComp(tVariable), uid(0), signed_flag(false), annotated(false) {}

netlist::Variable::Variable(const shell::location& lloc) 
  : NetComp(tVariable, lloc), uid(0), signed_flag(false), annotated(false) {}

netlist::Variable::Variable(const VIdentifier& id, vtype_t mtype)
  : NetComp(tVariable), vtype(mtype), name(*(id.deep_copy(NULL))), uid(0), signed_flag(false), annotated(false) {}

netlist::Variable::Variable(const Port& p)
  : NetComp(tVariable, p.loc), vtype(TWire), uid(0), signed_flag(false) , annotated(false)
{
  VIdentifier *newName = p.name.deep_copy(NULL);
  name = *newName;
  delete newName;
}

netlist::Variable::Variable(const shell::location& lloc, const VIdentifier& id, vtype_t mtype)
  : NetComp(tVariable, lloc), vtype(mtype), name(*(id.deep_copy(NULL))), uid(0), signed_flag(false), annotated(false) {}

netlist::Variable::Variable(const VIdentifier& id, const shared_ptr<Expression>& expp, vtype_t mtype)
  : NetComp(tVariable), vtype(mtype), name(*(id.deep_copy(NULL))), exp(expp), uid(0), signed_flag(false), annotated(false) {}

netlist::Variable::Variable(const shell::location& lloc, const VIdentifier& id, 
                            const shared_ptr<Expression>& expp, vtype_t mtype)
  : NetComp(tVariable, lloc), vtype(mtype), name(*(id.deep_copy(NULL))), exp(expp), uid(0), signed_flag(false), annotated(false) {}

void netlist::Variable::set_value(const Number& num) {
  if(exp) exp->db_expunge();
  exp.reset(new Expression(num));
  update();
}

void netlist::Variable::set_value(const VIdentifier& var) {
  if(exp) exp->db_expunge();
  VIdentifier * varp = var.deep_copy(NULL);
  exp.reset(new Expression(*varp));
  exp->db_register(1);
  delete varp;
  update();
}

void netlist::Variable::set_value(const shared_ptr<Expression>& mexp) {
  if(exp) exp->db_expunge();
  exp.reset(mexp->deep_copy(NULL));
  exp->db_register(1);
  update();
}

bool netlist::Variable::update() {
  if(!exp) return false; // no need to update

  exp->reduce();
  if(!exp->is_valuable()) {
#ifndef NDEBUG
    std::cout << *exp << std::endl;
#endif
    return false;
  }

  Number m = exp->get_value();
  exp.reset(new Expression(m));
  typedef pair<const unsigned int, VIdentifier *> fan_type;
  BOOST_FOREACH(fan_type f, fan[1]) {
    f.second->set_value(m);
  }
  return true;
}

void netlist::Variable::set_father(Block *pf) {
  if(father == pf) return;
  father = pf;
  name.set_father(pf);
  if(exp) exp->set_father(pf);
}

ostream& netlist::Variable::streamout(ostream& os, unsigned int indent) const {

  os << string(indent, ' ');

  switch(vtype) {
  case TWire:   os << "wire ";      break;
  case TReg:    os << "reg ";       break;
  case TParam:  os << "parameter "; break;
  case TLParam: os << "localparam "; break;
  case TGenvar: os << "genvar ";    break;
  default: assert(0 == "uninitialized variable!");
  }
  if(signed_flag) os << "signed ";
  name.get_range().RangeArrayCommon::streamout(os, 0, "", true, false); // show range of declaration 
  name.get_range().RangeArrayCommon::streamout(os, 1, name.get_name(), true, true); // show dimension of declaration
  if(exp) { os << " = " << *exp; }
  os << ";" << endl;
  return os;
}

unsigned int netlist::Variable::get_id() {

  uid++;
  unsigned int rv = uid;
  
  if(rv - fan[0].size() - fan[1].size() > MAX_NUMBER_UNUSED_IN_MAP) { // clean the map
    deque<unsigned int> availID;
    map<unsigned int, VIdentifier *> newFan[2];
    unsigned int i;
    for(i=1; !fan[0].empty() | !fan[1].empty(); i++) {
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
    
Variable* netlist::Variable::deep_copy(Variable *rv) const {
  if(!rv) {
    VIdentifier * newid = name.deep_copy(NULL);
    rv = new Variable(loc, *newid, vtype);
    delete newid;
  }
  NetComp::deep_copy(rv);
  
  if(exp) rv->exp.reset(exp->deep_copy(NULL));
  rv->signed_flag = signed_flag;
  // every time a variable is deep copied, all fan-in and -out connections are lost and need to regenerated

  return rv;
}

void netlist::Variable::db_register(int) {
  name.db_register(1);
  if(exp) exp->db_register(1);
}

void netlist::Variable::db_expunge() {
  name.db_expunge();
  if(exp) exp->db_expunge();
}

bool netlist::Variable::elaborate(std::set<shared_ptr<NetComp> >&,
                                  map<shared_ptr<NetComp>, list<shared_ptr<NetComp> > >&) {
  name.get_range().reduce(true);

  if(!name.get_range().is_valuable()) {
    G_ENV->error(loc, "ELAB-RANGE-0", name.get_name());
    return false;
  }
  
  if(!name.get_range().is_declaration()) {
    G_ENV->error(loc, "ELAB-VAR-5", name.get_name());
    return false;  
  }

  if(vtype & (TParam | TLParam))
    update();

  return true;
}

// used in shell/cmd/elaborate.cpp
string netlist::Variable::get_short_string() const {
  string rv = name.get_name();
  rv += "=";
  rv += toString(*exp);
  return rv;
}

void netlist::Variable::replace_variable(const VIdentifier& var, const Number& num) {
  if(exp) exp->replace_variable(var, num);
}

void netlist::Variable::replace_variable(const VIdentifier& var, const VIdentifier& nvar) {
  if(exp) exp->replace_variable(var, nvar);
}

unsigned int netlist::Variable::get_width() const {
  if(name.get_range().is_empty())
    return 1;
  else
    return name.get_range().get_width(name.get_range());
}

void netlist::Variable::annotate(mpz_class toggle, mpz_class duration) {
  if(annotated) {
    if(toggle_min < toggle) toggle_min = toggle;
    toggle_max += toggle;
  } else {
    annotated = true;
    toggle_duration = duration;
    toggle_min = toggle;
    toggle_max = toggle;
  }
}
