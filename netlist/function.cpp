/*
 * Copyright (c) 2011-2013 Wei Song <songw@cs.man.ac.uk> 
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
 * Combinational function
 * 22/02/2013   Wei Song
 *
 *
 */

#include "component.h"
#include "shell/env.h"
#include <algorithm>
#include <boost/foreach.hpp>
#include "sdfg/rtree.hpp"

using namespace netlist;
using std::ostream;
using std::endl;
using std::string;
using std::vector;
using boost::shared_ptr;
using std::list;
using std::pair;
using std::map;
using shell::location;

netlist::Function::Function()
  : Block(tFunction), automatic(false) {}

netlist::Function::Function(const shell::location& lloc)
  : Block(tFunction, lloc), automatic(false) {}

netlist::Function::Function(const shell::location& lloc, const FIdentifier& nm)
  : Block(tFunction, lloc), fname(nm), automatic(false) {}

void netlist::Function::set_return(boost::shared_ptr<Expression> lhs, boost::shared_ptr<Expression> rhs) {
  shared_ptr<Range> r(new Range(Range_Exp(lhs, rhs)));
  rtype.get_range().add_range(r);
}

void netlist::Function::set_inputs(const list<shared_ptr<Port> >& plist) {
  BOOST_FOREACH(shared_ptr<Port> m, plist) 
    add_input(m);
}

void netlist::Function::add_input(shared_ptr<Port>& p) {
  if(!db_port.find(p->name)) {
    db_port.insert(p->name, p);
  }
  shared_ptr<Port> pp = db_port.find(p->name);
  if(!db_var.find(pp->name)) {
    db_var.insert(pp->name, shared_ptr<Variable>(new Variable(pp->loc, pp->name, Variable::TReg)));
    if(pp->is_signed()) db_var.find(pp->name)->set_signed();
  }  
}

void netlist::Function::elab_inparse() {
  std::set<shared_ptr<NetComp> > to_del;

  Block::elab_inparse();

  BOOST_FOREACH(shared_ptr<NetComp> st, statements) {
    switch(st->get_type()) {
    case tPort: {
      SP_CAST(m, Port, st);
      add_input(m);
      to_del.insert(st);
      break;
    }
    default: ;
    }
  }
  
  
  BOOST_FOREACH(shared_ptr<NetComp> var, to_del) {
    statements.remove(var);
  }
  
}

void netlist::Function::set_father() {
  // macros defined in database.h
  DATABASE_SET_FATHER_FUN(db_port, VIdentifier, Port, this);
  Block::set_father();
}

void netlist::Function::set_father(Block *pf) {
  if(father == pf) return;
  father = pf;
}

ostream& netlist::Function::streamout(ostream& os, unsigned int indent) const {
  streamout(os, indent, false);
  return os;
}

ostream& netlist::Function::streamout(ostream& os, unsigned int indent, bool fl_prefix) const {
  
  if(!fl_prefix) os << string(indent, ' ');

  os << "function" ;
  
  if(!rtype.get_range().is_empty())
    os << " " << rtype.get_range() ;

  os << fname << endl;

  // ports and variables
  db_port.streamout(os, indent+2);
  db_var.streamout(os, indent+2);
  
  // statements
  if(statements.size() != 1)
    os << string(indent, ' ') << "begin" << endl;

  BOOST_FOREACH(const shared_ptr<NetComp>& it, statements)
    it->streamout(os, indent+2);
  
  if(statements.size() != 1)
    os << string(indent, ' ') << "end" << endl;

  os << string(indent, ' ') << "endfunction" << endl;

  return os;
}
 
Function* netlist::Function::deep_copy() const {
  Function* rv = new Function();
  
  rv->loc = loc;
  rv->name = name;
  rv->named = named;

  // data in Block
  BOOST_FOREACH(const shared_ptr<NetComp>& comp, statements)
    rv->statements.push_back(shared_ptr<NetComp>(comp->deep_copy()));
  DATABASE_DEEP_COPY_FUN(db_var,      VIdentifier, Variable,  rv->db_var       );
  rv->unnamed_block = unnamed_block;
  rv->unnamed_instance = unnamed_instance;
  rv->unnamed_var = unnamed_var;

  // data in Function
  rv->fname = fname;
  DATABASE_DEEP_COPY_FUN(db_port,      VIdentifier, Port,   rv->db_port        );
  rv->automatic = automatic;
  rv->rtype = rtype;

  rv->set_father();
  return rv;
}

void netlist::Function::db_register(int) {
  for_each(db_var.begin_order(), db_var.end_order(), [](pair<const VIdentifier, shared_ptr<Variable> >& m) {
      m.second->db_register(1);
    });
  BOOST_FOREACH(shared_ptr<NetComp>& m, statements) m->db_register(1);
}


void netlist::Function::db_expunge() {
  for_each(db_var.begin_order(), db_var.end_order(), [](pair<const VIdentifier, shared_ptr<Variable> >& m) {
      m.second->db_expunge();
    });
  BOOST_FOREACH(shared_ptr<NetComp>& m, statements) m->db_expunge();
}

bool netlist::Function::elaborate(std::set<shared_ptr<NetComp> >&,
                                  map<shared_ptr<NetComp>, list<shared_ptr<NetComp> > >&) {
  bool rv = true;
  std::set<shared_ptr<NetComp> > to_del;
  map<shared_ptr<NetComp>, list<shared_ptr<NetComp> > > to_add;
  
  // link all variables
  db_register(0);

  // check all variables
  for_each(db_port.begin_order(), db_port.end_order(), [&](pair<const VIdentifier, shared_ptr<Port> >& m) {
      rv &= m.second->elaborate(to_del, to_add);
    });
  //std::cout << "after port elaboration: " << std::endl << *this;
  if(!rv) return rv;

  // check variables
  for_each(db_var.begin_order(), db_var.end_order(), [&](pair<const VIdentifier, shared_ptr<Variable> >& m) {
      rv &= m.second->elaborate(to_del, to_add);
    });
  if(!rv) return rv;

  // elaborate the internals
  BOOST_FOREACH(shared_ptr<NetComp>& m, statements) 
    rv &= m->elaborate(to_del, to_add);
  if(!rv) return rv;
  
  // remove useless variables
  list<VIdentifier> var_to_be_removed;
  for_each(db_var.begin_order(), db_var.end_order(), [&](pair<const VIdentifier, shared_ptr<Variable> >& m) {
      if(m.second->is_useless()) var_to_be_removed.push_back(m.first);
    });
  BOOST_FOREACH(const VIdentifier& m, var_to_be_removed) 
    db_var.erase(m);

  return rv;
}
