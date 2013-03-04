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
 * A module instance
 * 19/03/2012   Wei Song
 *
 *
 */

#include "component.h"
#include "shell/env.h"
#include <boost/foreach.hpp>
#include "sdfg/rtree.hpp"
#include "sdfg/sdfg.hpp"

using namespace netlist;
using namespace SDFG;
using std::ostream;
using std::endl;
using std::string;
using std::vector;
using boost::shared_ptr;
using std::list;
using std::map;
using std::pair;
using shell::location;


netlist::Instance::Instance()
  : NetComp(tInstance), type(unknown_inst), named(false) {}

netlist::Instance::Instance(const IIdentifier& nm, const list<shared_ptr<PortConn> >& polist)
  : NetComp(tInstance), name(nm), port_list(polist), type(unknown_inst), named(true) { }

netlist::Instance::Instance(const shell::location& lloc, const IIdentifier& nm, const list<shared_ptr<PortConn> >& polist)
  : NetComp(tInstance, lloc), name(nm), port_list(polist), type(unknown_inst), named(true) { }

netlist::Instance::Instance(const IIdentifier& nm, const list<shared_ptr<PortConn> >& polist, type_t itype)
  : NetComp(tInstance), name(nm), port_list(polist), type(itype), named(true) {
  switch(itype) {
  case prim_in_inst: {
    BOOST_FOREACH(shared_ptr<PortConn>& it, port_list) it->set_in();
    port_list.front()->set_out();
    break;
  }
  case prim_out_inst: {
    BOOST_FOREACH(shared_ptr<PortConn>& it, port_list) it->set_out();
    port_list.back()->set_in();
    break;
  }
  default: ;
  }
}

netlist::Instance::Instance(const location& lloc,
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

netlist::Instance::Instance(const location& lloc, 
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
    os << ");" << endl <<endl;
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
  //rv->module_ptr = module_ptr;

  BOOST_FOREACH(const shared_ptr<PortConn>& p, port_list)
    rv->port_list.push_back(shared_ptr<PortConn>(p->deep_copy()));

  BOOST_FOREACH(const shared_ptr<ParaConn>& p, para_list)
    rv->para_list.push_back(shared_ptr<ParaConn>(p->deep_copy()));
    
  return rv;
}

void netlist::Instance::db_register(int) {
  BOOST_FOREACH(shared_ptr<PortConn> p, port_list) p->db_register(1);
  BOOST_FOREACH(shared_ptr<ParaConn> p, para_list) p->db_register(1); 
}

void netlist::Instance::db_expunge() {
  BOOST_FOREACH(shared_ptr<PortConn> p, port_list) p->db_expunge();
  BOOST_FOREACH(shared_ptr<ParaConn> p, para_list) p->db_expunge(); 
}

bool netlist::Instance::update_ports() {
  switch(type) {
  case prim_in_inst:
  case prim_out_inst: break;    // already set in initialisation
  case unknown_inst:
  case modu_inst: {
    // find the module
    shared_ptr<Module> modp = G_ENV->find_module(mname);
    if(modp.use_count() == 0) {
      G_ENV->error(loc, "ELAB-INST-0", mname.name);
      return false;
      // in the future, should check for cell library for library cells
    } else {
      type = modu_inst;
    }
    
    // update port directions
    list<shared_ptr<PortConn> >::iterator it, end;
    for(it=port_list.begin(), end=port_list.end(); it!=end; it++) {
      shared_ptr<Port> portp = modp->find_port((*it)->pname);
      if(portp.use_count() == 0) {
        G_ENV->error(loc, "ELAB-INST-1", (*it)->pname.name, mname.name);
        return false;
      } else {
        (*it)->set_dir(portp->get_dir());
      }
    }
    
    // connect unnamed parameters if they are unnamed
    if(!para_list.empty() && !para_list.front()->is_named()) {
      list<shared_ptr<ParaConn> >::iterator pit, pend;
      list<pair<const VIdentifier, shared_ptr<Variable> > >::iterator mit, mend;
      pit = para_list.begin(); 
      pend = para_list.end(); 
      mit = modp->db_param.begin_order();
      mend = modp->db_param.end_order();
      for(; pit != pend; ++pit, ++mit) {
        while(mit->second->vtype != Variable::TParam) ++mit;
        (*pit)->pname = mit->second->name;
      }
      if(pit!=pend || mit!=mend) { // number of the unnamed parameters do not match
        G_ENV->error(loc, "ELAB-INST-3");
      }
    }
    break;
  }
  case gate_inst:
  default:;
  }
  
  return true;
}

bool netlist::Instance::elaborate(std::set<shared_ptr<NetComp> >& to_del,
                                  map<shared_ptr<NetComp>, list<shared_ptr<NetComp> > >& to_add) {
  BOOST_FOREACH(shared_ptr<PortConn> p, port_list) if(!p->elaborate(to_del, to_add)) return false;
  BOOST_FOREACH(shared_ptr<ParaConn> p, para_list) if(!p->elaborate(to_del, to_add)) return false;
  return true;
}

bool netlist::Instance::elaborate(std::deque<boost::shared_ptr<Module> >& mfifo, 
                                  std::map<MIdentifier, boost::shared_ptr<Module> > & mmap) {
  std::cout << "instance elaboration " << mname << std::endl;

  bool rv = true;

  if(type == modu_inst) {
    // find the module in library
    shared_ptr<Module> tarModule = G_ENV->find_module(mname);
    if(tarModule.use_count() == 0) {
      G_ENV->error(loc,"ELAB-INST-0", mname.name);
      return false;
    }
    
    // calculate the new name
    string newName;
    rv &= tarModule->calculate_name(newName, para_list);
    if(!rv) {
      G_ENV->error(loc, "ELAB-INST-2", mname.name);
      return false;
    }
    
    // update the module name
    set_mname(newName);
    
    // check the new name in module map
    if(!mmap.count(newName)) {
      // if not elaborated yet, add it to the map and the module queue
      shared_ptr<Module> newModule(tarModule->deep_copy());
      mfifo.push_back(newModule);
      mmap[newName] = newModule;
      
      // set up the parameters
      BOOST_FOREACH(shared_ptr<ParaConn> p, para_list) {
        newModule->db_param.find(p->pname)->set_value(p->num);
      }
    }
  }

  para_list.clear();

  return rv;

}

void netlist::Instance::gen_sdfg(shared_ptr<dfgGraph> G) {

  // find out the node
  shared_ptr<dfgNode> node = G->get_node(name.name);
  assert(node);
  
  BOOST_FOREACH(shared_ptr<PortConn> m, port_list) {
    if(m->get_dir() <= 0) {     // input
      switch(m->type) {
      case PortConn::CEXP: {    // expression
        shared_ptr<dfgNode> exp_node = G->add_node(UniName::uni_name(), dfgNode::SDFG_COMB);
        shared_ptr<SDFG::RForest> rf(new SDFG::RForest());
        m->exp->scan_vars(rf, false);
        if(rf->tree.count("@CTL")) {
          BOOST_FOREACH(const string& s, rf->tree["@CTL"]->sig) {
            G->add_edge(s, SDFG::dfgEdge::SDFG_CTL, s, exp_node->name);
          }
        }
        if(rf->tree.count("@DATA")) {
          BOOST_FOREACH(const string& s, rf->tree["@DATA"]->sig) {
            G->add_edge(s, SDFG::dfgEdge::SDFG_DP, s, exp_node->name);
          }
        }
        G->add_edge(exp_node->name, dfgEdge::SDFG_DF, exp_node->name, node->name);
        node->add_port_sig(m->pname.name + "_P", exp_node->name);
        break;
      }
      case PortConn::CVAR: {    // variable
        G->add_edge(m->var.name, dfgEdge::SDFG_DF, m->var.name, node->name);
        node->add_port_sig(m->pname.name + "_P", m->var.name);
        break;
      }
      case PortConn::CNUM: {    // constant number
        break;                  // do nothing
      }
      default:
        assert(0 == "port type wrong, input cannot be open.");
      }
    }
    
    if(m->get_dir() >= 0) {     // output
      switch(m->type) {
      case PortConn::CVAR: {    // variable
        G->add_edge(m->pname.name, dfgEdge::SDFG_DF, node->name, m->var.name);
        node->add_port_sig(m->pname.name + "_P", m->var.name);
        break;
      }
      case PortConn::COPEN: {   // open
        break;                  // do nothing
      }
      default:
        assert(0 == "port type wrong, output cannot be expression or number.");
      }
    }
  }       
}

void netlist::Instance::replace_variable(const VIdentifier& var, const Number& num) {
  BOOST_FOREACH(shared_ptr<PortConn> pc, port_list) {
    pc->replace_variable(var, num);
  }
  BOOST_FOREACH(shared_ptr<ParaConn> pc, para_list) {
    pc->replace_variable(var, num);
  }
}

void netlist::Instance::set_mname(const MIdentifier& mod_name) {
  mname = mod_name;
}
