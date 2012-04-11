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
 * Definition of netlist components.
 * 15/02/2012   Wei Song
 *
 *
 */

#include "component.h"

using namespace netlist;

ostream& netlist::Module::streamout(ostream& os, unsigned int indent) const {
  os << string(indent, ' ') << name;
  if(db_port.empty()) os << ";" << endl;
  else {
    os << "(";
    list<pair<PoIdentifier, shared_ptr<Port> > >::const_iterator it, end;
    it = db_port.begin_order();
    end = db_port.end_order();
    while(it != end){
      os << it->second->name.name;
      it++;
      if(it != end)
        os << ", ";
      else
        os << ");" << endl;
    }
  }
  
  { // parameters
    if(!db_param.empty()) os << endl;
    list<pair<VIdentifier, shared_ptr<Variable> > >::const_iterator it, end;
    for(it = db_param.begin_order(), end = db_param.end_order(); it != end; it++)
      os << string(indent+2, ' ') << "parameter " << *(it->second) << ";" << endl;
  }
  
  { // ports
    if(!db_port.empty()) os << endl;
    db_port.streamout(os, indent+2);
  }
  
  { // wires and regs
    map<VIdentifier, shared_ptr<Variable> >::const_iterator it, end;
    if(!db_wire.empty()) os << endl;
    for(it = db_wire.begin(), end = db_wire.end(); it != end; it++)
      os << string(indent+2, ' ') << "wire " << *(it->second) << ";" << endl;
    if(!db_reg.empty()) os << endl;
    for(it = db_reg.begin(), end = db_reg.end(); it != end; it++)
      os << string(indent+2, ' ') << "reg " << *(it->second) << ";" << endl;
  }

  { // gen vars
    list<pair<VIdentifier, shared_ptr<Variable> > >::const_iterator it, end;
    if(!db_genvar.empty()) os << endl;
    for(it = db_genvar.begin_order(), end = db_genvar.end_order(); it != end; it++)
      os << string(indent+2, ' ') << "genvar " << *(it->second) << ";" << endl;
  }

  { // continueous assignments
    if(!statements.empty()) os << endl;
    list<shared_ptr<NetComp> >::const_iterator it, end;
    for(it = statements.begin(), end = statements.end(); it != end; it++) {
      if((*it)->get_type() == NetComp::tAssign)
        os << string(indent+2, ' ') << "assign " << **it << ";" << endl;
      else {
        os << endl;
        (*it)->streamout(os, indent+2);
      }
    }
  }

  { // module instances
    if(!db_instance.empty()) os << endl;
    db_instance.streamout(os, indent+2);
  }

  os << endl << string(indent, ' ') << "endmodule" << endl << endl;
  return os;
}


void netlist::Module::clear() {
  Block::clear();
  db_port.clear();
  db_param.clear();
  db_genvar.clear();
}

VIdentifier& netlist::Module::new_VId() {
  while(db_wire.find(unnamed_var).use_count() +
        db_reg.find(unnamed_var).use_count() +
        db_param.find(unnamed_var).use_count() +
        db_genvar.find(unnamed_var).use_count() != 0)
    ++unnamed_var;
  return unnamed_var;
}

void netlist::Module::elab_inparse() {
  list<shared_ptr<NetComp> >::iterator it, end;
  for(it=statements.begin(), end=statements.end(); it!=end; it++) {
    switch((*it)->get_type()) {
    case tAssign: {
      SP_CAST(m, Assign, *it);
      m->set_name(new_BId());
      db_other.insert(m->name, m);
      break;
    }
    case tBlock: {
      SP_CAST(m, Assign, *it);
      if(!m->is_named()) m->name = new_BId();
      db_other.insert(m->name, m);
      break;
    }
    case tCase: {
      SP_CAST(m, CaseState, *it);
      m->set_name(new_BId());
      db_other.insert(m->name, m);
      break;
    }
    case tFor: {
      SP_CAST(m, ForState, *it);
      m->set_name(new_BId());
      db_other.insert(m->name, m);
      break;
    }
    case tIf: {
      SP_CAST(m, IfState, *it);
      m->set_name(new_BId());
      db_other.insert(m->name, m);
      break;
    }
    case tInstance: {
      SP_CAST(m, Instance, *it);
      if(!m->is_named()) m->set_name(new_IId());
      db_instance.insert(m->name, m);
      break;
    }
    case tWhile: {
      SP_CAST(m, WhileState, *it);
      m->set_name(new_BId());
      db_other.insert(m->name, m);
      break;
    }
    case tPort: {
      SP_CAST(m, Port, *it);
      db_port.insert(m->name, m);
      break;
    }
    case tVariable: {
      SP_CAST(m, Variable, *it);
      switch(m->get_vtype()) {
      case Variable::TWire: {
        db_wire.insert(m->name, m);
        break;
      }
      case Variable::TReg: {
        db_reg.insert(m->name, m);
        break;
      }
      case Variable::TParam: {
        db_param.insert(m->name, m);
        break;
      }
      case Variable::TGenvar: {
        db_genvar.insert(m->name, m);
        break;
      }
      default:
        assert(0 == "wrong type of variable in general block!");
      }
      break;
    }
    default:
      assert(0 == "wrong type os statement in general block!");
    }
  }

  // double check the size
  if(statements.size() > 1)
    blocked = true;             // indicating multiple variable defintions (may happen when it is module or genblock)
}

