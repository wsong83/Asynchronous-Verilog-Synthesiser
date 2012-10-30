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
#include "shell/env.h"
#include <algorithm>
#include <boost/foreach.hpp>
#include "sdfg/sdfg.hpp"

using namespace netlist;
using namespace SDFG;
using std::ostream;
using std::endl;
using std::string;
using boost::shared_ptr;
using boost::static_pointer_cast;
using std::list;
using std::pair;
using std::map;
using shell::location;
using std::for_each;

netlist::Module::Module()
  : Block(tModule) {}

netlist::Module::Module(const MIdentifier& nm)
  : Block(tModule), name(nm) { named=true; }

netlist::Module::Module(const shell::location& lloc, const MIdentifier& nm)
  : Block(tModule, lloc), name(nm) { named=true; }

netlist::Module::Module(const MIdentifier& nm, const shared_ptr<Block>& body)
  : Block(*body), name(nm) 
{
  ctype = tModule;
  named=true; 
  elab_inparse();
}

netlist::Module::Module(const location& lloc, const MIdentifier& nm, const shared_ptr<Block>& body)
  : Block(*body), name(nm) 
{
  ctype = tModule;
  named=true; 
  loc = lloc;
  elab_inparse();
}

netlist::Module::Module(const MIdentifier& nm, const list<shared_ptr<Port> >& port_list, const shared_ptr<Block>& body)
  : Block(*body), name(nm) 
{
  ctype = tModule;
  named=true;
  elab_inparse();
  init_port_list(port_list);
}

netlist::Module::Module(const location& lloc, const MIdentifier& nm, const list<shared_ptr<Port> >& port_list, const shared_ptr<Block>& body)
  : Block(*body), name(nm) 
{
  ctype = tModule;
  named=true;
  loc = lloc;
  elab_inparse();
  init_port_list(port_list);
}

netlist::Module::Module(const MIdentifier& nm, const list<shared_ptr<Variable> >& para_list,
                        const list<shared_ptr<Port> >& port_list, const shared_ptr<Block>& body)
  : Block(*body), name(nm) 
{
  ctype = tModule;
  named=true;
  elab_inparse();
  init_param_list(para_list);
  init_port_list(port_list);
}

netlist::Module::Module(const location& lloc, const MIdentifier& nm, 
                        const list<shared_ptr<Variable> >& para_list,
                        const list<shared_ptr<Port> >& port_list, const shared_ptr<Block>& body)
  : Block(*body), name(nm) 
{
  ctype = tModule;
  named = true;
  loc = lloc;
  elab_inparse();
  init_param_list(para_list);
  init_port_list(port_list);
}


ostream& netlist::Module::streamout(ostream& os, unsigned int indent) const {
  os << string(indent, ' ') << name;
  if(db_port.empty()) os << ";" << endl;
  else {
    os << "(";
    list<pair<const VIdentifier, shared_ptr<Port> > >::const_iterator it, end;
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

  // parameters
  // variables
  if(db_var.size() > 0) {
    os << endl;
    db_var.streamout(os, indent+2);
  }
  // ports
  if(db_port.size() > 0) {
    os << endl;
    db_port.streamout(os, indent+2);
  }

  // statements
  ctype_t mt = tUnknown;
  BOOST_FOREACH(const shared_ptr<NetComp>& it, statements) {
    ctype_t mt_nxt = it->get_type();
    if(mt != mt_nxt || mt != tAssign) { 
      os << endl; mt = mt_nxt; 
    }
    it->streamout(os, indent+2);
  }

  // instances
  os << db_instance.size() << endl;
  if(db_instance.size() > 0) {
    os << endl;
    db_instance.streamout(os, indent+2);
  }

  os << endl << string(indent, ' ') << "endmodule" << endl << endl;
  return os;
}

Module* netlist::Module::deep_copy() const {
  Module* rv = new Module();
  rv->loc = loc;
  rv->set_name(name);
  
  // data in Block
  // lambda expression, need C++0x support
  BOOST_FOREACH(const shared_ptr<NetComp>& comp, statements)
    rv->statements.push_back(shared_ptr<NetComp>(comp->deep_copy())); 
  
  DATABASE_DEEP_COPY_FUN(db_var,      VIdentifier, Variable,  rv->db_var        );
  rv->unnamed_block = unnamed_block;
  rv->unnamed_instance = unnamed_instance;
  rv->unnamed_var = unnamed_var;
  rv->blocked = blocked;

  // data in Module;
  DATABASE_DEEP_COPY_FUN(db_port,   VIdentifier, Port,      rv->db_port        );
  DATABASE_DEEP_COPY_FUN(db_param,  VIdentifier,  Variable,  rv->db_param      );
  
  // set father
  rv->set_father();

  // SDFG
  rv->DFG = DFG;
  rv->RG = RG;

  return rv;
}

void netlist::Module::db_register(int) {
  // The item in statements are duplicated in db_instance, db_other, db_seqblock, db_assign and db_genblock.
  // Therefore, only statements are executed.
  for_each(db_port.begin_order(), db_port.end_order(), [](pair<const VIdentifier, shared_ptr<Port> >& m) {
      m.second->db_register(1);
    });
  for_each(db_var.begin_order(), db_var.end_order(), [](pair<const VIdentifier, shared_ptr<Variable> >& m) {
      m.second->db_register(1);
    });
  BOOST_FOREACH(shared_ptr<NetComp>& m, statements) m->db_register(1);
  for_each(db_instance.begin(), db_instance.end(), [](pair<const IIdentifier, shared_ptr<Instance> >& m) {
      m.second->db_register(1);
    });
}

void netlist::Module::db_expunge() {
  for_each(db_port.begin_order(), db_port.end_order(), [](pair<const VIdentifier, shared_ptr<Port> >& m) {
      m.second->db_expunge();
    });
  for_each(db_var.begin_order(), db_var.end_order(), [](pair<const VIdentifier, shared_ptr<Variable> >& m) {
      m.second->db_expunge();
    });
  BOOST_FOREACH(shared_ptr<NetComp>& m, statements) m->db_expunge();
}

/* find a variable in current block*/
shared_ptr<Variable> netlist::Module::find_var(const VIdentifier& key) const {
  shared_ptr<Variable>     rv = db_param.find(key);
  if(rv.use_count() == 0)  rv = db_var.find(key);
  return rv;
}

/* find a variable in the global environment, up to the module level */
shared_ptr<Variable> netlist::Module::gfind_var(const VIdentifier& key) const {
  return find_var(key);         // for a module, it is the highest level
}

shared_ptr<Port> netlist::Module::find_port(const VIdentifier& key) const {
  return db_port.find(key);
}

shared_ptr<NetComp> netlist::Module::search(const string& key) const {
  shared_ptr<NetComp> rv = find_var(key);
  if(!rv)             rv = find_instance(key);
  return rv;
}

void netlist::Module::set_father() {
  // macros defined in database.h
  DATABASE_SET_FATHER_FUN(db_port, VIdentifier, Port, this);
  Block::set_father();
}

bool netlist::Module::calculate_name( string& newName,
                                      const list<shared_ptr<ParaConn> >& mplist) const {
  bool rv = true;
  
  // generate a new module for name calculation
  shared_ptr<Module> tmpModule(new Module(name));
  DATABASE_DEEP_COPY_FUN(db_param,  VIdentifier,  Variable,  tmpModule->db_param      );

  // set the new value
  BOOST_FOREACH(const shared_ptr<ParaConn>& m, mplist) {
    shared_ptr<Variable> paramp = tmpModule->db_param.find(m->pname);
    if(paramp.use_count() == 0) {
      G_ENV->error(m->loc, "ELAB-PARA-1", m->pname.name, tmpModule->name.name);
      rv = false;
      break;
    }
    switch(m->type) {
    case ParaConn::CEXP:
      paramp->set_value(m->exp); break;
    case ParaConn::CVAR:
      paramp->set_value(m->var); break;
    case ParaConn::CNUM:
      paramp->set_value(m->num); break;
    default:;
    }
  }
  if(!rv) return rv;

  tmpModule->set_father();
  tmpModule->db_register(0);

  // resolve all parameters and get the new name
  newName = tmpModule->name.name;
  for_each(tmpModule->db_param.begin_order(), tmpModule->db_param.end_order(), 
           [&](pair<const VIdentifier, shared_ptr<Variable> >& m) {
             rv &= m.second->update();
             if(!rv) 
               G_ENV->error(m.second->loc, "ELAB-PARA-0", m.second->name.name, tmpModule->name.name);
             else
               newName += string("_") + m.second->name.name + m.second->get_value().get_value().get_str();
           });
  return rv;
}

void netlist::Module::elab_inparse() {
  std::set<shared_ptr<NetComp> > to_del;

  Block::elab_inparse();

  BOOST_FOREACH(shared_ptr<NetComp> st, statements) {
    switch(st->get_type()) {
    case tPort: {
      SP_CAST(m, Port, st);
      db_port.swap(m->name, m);
      to_del.insert(st);
      break;
    }
    default: ;
    }
  }
  
  
  BOOST_FOREACH(shared_ptr<NetComp> var, to_del) {
    statements.remove(var);
  }
  
  typedef pair<const VIdentifier, shared_ptr<Variable> > var_type;
  BOOST_FOREACH(var_type var, db_var.db_list) {
    if(var.second->get_vtype() == Variable::TParam) {
      db_param.insert(var.first, var.second);
    }
  }

}


bool netlist::Module::elaborate(std::deque<shared_ptr<Module> >& mfifo,
                                map<MIdentifier, shared_ptr<Module> > & mmap) {
  bool rv = true;
  std::set<shared_ptr<NetComp> > to_del;
  map<shared_ptr<NetComp>, list<shared_ptr<NetComp> > > to_add;

  // before register all variable, update the port direction of all instance
  // as it will affect the direction of wires
  for_each(db_instance.begin(), db_instance.end(), [&](pair<const IIdentifier, shared_ptr<Instance> >& m) {
      rv &= m.second->update_ports();
    });
  if(!rv) return rv;

  // link all variables
  db_register(0);

  //std::cout << "after instance port update: " << std::endl << *this;

  // update the value of parameter to all variables after db_register
  // the update during update_name is not sufficient to resolve all parameters 
  // as db_register is run after elaboration
  for_each(db_param.begin_order(), db_param.end_order(), [&](pair<const VIdentifier, shared_ptr<Variable> >& m) {
      rv &= m.second->update();
    });
  if(!rv) return rv;

  //std::cout << "after parameter update: " << std::endl << *this;

  // check all variables
  for_each(db_var.begin_order(), db_var.end_order(), [&](pair<const VIdentifier, shared_ptr<Variable> >& m) {
      rv &= m.second->elaborate(to_del, to_add);
    });
  if(!rv) return rv;

  //std::cout << "after var elaboration: " << std::endl << *this;
  
  // resolve all generate variables
  //for_each(db_genvar.begin_order(), db_genvar.end_order(), [](pair<const VIdentifier, shared_ptr<Variable> >& m) {
  //      m.second->update();
  //    });
  
  //std::cout << "after genvar elaboration: " << std::endl << *this;
  
  // elaborate the internals
  BOOST_FOREACH(shared_ptr<NetComp>& m, statements) 
    rv &= m->elaborate(to_del, to_add);
  if(!rv) return rv;
  
  //std::cout << "after statements elaboration: " << std::endl << *this;
  
  // remove useless variables
  list<VIdentifier> var_to_be_removed;
  for_each(db_var.begin_order(), db_var.end_order(), [&](pair<const VIdentifier, shared_ptr<Variable> >& m) {
      if(m.second->is_useless()) var_to_be_removed.push_back(m.first);
    });
  BOOST_FOREACH(const VIdentifier& m, var_to_be_removed) 
    db_var.erase(m);
  BOOST_FOREACH(const VIdentifier& m, var_to_be_removed) 
    db_param.erase(m);
  
  //if(!db_param.empty()) std::cout << *this << endl;
  assert(db_param.empty());

  // add called modules (instances) to the module queue in cmd/elaborate
  for_each(db_instance.begin(), db_instance.end(), 
           [&](pair<const IIdentifier, shared_ptr<Instance> >& m) {
             rv &= m.second->elaborate(mfifo, mmap);
           });
  
  return rv;
}

void netlist::Module::get_hier(list<shared_ptr<Module> >& mfifo, 
                               std::set<MIdentifier> & mmap) const{
  list<shared_ptr<Module> > myqueue;
  for_each(db_instance.begin(), db_instance.end(),
           [&](const pair<const IIdentifier, shared_ptr<Instance> >& m) {
             shared_ptr<Module> tarModule = G_ENV->find_module(m.second->mname);
             if(tarModule && !mmap.count(tarModule->name.name)) {
               mfifo.push_front(tarModule);
               myqueue.push_front(tarModule);
               mmap.insert(tarModule->name);
             }
           });
  // breadth first
  BOOST_FOREACH(shared_ptr<Module>& m, myqueue) 
    m->get_hier(mfifo, mmap);
}

shared_ptr<dfgGraph> netlist::Module::extract_sdfg(bool quiet) {
  if(!quiet)
    G_ENV->error("SDFG-EXTRACT-1", name.name);

  shared_ptr<dfgGraph> G(new dfgGraph(name.name));
  
  // put all ports into the list
  for_each(db_port.begin_order(), db_port.end_order(), 
           [&](const pair<const VIdentifier, shared_ptr<Port> >& m) {
             shared_ptr<dfgNode> n = G->add_node(m.first.name + "_P", dfgNode::SDFG_PORT);
             switch(m.second->get_dir()) {
             case  1: n->type = dfgNode::SDFG_OPORT; break;
             case -1: n->type = dfgNode::SDFG_IPORT; break;
             default: ;
             }
             n->ptr = m.second;
           });

  // put all signals into the list
  for_each(db_var.begin_order(), db_var.end_order(),
           [&](const pair<const VIdentifier, shared_ptr<Variable> >& m) {
             shared_ptr<dfgNode> n = G->add_node(m.first.name, dfgNode::SDFG_DF);
             n->ptr = m.second;
           });
  
  // link port to signals
  for_each(db_port.begin_order(), db_port.end_order(), 
           [&](const pair<const VIdentifier, shared_ptr<Port> >& m) {
             switch(m.second->get_dir()) {
             case  1:           // output
               G->add_edge(m.first.name, dfgEdge::SDFG_DF, m.first.name, m.first.name + "_P"); 
               break;
             case -1:           // input
               G->add_edge(m.first.name, dfgEdge::SDFG_DF, m.first.name + "_P", m.first.name); 
               break;
             default:           // inout
               G->add_edge(m.first.name, dfgEdge::SDFG_DF, m.first.name, m.first.name + "_P"); 
               G->add_edge(m.first.name, dfgEdge::SDFG_DF, m.first.name + "_P", m.first.name);
             }
           });

  // put all modules into the graph
  for_each(db_instance.begin(), db_instance.end(),
           [&](const pair<const IIdentifier, shared_ptr<Instance> >& m) {
             shared_ptr<dfgNode> n = G->add_node(m.first.name, dfgNode::SDFG_MODULE);
             n->ptr = m.second;
             shared_ptr<Module> subMod = G_ENV->find_module(m.second->mname);
             if(subMod) { // has sub-module
               n->child_name = m.second->mname.name;
               n->child = subMod->extract_sdfg(quiet);
               n->child->father = n.get();
             }
           });

  // now cope with internal structures
  BOOST_FOREACH(shared_ptr<NetComp>& m, statements) {
    m->gen_sdfg(G, std::set<string>(), std::set<string>(), std::set<string>());
  }

  return G;
}

void netlist::Module::init_port_list(const list<shared_ptr<Port> >& port_list) {
  BOOST_FOREACH(shared_ptr<Port> m, port_list) {
    if(!db_port.find(m->name)) {
      db_port.insert(m->name, m);
    }
    if(!db_var.find(m->name)) {
      if(m->ptype)
        db_var.insert(m->name, shared_ptr<Variable>(new Variable(m->loc, m->name, Variable::TReg)));
      else
        db_var.insert(m->name, shared_ptr<Variable>(new Variable(m->loc, m->name, Variable::TWire)));
    }
  }
}

void netlist::Module::init_param_list(const list<shared_ptr<Variable> >& para_list) {
  BOOST_FOREACH(const shared_ptr<Variable>& m, para_list) {
    if(!db_param.find(m->name))
      db_param.insert(m->name, m);
  }
}


