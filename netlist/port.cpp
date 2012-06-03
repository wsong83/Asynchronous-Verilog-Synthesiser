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
 * Ports
 * 16/02/2012   Wei Song
 *
 *
 */

#include "component.h"
#include "shell/env.h"

using namespace netlist;
using std::ostream;
using std::endl;
using std::string;
using boost::shared_ptr;
using shell::location;

netlist::Port::Port(const VIdentifier& pid)
  : NetComp(tPort), name(pid), dir(0) {}

netlist::Port::Port(const location& lloc, const VIdentifier& pid)
  : NetComp(tPort, lloc), name(pid), dir(0) {}

void netlist::Port::set_father(Block *pf) {
  if(father == pf) return;
  father = pf;
  name.set_father(pf);
}

bool netlist::Port::check_inparse() {
  shared_ptr<Variable> m = father->gfind_var(VIdentifier(name.name));
  if(m.use_count() == 0) {      // port without wire/reg definition
    G_ENV->error(loc, "SYN-PORT-2", name.name);
    father->db_var.insert(VIdentifier(name.name), shared_ptr<Variable>( new Variable(*this)));
  }
  return true;
}


ostream& netlist::Port::streamout(ostream& os, unsigned int indent) const {
  os << string(indent, ' ');
  if(is_in())
    os << "input ";
  else if(is_out())
    os << "output ";
  else if(is_inout())
    os << "inout ";
  else
    os << "UNKOWN_port ";

  os << name << ";" << endl;

  return os;

}

bool netlist::Port::elaborate() {
  name.set_pcomp(this);
  return name.elaborate();
}
