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
#include "shell/env.h"

using namespace netlist;
using std::ostream;
using std::endl;
using std::string;
using boost::shared_ptr;
using std::list;
using shell::location;


netlist::Instance::Instance(const IIdentifier& nm, const list<shared_ptr<PortConn> >& polist, type_t itype)
  : NetComp(tInstance), name(nm), port_list(polist), type(itype), named(true) {
  switch(itype) {
  case prim_in_inst: {
    list<shared_ptr<PortConn> >::iterator it, end;
    it = port_list.begin();
    (*it)->set_out();
    it++;
    for(end=port_list.end(); it!=end; it++) {
      (*it)->set_in();
    }
    break;
  }
  case prim_out_inst: {
    list<shared_ptr<PortConn> >::iterator it, end;
    it = port_list.begin();
    for(end=port_list.end(); it!=end; it++) {
      (*it)->set_out();
    }
    it--;
    (*it)->set_in();
    break;
  }
  default: ;
  }
}

netlist::Instance::Instance(
                            const location& lloc,
                            const IIdentifier& nm, 
                            const list<shared_ptr<PortConn> >& polist, 
                            type_t itype
                            )
  : NetComp(tInstance, lloc), name(nm), port_list(polist), type(itype), named(true) {
  switch(itype) {
  case prim_in_inst: {
    list<shared_ptr<PortConn> >::iterator it, end;
    it = port_list.begin();
    (*it)->set_out();
    it++;
    for(end=port_list.end(); it!=end; it++) {
      (*it)->set_in();
    }
    break;
  }
  case prim_out_inst: {
    list<shared_ptr<PortConn> >::iterator it, end;
    it = port_list.begin();
    for(end=port_list.end(); it!=end; it++) {
      (*it)->set_out();
    }
    it--;
    (*it)->set_in();
    break;
  }
  default: ;
  }
}

netlist::Instance::Instance(const list<shared_ptr<PortConn> >& polist, type_t itype)
  : NetComp(tInstance), port_list(polist), type(itype), named(false) {
  switch(itype) {
  case prim_in_inst: {
    list<shared_ptr<PortConn> >::iterator it, end;
    it = port_list.begin();
    (*it)->set_out();
    it++;
    for(end=port_list.end(); it!=end; it++) {
      (*it)->set_in();
    }
    break;
  }
  case prim_out_inst: {
    list<shared_ptr<PortConn> >::iterator it, end;
    it = port_list.begin();
    for(end=port_list.end(); it!=end; it++) {
      (*it)->set_out();
    }
    it--;
    (*it)->set_in();
    break;
  }
  default: ;
  }
}

netlist::Instance::Instance(
                            const location& lloc, 
                            const list<shared_ptr<PortConn> >& polist, 
                            type_t itype
                            )
  : NetComp(tInstance, lloc), port_list(polist), type(itype), named(false) {
  switch(itype) {
  case prim_in_inst: {
    list<shared_ptr<PortConn> >::iterator it, end;
    it = port_list.begin();
    (*it)->set_out();
    it++;
    for(end=port_list.end(); it!=end; it++) {
      (*it)->set_in();
    }
    break;
  }
  case prim_out_inst: {
    list<shared_ptr<PortConn> >::iterator it, end;
    it = port_list.begin();
    for(end=port_list.end(); it!=end; it++) {
      (*it)->set_out();
    }
    it--;
    (*it)->set_in();
    break;
  }
  default: ;
  }
}

void netlist::Instance::set_father(Block *pf) {
  if(father == pf) return;
  father = pf;
  name.set_father(pf);
  mname.set_father(pf);
  {
    list<shared_ptr<PortConn> >::iterator it, end;
    for(it=port_list.begin(),end=port_list.end(); it!=end; it++)
      (*it)->set_father(pf);
  }
 
  {
    list<shared_ptr<ParaConn> >::iterator it, end;
    for(it=para_list.begin(),end=para_list.end(); it!=end; it++)
      (*it)->set_father(pf);
  }
}

bool netlist::Instance::check_inparse() {
  bool rv = true;
  {
    list<shared_ptr<PortConn> >::iterator it, end;
    for(it=port_list.begin(),end=port_list.end(); it!=end; it++)
      rv &= (*it)->check_inparse();
  }
 
  {
    list<shared_ptr<ParaConn> >::iterator it, end;
    for(it=para_list.begin(),end=para_list.end(); it!=end; it++)
      rv &= (*it)->check_inparse();
  }
  return rv;
}  

ostream& netlist::Instance::streamout(ostream& os, unsigned int indent) const {
  // the module name
  os << string(indent, ' ') << mname.name << " ";

  // parameter list
  if(!para_list.empty()) {
    list<shared_ptr<ParaConn> >::const_iterator it, end;
    os << "#(";
    it=para_list.begin();
    end=para_list.end(); 
    while(it!= end) {
      os << **it;
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
    list<shared_ptr<PortConn> >::const_iterator it, end;
    it=port_list.begin();
    end=port_list.end(); 
    while(it!= end) {
      os << **it;
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

Instance* netlist::Instance::deep_copy() const {
  Instance* rv = new Instance();
  rv->loc = loc;
  rv->mname = mname;
  rv->name = name;
  rv->named = named;
  rv->type = type;
  rv->module_ptr = module_ptr;

  // lambda expression, need C++0x support
  for_each(port_list.begin(), port_list.end(), [rv](const shared_ptr<PortConn>& comp) { 
      rv->port_list.push_back(shared_ptr<PortConn>(comp->deep_copy())); 
    });
  
  for_each(para_list.begin(), para_list.end(), [rv](const shared_ptr<ParaConn>& comp) { 
      rv->para_list.push_back(shared_ptr<ParaConn>(comp->deep_copy())); 
    });
    
  return rv;
}

void netlist::Instance::db_register(int iod) {
  for_each(port_list.begin(), port_list.end(), [](shared_ptr<PortConn>& m) {m->db_register(1);});
  for_each(para_list.begin(), para_list.end(), [](shared_ptr<ParaConn>& m) {m->db_register(1);});
}

void netlist::Instance::db_expunge() {
  for_each(port_list.begin(), port_list.end(), [](shared_ptr<PortConn>& m) {m->db_expunge();});
  for_each(para_list.begin(), para_list.end(), [](shared_ptr<ParaConn>& m) {m->db_expunge();});
}

bool netlist::Instance::update_ports() {
  shared_ptr<Module> modp = G_ENV->find_module(mname);
  if(modp.use_count() == 0) {
    G_ENV->error(loc, "ELAB-INST-0", mname.name);
    return false;
  }

  list<shared_ptr<PortConn> >::iterator it, end;
  for(it=port_list.begin(), end=port_list.end(); it!=end; it++) {
    shared_ptr<Port> portp = modp->find_port((*it)->pname);
    if(portp.use_count() == 0) {
      G_ENV->error(loc, "ELAB-INST-2", (*it)->pname.name, mname.name);
      return false;
    } else {
      (*it)->set_dir(portp->get_dir());
    }
  }

  return true;
}
