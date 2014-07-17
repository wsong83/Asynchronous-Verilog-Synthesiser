/*
 * Copyright (c) 2012-2014 Wei Song <songw@cs.man.ac.uk> 
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
#include <boost/tuple/tuple.hpp>
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
  : Block(tModule), fsm_extracted(false) {}

netlist::Module::Module(const MIdentifier& nm)
  : Block(tModule), name(nm), fsm_extracted(false) { named=true; }

netlist::Module::Module(const shell::location& lloc, const MIdentifier& nm)
  : Block(tModule, lloc), name(nm), fsm_extracted(false) { named=true; }

netlist::Module::Module(const MIdentifier& nm, const shared_ptr<Block>& body)
  : Block(*body), name(nm), fsm_extracted(false) 
{
  ctype = tModule;
  named=true; 
  elab_inparse();
}

netlist::Module::Module(const location& lloc, const MIdentifier& nm, const shared_ptr<Block>& body)
  : Block(*body), name(nm), fsm_extracted(false) 
{
  ctype = tModule;
  named=true; 
  loc = lloc;
  elab_inparse();
}

netlist::Module::Module(const MIdentifier& nm, const list<shared_ptr<Port> >& port_list, const shared_ptr<Block>& body)
  : Block(*body), name(nm), fsm_extracted(false) 
{
  ctype = tModule;
  named=true;
  elab_inparse();
  init_port_list(port_list);
}

netlist::Module::Module(const location& lloc, const MIdentifier& nm, const list<shared_ptr<Port> >& port_list, const shared_ptr<Block>& body)
  : Block(*body), name(nm), fsm_extracted(false) 
{
  ctype = tModule;
  named=true;
  loc = lloc;
  elab_inparse();
  init_port_list(port_list);
}

netlist::Module::Module(const MIdentifier& nm, const list<shared_ptr<Variable> >& para_list,
                        const list<shared_ptr<Port> >& port_list, const shared_ptr<Block>& body)
  : Block(*body), name(nm), fsm_extracted(false) 
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
  : Block(*body), name(nm), fsm_extracted(false) 
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
      os << it->second->name.get_name();
      it++;
      if(it != end)
        os << ", ";
      else
        os << ");" << endl;
    }
  }

  // parameters
  if(db_param.size() > 0) {
    os << endl;
    db_param.streamout(os, indent+2);
  }

  // ports
  if(db_port.size() > 0) {
    os << endl;
    db_port.streamout(os, indent+2);
  }

  // variables
  if(db_var.size() > 0) {
    os << endl;
    db_var.streamout(os, indent+2);
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
  if(db_instance.size() > 0) {
    os << endl;
    db_instance.streamout(os, indent+2);
  }

  // functions
  for_each(db_func.begin(), db_func.end(), [&](const pair<const FIdentifier, shared_ptr<Function> >& m) {
      os << endl;
      m.second->streamout(os, indent+2);
    });
  
  os << endl << string(indent, ' ') << "endmodule" << endl << endl;
  return os;
}

Module* netlist::Module::deep_copy(NetComp* bp) const {
  bool base_call = true;
  Module *rv;
  if(!bp) {
    rv = new Module();
    base_call = false;
  } else
    rv = static_cast<Module *>(bp); // C++ does not support multiple dispatch
  Block::deep_copy(rv);
  rv->set_name(name);
  
  // data in Module;
  DATABASE_DEEP_COPY_FUN(db_port,      VIdentifier,  Port,      rv->db_port     );
  DATABASE_DEEP_COPY_FUN(db_param,     VIdentifier,  Variable,  rv->db_param    );
  DATABASE_DEEP_COPY_FUN(db_instance,  IIdentifier,  Instance,  rv->db_instance );
  DATABASE_DEEP_COPY_FUN(db_func,      FIdentifier,  Function,  rv->db_func     );
  
  // set father
  if(!base_call) {
    rv->set_father();
  }

  return rv;
}

void netlist::Module::db_register(int) {
  // The item in statements are duplicated in db_instance, db_other, db_seqblock, db_assign and db_genblock.
  // Therefore, only statements are executed.
  for_each(db_param.begin_order(), db_param.end_order(), [](pair<const VIdentifier, shared_ptr<Variable> >& m) {
      m.second->db_register(1);
    });
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
  for_each(db_func.begin(), db_func.end(), [](pair<const FIdentifier, shared_ptr<Function> >& m) {
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
  for_each(db_instance.begin(), db_instance.end(), [](pair<const IIdentifier, shared_ptr<Instance> >& m) {
      m.second->db_expunge();
    });
  for_each(db_func.begin(), db_func.end(), [](pair<const FIdentifier, shared_ptr<Function> >& m) {
	m.second->db_expunge();
      });  
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
  DATABASE_SET_FATHER_FUN(db_param, VIdentifier, Variable, this);
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
      G_ENV->error(m->loc, "ELAB-PARA-1", m->pname.get_name(), tmpModule->name.get_name());
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
  newName = tmpModule->name.get_name();
  for_each(tmpModule->db_param.begin_order(), tmpModule->db_param.end_order(), 
           [&](pair<const VIdentifier, shared_ptr<Variable> >& m) {
             if(rv && m.second->vtype == Variable::TParam) {
               rv &= m.second->update();
               if(!rv) 
                 G_ENV->error(m.second->loc, "ELAB-PARA-0", m.second->name.get_name(), tmpModule->name.get_name());
               else
                 newName += string("_") + m.second->name.get_name() + m.second->get_value().get_value().get_str();
             }
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
  
  std::set<VIdentifier> to_del_var;
  typedef pair<const VIdentifier, shared_ptr<Variable> > var_type;
  BOOST_FOREACH(var_type var, db_var.db_list) {
    if(var.second->get_vtype() & (Variable::TParam | Variable::TLParam)) {
      db_param.insert(var.first, var.second);
      to_del_var.insert(var.first);
    }
  }

  BOOST_FOREACH(const VIdentifier& v, to_del_var) {
    db_var.erase(v);
  }

}

bool netlist::Module::elaborate(std::deque<shared_ptr<Module> >& mfifo,
                                map<MIdentifier, shared_ptr<Module> > & mmap) {
  bool rv = true;
  std::set<shared_ptr<NetComp> > to_del;
  map<shared_ptr<NetComp>, list<shared_ptr<NetComp> > > to_add;

  //std::cout << "elab " << name.get_name() << std::endl;

  // reduce all parameters
  for_each(db_param.begin_order(), db_param.end_order(), [&](pair<const VIdentifier, shared_ptr<Variable> >& m) {
      rv &= m.second->elaborate(to_del, to_add);
      if(!m.second->is_valuable()) {
        G_ENV->error(m.second->loc, "ELAB-PARA-0", m.first.get_name(), name.get_name());
        rv = false;
      } else {
        replace_variable(m.first, m.second->get_value());
        for_each(db_param.begin_order(), db_param.end_order(), [&](pair<const VIdentifier, shared_ptr<Variable> >& par) {
            par.second->replace_variable(m.first, m.second->get_value());
          });
        for_each(db_port.begin_order(), db_port.end_order(), [&](pair<const VIdentifier, shared_ptr<Port> >& par) {
            par.second->replace_variable(m.first, m.second->get_value());
          });
      }
    });

  //std::cout << "after param elaboration: " << std::endl << *this;
  if(!rv) 
    return rv;
  else
    db_param.clear();
  
  //std::cout << *this;

  // unfold the module to handle all generation loops
  unfold();

  //std::cout << *this;

  // before register all variable, update the port direction of all instance
  // as it will affect the direction of wires
  for_each(db_instance.begin(), db_instance.end(), [&](pair<const IIdentifier, shared_ptr<Instance> >& m) {
      rv &= m.second->update_ports();
    });
  if(!rv) return rv;

  // link all variables
  db_register(0);

  //std::cout << "after instance port update: " << std::endl << *this;

  if(!rv) return rv;
  // check all variables
  for_each(db_port.begin_order(), db_port.end_order(), [&](pair<const VIdentifier, shared_ptr<Port> >& m) {
      rv &= m.second->elaborate(to_del, to_add);
    });
  //std::cout << "after port elaboration: " << std::endl << *this;
  if(!rv) return rv;
  for_each(db_var.begin_order(), db_var.end_order(), [&](pair<const VIdentifier, shared_ptr<Variable> >& m) {
      rv &= m.second->elaborate(to_del, to_add);
    });
  if(!rv) return rv;

  //std::cout << "after var elaboration: " << std::endl << *this;
  
  // elaborate the internals
  BOOST_FOREACH(shared_ptr<NetComp>& m, statements) 
    rv &= m->elaborate(to_del, to_add);
  if(!rv) return rv;
  
  // elaborate the variables in instances
  for_each(db_instance.begin(), db_instance.end(), 
           [&](pair<const IIdentifier, shared_ptr<Instance> >& m) {
             rv &= m.second->elaborate(to_del, to_add);
           });
  if(!rv) return rv;

  //std::cout << "after statements elaboration: " << std::endl << *this;
  
  // add called modules (instances) to the module queue in cmd/elaborate
  for_each(db_instance.begin(), db_instance.end(), 
           [&](pair<const IIdentifier, shared_ptr<Instance> >& m) {
             rv &= m.second->elaborate(mfifo, mmap);
           });
  
  if(!rv) return rv;

  // remove useless variables
  list<VIdentifier> var_to_be_removed;
  for_each(db_var.begin_order(), db_var.end_order(), [&](pair<const VIdentifier, shared_ptr<Variable> >& m) {
      if(m.second->is_useless()) var_to_be_removed.push_back(m.first);
    });
  BOOST_FOREACH(const VIdentifier& m, var_to_be_removed) 
    db_var.erase(m);

  return rv;
}

shared_ptr<NetComp> netlist::Module::get_sp() {
  return shared_from_this();
}

Module* netlist::Module::get_module() {
  return this;
}

void netlist::Module::get_hier(list<shared_ptr<Module> >& mfifo, 
                               std::set<MIdentifier> & mmap) const{
  list<shared_ptr<Module> > myqueue;
  for_each(db_instance.begin(), db_instance.end(),
           [&](const pair<const IIdentifier, shared_ptr<Instance> >& m) {
             shared_ptr<Module> tarModule = G_ENV->find_module(m.second->mname);
             if(tarModule && !mmap.count(tarModule->name.get_name())) {
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
    G_ENV->error("SDFG-EXTRACT-1", name.get_name());

  if(DFG) return DFG;

  // else, build a new
  shared_ptr<dfgGraph> G(new dfgGraph(name.get_name()));
  
  // put all ports into the list
  for_each(db_port.begin_order(), db_port.end_order(), 
           [&](const pair<const VIdentifier, shared_ptr<Port> >& m) {
             VIdentifier& pname = m.second->name;
             string psig_full_name = 
               SDFG::get_full_selected_name(pname.get_selected_name(),
                                            toString(pname.get_full_range()));
             shared_ptr<dfgNode> nport = G->add_node(pname.get_name() + "_P", dfgNode::SDFG_PORT);
             nport->ptr.insert(m.second);

             // also add the corresponding signal and connect them
             shared_ptr<dfgNode> nsig = G->add_node(psig_full_name, dfgNode::SDFG_DF);
             nsig->ptr.insert(db_var.find(m.first));
             
             // connect signals to ports
             switch(m.second->get_dir()) {
             case  1: {         // output
               nport->type = dfgNode::SDFG_OPORT; 
               G->add_edge(m.first.get_name(), dfgEdge::SDFG_ASS, nsig, nport);
               break;
             }
             case -1: {         // input
               nport->type = dfgNode::SDFG_IPORT; 
               G->add_edge(m.first.get_name(), dfgEdge::SDFG_ASS, nport, nsig);
               break;
             }
             default: {         // inout
               G->add_edge(m.first.get_name(), dfgEdge::SDFG_ASS, nsig, nport);
               G->add_edge(m.first.get_name(), dfgEdge::SDFG_ASS, nport, nsig);
             }
             }
           });

  // put all modules into the graph
  for_each(db_instance.begin(), db_instance.end(),
           [&](const pair<const IIdentifier, shared_ptr<Instance> >& m) {
             if(m.second->type == Instance::modu_inst) {
               shared_ptr<dfgNode> n = G->add_node(m.first.get_name(), dfgNode::SDFG_MODULE);
               n->ptr.insert(m.second);
               shared_ptr<Module> subMod = G_ENV->find_module(m.second->mname);
               if(subMod) { // has sub-module
                 n->child_name = m.second->mname.get_name();
                 n->child = subMod->extract_sdfg(quiet);
                 n->child->father = n.get();
               }
             } else {           // gate
               shared_ptr<dfgNode> n = G->add_node(m.first.get_name(), dfgNode::SDFG_GATE);
               n->ptr.insert(m.second);
             }
           });

  // now cope with internal structures
  BOOST_FOREACH(shared_ptr<NetComp>& m, statements) {
    m->gen_sdfg(G);
  }
  for_each(db_instance.begin(), db_instance.end(),
           [&](const pair<const IIdentifier, shared_ptr<Instance> >& m) {
             m.second->gen_sdfg(G);
           });
  
  G->connect_partial_nodes();

  G->check_integrity();

  DFG = G;
  G->pModule = this;
  return G;
}

void netlist::Module::assign_dataDFG() {
  assert(DataDFG);
  DataDFG->pModule = this;
  for_each(db_instance.begin(), db_instance.end(),
           [&](const pair<const IIdentifier, shared_ptr<Instance> >& m) {
             if(m.second->type == Instance::modu_inst) {
               shared_ptr<Module> subMod = G_ENV->find_module(m.second->mname);
               shared_ptr<dfgNode> dfgMod = DataDFG->get_node(SDFG::divide_signal_name(m.second->name.get_name()));
               assert(subMod);
               if(subMod && dfgMod && dfgMod->child) {
                 subMod->DataDFG = dfgMod->child;
                 subMod->assign_dataDFG();
               }
             } 
           });
}

std::set<string> netlist::Module::extract_fsms(bool verbose, bool force,
                                               shared_ptr<SDFG::dfgGraph> pRRG,
                                               unsigned int& num_n,
                                               unsigned int& num_r,
                                               unsigned int& num_pf) {
  if(!fsm_extracted || force) {
    assert(DFG);
    
    std::set<shared_ptr<SDFG::dfgNode> > dfg_fsms = 
      DFG->get_fsms(verbose, pRRG, num_n, num_r, num_pf);
    FSMs.clear();
    BOOST_FOREACH(shared_ptr<SDFG::dfgNode> n, dfg_fsms)
      FSMs.insert(n->get_full_name());
    fsm_extracted = true;
  }

  for_each(db_instance.begin(), db_instance.end(),
           [&](const pair<const IIdentifier, shared_ptr<Instance> >& m) {
             if(m.second->type == Instance::modu_inst) {
               shared_ptr<Module> subMod = G_ENV->find_module(m.second->mname);
               if(subMod) {
                 std::set<string> fsm_set_m = 
                   subMod->extract_fsms(verbose, force, pRRG, num_n, num_r, num_pf);
                 FSMs.insert(fsm_set_m.begin(), fsm_set_m.end());
               }
             }
           });

  return FSMs;
}

map<string, string> netlist::Module::extract_fsms_new() {
  map<string, string> rv;
  map<shared_ptr<SDFG::dfgNode>, int > dfg_fsms = RRG->get_fsms_new();
  typedef std::pair<const shared_ptr<SDFG::dfgNode>, int> dfg_fsms_type;
  BOOST_FOREACH(dfg_fsms_type f, dfg_fsms) {
    rv[f.first->get_hier_name()] = SDFG::dfgNode::get_fsm_type(f.second);
  }
  return rv;
}

double netlist::Module::get_ratio_state_preserved_oport(map<VIdentifier, pair<bool, string> >& port_ana, const std::set<string>& gFSMs) {
  assert(DFG);
  
  unsigned int num_of_spports = 0; // number of state preserved ports
  unsigned int num_of_oports = 0;  // number of output ports
  
  DataBase<VIdentifier, Port, true>::DBTL::iterator pit, pend;
  for(pit = db_port.begin_order(), pend = db_port.end_order(); pit != pend; ++pit) {
    if(pit->second->get_dir() >= 0) { // output or inout
      shared_ptr<SDFG::dfgNode> pnode = DFG->get_node(SDFG::divide_signal_name(pit->second->name.get_name() + "_P"));
      bool is_state_preserved = false; // preserve state
      bool has_ff_input = false;
      string data_source("");
      BOOST_FOREACH(shared_ptr<SDFG::dfgPath> p, pnode->get_in_paths_fast_cb()) {
        if(p->src->type & (SDFG::dfgNode::SDFG_FF | SDFG::dfgNode::SDFG_LATCH))
          has_ff_input = true;
        
        data_source += "[" + p->src->get_full_name() + ":";
        
        if(gFSMs.count(p->src->get_full_name())) {
          is_state_preserved = true;
          data_source += "self-fsm:";
        } 
        
        bool has_control_fsm = false;
        BOOST_FOREACH(shared_ptr<SDFG::dfgPath> pp, p->src->get_in_paths_fast_cb()) {
          if(gFSMs.count(pp->src->get_full_name())){
            if(!is_state_preserved) is_state_preserved = true;
            if(!has_control_fsm) data_source += "ctl-fsm(";
            data_source += pp->src->get_full_name() + ",";
            has_control_fsm = true;
          }
        }
        
        if(has_control_fsm) {
          data_source[data_source.size()-1] = ')';
          data_source += "]";
        } else if(is_state_preserved) {
          data_source[data_source.size()-1] = ']';
        } else {
          data_source += "data-pipeline]";
        }
      }
      
      
      if(has_ff_input) {
        num_of_oports++;
        port_ana[pit->first] = pair<bool, string>(is_state_preserved, data_source);
        if(is_state_preserved) num_of_spports++;
      } else {
        port_ana[pit->first] = pair<bool, string>(false, string("[through wire]"));
      }
    }
  }
  
  if(num_of_oports > 0)
    return (double)(num_of_spports) / num_of_oports;
  else
    return 0.0;
}

void netlist::Module::cal_partition(const double& acc_ratio, std::ostream& ostm, const std::set<string>& gFSMs, bool verbose) {
  // recursively process starts from sub-modules
  DataBase<IIdentifier, Instance>::DBTM::iterator iit, iend;
  for(iit = db_instance.begin(), iend = db_instance.end(); iit != iend; ++iit) {
    if(iit->second->type == Instance::modu_inst) {
      shared_ptr<Module> subMod = G_ENV->find_module(iit->second->mname);
      subMod->cal_partition(acc_ratio, ostm, gFSMs, verbose);
    }
  }
  if(DFG->father != NULL) {
    map<VIdentifier, pair<bool, string> > port_ana; // port analyses
    typedef pair<const VIdentifier, pair<bool, string> > port_ana_type;
    double r = get_ratio_state_preserved_oport(port_ana, gFSMs);
    if(r >= acc_ratio || verbose) {
      ostm << DFG->father->get_full_name() << "\t(" << name << ") with rate " << r ;
      if(r >= acc_ratio)
        ostm << " >= " << acc_ratio;
      else
        ostm << " < " << acc_ratio;
      ostm << ": " << endl;
      BOOST_FOREACH(port_ana_type p, port_ana) {
        const unsigned int const_size_of_name = 28;
        ostm << string(4, ' ');
        ostm << p.first << 
          string(p.first.get_name().size() > const_size_of_name - 4 ? 4 : const_size_of_name - p.first.get_name().size(), ' ') << 
          p.second.first << "\t" << p.second.second << std::endl;
      }
      ostm << endl;
    }
  }
}

void netlist::Module::partition() {
  // process submodules
  DataBase<IIdentifier, Instance>::DBTM::iterator iit, iend;
  for(iit = db_instance.begin(), iend = db_instance.end(); iit != iend; ++iit) {
    if(iit->second->type == Instance::modu_inst) {
      shared_ptr<Module> subMod = G_ENV->find_module(iit->second->mname);
      subMod->partition();
    }
  }

  if(DFG->father != NULL && DataDFG) {     // sub modules and data path exists
    unsigned int total = 0;
    unsigned int pausible = 0;

    std::list<shared_ptr<SDFG::dfgNode> > oports = DataDFG->get_list_of_nodes(SDFG::dfgNode::SDFG_OPORT, true);
    BOOST_FOREACH(shared_ptr<SDFG::dfgNode> op, oports) {
      // type the output port
      shared_ptr<SDFG::dfgNode> op_dfg = DFG->get_node(SDFG::divide_signal_name(op->get_hier_name()));
      if(op_dfg->get_in_edges_type() == SDFG::dfgEdge::SDFG_ASS)
        op_dfg = op_dfg->get_in_nodes().front();

      enum IO_TYPE {
        IO_MEM              = 0x00001, // memory interface
        IO_HS               = 0x00002  // handshake
      };

      unsigned int op_type = 0;
      
      //std::cout << "analyse " << op_dfg->get_full_name() << std::endl;
      
      // exam for memory interfaces
      {
        std::list<shared_ptr<SDFG::dfgPath> > ipaths = op_dfg->get_in_paths_fast_cb();
        // get the combined type
        unsigned int comb_itype = 0;
        BOOST_FOREACH(shared_ptr<SDFG::dfgPath> p, ipaths)
          comb_itype |= p->type;
        
        if(comb_itype & SDFG::dfgEdge::SDFG_ADR) { // possible a memory output
          std::list<shared_ptr<SDFG::dfgNode> > adr;
          std::list<shared_ptr<SDFG::dfgNode> > mem;
          BOOST_FOREACH(shared_ptr<SDFG::dfgPath> p, ipaths) {
            if(p->type & SDFG::dfgEdge::SDFG_ADR)
              adr.push_back(p->src);
            else if(p->type & SDFG::dfgEdge::SDFG_DAT_MASK && p->src != op_dfg) {
              if(p->src->get_in_edges_type() & SDFG::dfgEdge::SDFG_ADR)
                mem.push_back(p->src);
            }
          }
          if(adr.size() > 0 && mem.size() > 0) {
            std::cout << "M: " << op_dfg->get_full_name() << " [MEM DOUT]: " << std::endl;
            std::cout << std::string(4, ' ');
            BOOST_FOREACH(shared_ptr<SDFG::dfgNode> m, mem)
              std::cout << m->get_full_name() << " ";
            BOOST_FOREACH(shared_ptr<SDFG::dfgNode> a, adr)
              std::cout << "[" << a->get_full_name() << "]";
            std::cout << endl;
            op_type |= IO_MEM;
          }
        }
      }

      // exam for handshakes
      if(!(op_type & IO_MEM))
      {
        // get the control of source
        std::set<shared_ptr<SDFG::dfgNode> > s_ctls;
        if(op_dfg->type & SDFG::dfgNode::SDFG_FF) {
          std::list<shared_ptr<SDFG::dfgPath> > ipaths = op_dfg->get_in_paths_fast_cb();
          BOOST_FOREACH(shared_ptr<SDFG::dfgPath> p, ipaths) {
            if(p->type & SDFG::dfgEdge::SDFG_CTL_MASK)
              s_ctls.insert(p->src);
          }
        } else {
          std::list<shared_ptr<SDFG::dfgPath> > idpaths = op->get_in_paths_fast_cb();
          BOOST_FOREACH(shared_ptr<SDFG::dfgPath> dp, idpaths) {
            shared_ptr<dfgNode> dn = dp->src->pg->pModule->DFG->get_node(SDFG::divide_signal_name(dp->src->get_hier_name()));
            std::list<shared_ptr<SDFG::dfgPath> > ipaths = dn->get_in_paths_fast_cb();
            BOOST_FOREACH(shared_ptr<SDFG::dfgPath> p, ipaths) {
              if(p->type & (SDFG::dfgEdge::SDFG_LOG | SDFG::dfgEdge::SDFG_EQU) && p->src->is_fsm())
                s_ctls.insert(p->src);
            }
          }
        }

        // get the connected datapath node
        std::set<shared_ptr<SDFG::dfgNode> > t_nodes;
        std::list<shared_ptr<SDFG::dfgPath> > odpaths = op->get_out_paths_fast_cb();
        BOOST_FOREACH(shared_ptr<SDFG::dfgPath> p, odpaths) {
          shared_ptr<SDFG::dfgNode> tn = p->tar->pg->pModule->DFG->get_node(SDFG::divide_signal_name(p->tar->get_hier_name()));
          if(tn->type & SDFG::dfgNode::SDFG_FF)
            t_nodes.insert(tn);
        }
        unsigned int h_cnt = 0;
        BOOST_FOREACH(shared_ptr<SDFG::dfgNode> t, t_nodes) {
          std::list<shared_ptr<SDFG::dfgPath> > tcpaths = t->get_in_paths_fast_cb();
          std::set<shared_ptr<SDFG::dfgNode> > cset;
          BOOST_FOREACH(shared_ptr<SDFG::dfgPath> p, tcpaths) {
            if(p->type & (SDFG::dfgEdge::SDFG_LOG | SDFG::dfgEdge::SDFG_EQU) && p->src->is_fsm())
              if(s_ctls.count(p->src))
                cset.insert(p->src);
          }
          if(cset.size()) {
            
            std::cout << "H: " << op_dfg->get_full_name() << " and " << t->get_full_name();
            std::cout << " share the control of:" << std::endl;
            std::cout << std::string(4, ' ');
            BOOST_FOREACH(shared_ptr<SDFG::dfgNode> a, cset) {
              std::cout << "[" << a->get_full_name() << "]";
            }
            std::cout << endl;
            std::cout << std::string(4, ' ');
            std::cout << t_nodes.size() << std::endl;
            
            h_cnt++;
          }
        }
        if(h_cnt && h_cnt == t_nodes.size()) {
          std::cout << "H: " << op_dfg->get_full_name() << " could be a handshake output" << std::endl;
        }
        
      }

    }
  }
}


void netlist::Module::init_port_list(const list<shared_ptr<Port> >& port_list) {
  BOOST_FOREACH(shared_ptr<Port> m, port_list) {
    if(!db_port.find(m->name)) {
      db_port.insert(m->name, m);
    }
    shared_ptr<Port> pp = db_port.find(m->name);
    if(!db_var.find(pp->name)) {
      if(pp->ptype)
        db_var.insert(pp->name, shared_ptr<Variable>(new Variable(pp->loc, pp->name, Variable::TReg)));
      else {
        db_var.insert(pp->name, shared_ptr<Variable>(new Variable(pp->loc, pp->name, Variable::TWire)));
      }
      if(pp->is_signed())
        db_var.find(pp->name)->set_signed();
    }
  }
}

void netlist::Module::init_param_list(const list<shared_ptr<Variable> >& para_list) {
  BOOST_FOREACH(const shared_ptr<Variable>& m, para_list) {
    if(!db_param.find(m->name))
      db_param.insert(m->name, m);
  }
}


