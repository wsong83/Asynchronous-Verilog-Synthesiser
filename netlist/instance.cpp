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
 * A module instance
 * 19/03/2012   Wei Song
 *
 *
 */

#include "component.h"

using namespace netlist;

netlist::Instance::Instance(const IIdentifier& nm, const list<PortConn>& polist, type_t itype)
  : NetComp(tInstance), name(nm), port_list(polist), type(itype) {
  switch(itype) {
  case prim_in_inst: {
    list<PortConn>::iterator it, end;
    it = port_list.begin();
    it->set_out();
    it++;
    for(end=port_list.end(); it!=end; it++) {
      it->set_in();
    }
    break;
  }
  case prim_out_inst: {
    list<PortConn>::iterator it, end;
    it = port_list.begin();
    for(end=port_list.end(); it!=end; it++) {
      it->set_out();
    }
    it--;
    it->set_in();
    break;
  }
  default: ;
  }
}


ostream& netlist::Instance::streamout(ostream& os, unsigned int indent) const {
  // the module name
  os << string(indent, ' ') << mname.name << " ";

  // parameter list
  if(!para_list.empty()) {
    list<ParaConn>::const_iterator it, end;
    os << "#(";
    it=para_list.begin();
    end=para_list.end(); 
    while(it!= end) {
      os << *it;
      it++;
      if(it != end)
        os << ", ";
      else
        break;
    }
    os << ") ";
  }

  // instance name
  os << name << " (";

  // port connections
  {
    list<PortConn>::const_iterator it, end;
    it=port_list.begin();
    end=port_list.end(); 
    while(it!= end) {
      os << *it;
      it++;
      if(it != end)
        os << ", ";
      else
        break;
    }
    os << ");" << endl;
  }
  return os;
}
