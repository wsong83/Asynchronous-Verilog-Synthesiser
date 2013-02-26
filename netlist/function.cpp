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
