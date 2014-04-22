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
 * Port connections
 * 25/10/2012   Wei Song
 *
 *
 */

#include "component.h"

using namespace netlist;
using boost::shared_ptr;
using std::list;
using std::pair;
using std::map;


netlist::PortConn::PortConn(const shared_ptr<Expression>& exp, int dir_m)
  : NetComp(tPortConn), dir(dir_m), exp(exp), type(CEXP), named(false) 
{ 
  reduce(); 
}
    
netlist::PortConn::PortConn(const shell::location& lloc, 
                            const shared_ptr<Expression>& exp, 
                            int dir_m)
  : NetComp(tPortConn, lloc), dir(dir_m), exp(exp), type(CEXP), named(false) 
{ 
  reduce(); 
}
    
netlist::PortConn::PortConn(const shared_ptr<LConcatenation>& lval, int dir_m)
  : NetComp(tPortConn), dir(dir_m), exp(new Expression(lval)), type(CEXP), named(false) 
{ 
  reduce(); 
}
    
netlist::PortConn::PortConn(const shell::location& lloc, 
                            const boost::shared_ptr<LConcatenation>& lval, 
                            int dir_m)
  : NetComp(tPortConn, lloc), dir(dir_m), exp(new Expression(lval)), type(CEXP), named(false) 
{ 
  reduce(); 
}
    
netlist::PortConn::PortConn()
  : NetComp(tPortConn), dir(1), type(COPEN) {}
    
netlist::PortConn::PortConn(const VIdentifier& pn, const shared_ptr<Expression>& exp, int dir_m)
  : NetComp(tPortConn), pname(pn), dir(dir_m), exp(exp), type(CEXP), named(true) 
{ 
  reduce(); 
}

netlist::PortConn::PortConn(const shell::location& lloc, 
                            const VIdentifier& pn, 
                            const boost::shared_ptr<Expression>& exp,
                            int dir_m)
  : NetComp(tPortConn, lloc), pname(pn), dir(dir_m), exp(exp), type(CEXP), named(true) 
{ 
  reduce(); 
}

netlist::PortConn::PortConn(const VIdentifier& pn)
  : NetComp(tPortConn), pname(pn), dir(1), type(COPEN), named(true) {}

netlist::PortConn::PortConn(const shell::location& lloc, const VIdentifier& pn)
  : NetComp(tPortConn, lloc), pname(pn), dir(1), type(COPEN), named(true) {}

void netlist::PortConn::set_father(Block* pf) { 
  if(father == pf) return;
  father = pf;
  pname.set_father(pf);
  if(exp.use_count() != 0) exp->set_father(pf);
  var.set_father(pf);
}

PortConn* netlist::PortConn::deep_copy(PortConn *rv) const {
  if(!rv) rv = new PortConn();
  NetComp::deep_copy(rv);
  rv->pname = pname;
  rv->named = named;
  rv->dir = dir;
  if(exp.use_count() != 0) rv->exp.reset(exp->deep_copy(NULL));
  VIdentifier *pm = var.deep_copy(NULL);
  rv->var = *pm;
  delete pm;
  rv->num = num;
  rv->type = type;
  return rv;
}

void netlist::PortConn::replace_variable(const VIdentifier& v, const Number& n) {
  switch(type) {
  case CEXP:
    exp->replace_variable(v, n); break;
  case CVAR:
    if(var == v) {
      type = CNUM;
      num = n;
    } else {
      var.replace_variable(v, n);
    }
    break;
  default: ;
  }
}

void netlist::PortConn::replace_variable(const VIdentifier& v, const VIdentifier& nvar) {
  switch(type) {
  case CEXP:
    exp->replace_variable(v, nvar); break;
  case CVAR:
    if(var == v) {
      var.set_name(nvar.get_name());
    } else {
      var.replace_variable(v, nvar);
    }
    break;
  default: ;
  }
}

void netlist::PortConn::db_register(int) {
  switch(type) {
  case CEXP: 
    if(dir >= 0)  {
      assert(0 == "expression in a port connection when it is an output port, not support yet!");
      exp->db_register(0);
    }
    if(dir <= 0) exp->db_register(1);
    break;
  case CVAR: 
    if(dir >= 0) var.db_register(0);
    if(dir <= 0) var.db_register(1);
    break;
  default: ;
  }
}
      
void netlist::PortConn::db_expunge() {
  switch(type) {
  case CEXP: exp->db_expunge(); break;
  case CVAR: var.db_expunge(); break;
  default: ;
  }
}

bool netlist::PortConn::elaborate(std::set<shared_ptr<NetComp> >&,
                                  map<shared_ptr<NetComp>, list<shared_ptr<NetComp> > >&) {
  switch(type) {
  case CEXP: {
    exp->reduce();
    if(exp->is_valuable()) {
      type = CNUM;
      num = exp->get_value();
      exp.reset();
    } else if(exp->is_variable()) {
      type = CVAR;
      var = exp->get_variable();
      exp.reset();
    }
    break;
  }
  case CVAR: {
    var.reduce();
    if(var.is_valuable()) {
      type = CNUM;
      num = var.get_value();
      var.db_expunge();
    }
    break;
  }
  default: ;
  }
  return true;
}

std::ostream& netlist::PortConn::streamout (std::ostream& os, unsigned int) const {
  if(named) os << "." << pname.get_name() << "(";
  switch(type) {
  case CEXP: os << *exp; break;
  case CVAR: os << var; break;
  case CNUM: os << num; break;
  case COPEN: os << " "; break;
  default:
    assert(0 == "wrong connection type!");
  }
  if(named) os << ")";
  return os;
}

netlist::ParaConn::ParaConn()
  : NetComp(tParaConn), type(COPEN), named(false) { }

netlist::ParaConn::ParaConn(const shared_ptr<Expression>& exp)
  : NetComp(tParaConn), exp(exp), type(CEXP), named(false) 
{ 
  reduce(); 
}

netlist::ParaConn::ParaConn(const shell::location& lloc, const shared_ptr<Expression>& exp)
  : NetComp(tParaConn, lloc), exp(exp), type(CEXP), named(false) 
{ 
  reduce(); 
}

netlist::ParaConn::ParaConn(const VIdentifier& pn, const shared_ptr<Expression>& exp)
  : NetComp(tParaConn), pname(pn), exp(exp), type(CEXP), named(true) 
{ 
  reduce(); 
}

netlist::ParaConn::ParaConn(const shell::location& lloc, const VIdentifier& pn, const shared_ptr<Expression>& exp)
  : NetComp(tParaConn, lloc), pname(pn), exp(exp), type(CEXP), named(true) 
{ 
  reduce(); 
}

netlist::ParaConn::ParaConn(const VIdentifier& pn)
  : NetComp(tParaConn), pname(pn), type(COPEN), named(true) { }

netlist::ParaConn::ParaConn(const shell::location& lloc, const VIdentifier& pn)
  : NetComp(tParaConn, lloc), pname(pn), type(COPEN), named(true) { }

void netlist::ParaConn::set_father(Block* pf) { 
  if(father == pf) return;
  father = pf;
  pname.set_father(pf);
  if(exp.use_count() != 0) exp->set_father(pf);
  var.set_father(pf);
}

ParaConn* netlist::ParaConn::deep_copy(ParaConn *rv) const {
  if(!rv) rv = new ParaConn();
  NetComp::deep_copy(rv);
  rv->pname = pname;
  rv->named = named;
  if(exp.use_count() != 0) rv->exp.reset(exp->deep_copy(NULL));
  VIdentifier *pm = var.deep_copy(NULL);
  rv->var = *pm;
  delete pm;
  rv->num = num;
  rv->type = type;
  return rv;
} 

void netlist::ParaConn::db_register(int) {
  switch(type) {
  case CEXP: exp->db_register(1); break;
  case CVAR: var.db_register(1); break;
  default: ;
  }
}
      
void netlist::ParaConn::db_expunge() {
  switch(type) {
  case CEXP: exp->db_expunge(); break;
  case CVAR: var.db_expunge(); break;
  default: ;
  }
}

bool netlist::ParaConn::elaborate(std::set<shared_ptr<NetComp> >&,
                                  map<shared_ptr<NetComp>, list<shared_ptr<NetComp> > >&) {
  switch(type) {
  case CEXP: {
    exp->reduce();
    if(exp->is_valuable()) {
      type = CNUM;
      num = exp->get_value();
      exp.reset();
    } else if(exp->is_variable()) {
      type = CVAR;
      var = exp->get_variable();
      exp.reset();
    }
    break;
  }
  case CVAR: {
    var.reduce();
    if(var.is_valuable()) {
      type = CNUM;
      num = var.get_value();
      var.db_expunge();
    }
    break;
  }
  default: ;
  }
  return true;
}

void netlist::ParaConn::replace_variable(const VIdentifier& v, const Number& n) {
  switch(type) {
  case CEXP:
    exp->replace_variable(v, n); break;
  case CVAR:
    if(var == v) {
      type = CNUM;
      num = n;
    } else {
      var.replace_variable(v, n);
    }
    break;
  default: ;
  }
}

void netlist::ParaConn::replace_variable(const VIdentifier& v, const VIdentifier& nvar) {
  switch(type) {
  case CEXP:
    exp->replace_variable(v, nvar); break;
  case CVAR:
    if(var == v) {
      var.set_name(nvar.get_name());
    } else {
      var.replace_variable(v, nvar);
    }
    break;
  default: ;
  }
}

std::ostream& netlist::ParaConn::streamout (std::ostream& os, unsigned int) const {
  if(named) os << "." << pname.get_name() << "(";
  switch(type) {
  case CEXP: os << *exp; break;
  case CVAR: os << var; break;
  case CNUM: os << num; break;
  case COPEN: os << " "; break;
  default:
    assert(0 == "wrong connection type!");
  }
  if(named) os << ")";
  return os;
}
