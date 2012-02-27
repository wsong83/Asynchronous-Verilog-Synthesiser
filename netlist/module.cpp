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

ostream& netlist::Module::streamout(ostream& os) const {
  os << name;
  if(db_port.empty()) os << ";" << endl;
  else {
    os << "(";
    map<PoIdentifier, shared_ptr<Port> >::const_iterator it, end;
    it = db_port.begin();
    end = db_port.end();
    while(it != end){
      os << (it->second)->name.name;
      it++;
      if(it != end)
        os << ", ";
      else
        os << ");" << endl;
    }
  }
  os << db_port;
  os << db_wire;
  os << db_reg;
  
  // continueous assignments
  {
    map<string, shared_ptr<Assign> >::const_iterator it, end;
    for(it = db_assign.begin(), end = db_assign.end(); it != end; it++)
      os << "assign " << *(it->second) << ";" << endl;
  }

  os << "endmodule" << endl;
  return os;
}