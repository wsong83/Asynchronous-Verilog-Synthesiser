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
using std::vector;
using std::map;
using std::list;

netlist::Port::Port(const VIdentifier& pid)
  : NetComp(tPort), name(*(pid.deep_copy())), ptype(0), dir(-2), signed_flag(false) {}

netlist::Port::Port(const location& lloc, const VIdentifier& pid)
  : NetComp(tPort, lloc), name(*(pid.deep_copy())), ptype(0), dir(-2), signed_flag(false) {}

netlist::Port::Port(const location& lloc)
  : NetComp(tPort, lloc), ptype(0), dir(-2), signed_flag(false) {}

void netlist::Port::set_father(Block *pf) {
  if(father == pf) return;
  father = pf;
  name.set_father(pf);
}

void netlist::Port::db_register(int) {
  if(dir <= 0) name.db_register(0);
  if(dir >= 0) name.db_register(1);
}

void netlist::Port::db_expunge() {
  name.db_expunge();
}

bool netlist::Port::elaborate(std::set<shared_ptr<NetComp> >&,
                              map<shared_ptr<NetComp>, list<shared_ptr<NetComp> > >&) {
  name.reduce();
  return true;
}

Port* netlist::Port::deep_copy() const {
  Port *rv = new Port(loc);
  VIdentifier *mname = name.deep_copy();
  rv->name = *mname;
  delete mname;
  rv->dir = dir;
  rv->ptype = ptype;
  return rv;
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

  if(signed_flag) os << "signed ";
  name.get_range().RangeArrayCommon::streamout(os, 0, "", true, false); // show range of declaration 
  name.get_range().RangeArrayCommon::streamout(os, 1, name.name, true, true); // show dimension of declaration
  os << ";" << endl;

  return os;
}
