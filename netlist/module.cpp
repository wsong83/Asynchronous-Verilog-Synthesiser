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
  if(list_port.empty()) os << ";" << endl;
  else {
    os << "(";
    map<PoIdentifer, shared_ptr<Port> >::const_iterator it, end;
    it = db_port.begin();
    end = db_port.end();
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
    map<VIdentifier, shared_ptr<Variable> >::const_iterator it, end;
    for(it = db_param.begin(), end = db_param.end(); it != end; it++)
      os << string(indent+2, ' ') << "parameter " << *(it->second) << ";" << endl;
  }
  
  { // ports
    if(!list_port.empty()) os << endl;
    list<shared_ptr<Port> >::const_iterator it, end;
    for(it=list_port.begin(), end=list_port.end(); it!=end; it++)
      os << string(indent+2, ' ') << *(*it);
  }
  
  { // wires and regs
    map<VIdentifier, shared_ptr<Variable> >::const_iterator it, end;
    if(!db_wire.empty()) os << endl;
    for(it = db_wire.begin(), end = db_wire.end(); it != end; it++)
      os << string(indent+2, ' ') << "wire " << *(it->second) << ";" << endl;
    if(!db_reg.empty()) os << endl;
    for(it = db_reg.begin(), end = db_reg.end(); it != end; it++)
      os << string(indent+2, ' ') << "reg " << *(it->second) << ";" << endl;
    if(!db_genvar.empty()) os << endl;
    for(it = db_genvar.begin(), end = db_genvar.end(); it != end; it++)
      os << string(indent+2, ' ') << "genvar " << *(it->second) << ";" << endl;
  }

  { // continueous assignments
    if(!db_assign.empty()) os << endl;
    map<string, shared_ptr<Assign> >::const_iterator it, end;
    for(it = db_assign.begin(), end = db_assign.end(); it != end; it++)
      os << string(indent+2, ' ') << "assign " << *(it->second) << ";" << endl;
  }

  { // module instances
    if(!db_instance.empty()) os << endl;
    db_instance.streamout(os, indent+2);
  }

  { // sequential blocks
    if(!db_block.empty()) os << endl;
    map<BIdentifier, shared_ptr<SeqBlock> >::const_iterator it, end;
    for(it = db_assign.begin(), end = db_assign.end(); it != end; it++) {
      os << string(indent+2, ' ') << "always ";
      it->second->streamout(os, indent+2, true);
    }
  }

  os << endl << string(indent, ' ') << "endmodule" << endl << endl;
  return os;
}

BIdentifier& netlist::Module::new_BId() {
  while(db_block.find(unnamed_block).use_count() != 0)
    ++unnamed_block;
  return unnamed_block;
}

IIdentifier& netlist::Module::new_IId() {
  while(db_instance.find(unnamed_instance).use_count() != 0)
    ++unnamed_instance;
  return unnamed_instance;
}

VIdentifier& netlist::Module::new_VId() {
  while(db_wire.find(unnamed_var).use_count() +
        db_reg.find(unnamed_var).use_count() +
        db_param.find(unnamed_var).use_count() +
        db_genvar.find(unnamed_var).use_count() != 0)
    ++unnamed_var;
  return unnamed_var;
}
