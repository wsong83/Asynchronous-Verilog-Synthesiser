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
 * the part of a SDFG graph related to AVS
 * 01/10/2012   Wei Song
 *
 *
 */

#include "dfg_node.hpp"
#include "dfg_edge.hpp"
#include "dfg_path.hpp"
#include "dfg_graph.hpp"

#include <boost/foreach.hpp>

#include "shell/env.h"

#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
using namespace boost::filesystem;

using namespace SDFG;
using boost::shared_ptr;
using std::map;
using std::string;
using std::pair;
using std::list;

void SDFG::dfgGraph::edge_type_propagate() {
  // get a list of all nodes
  list<shared_ptr<dfgNode> > nlook_list;     // list of node to be processed
  std::set<shared_ptr<dfgNode> > nlook_set;  // set of nodes to be processed
  std::set<shared_ptr<dfgNode> > nall_set;   // all nodes that has been visited
  BOOST_FOREACH(shared_ptr<dfgNode> n, 
                get_list_of_nodes(dfgNode::SDFG_FF|dfgNode::SDFG_LATCH|dfgNode::SDFG_PORT)) {
    nlook_list.push_back(n);
    nlook_set.insert(n);
    nall_set.insert(n);
  }
  
  while(nlook_list.size()) {
    shared_ptr<dfgNode> node = nlook_list.front();
    nlook_list.pop_front();
    nlook_set.erase(node);

    switch(node->type) {
    case dfgNode::SDFG_DF: 
    case dfgNode::SDFG_GATE:
    case dfgNode::SDFG_COMB:
    case dfgNode::SDFG_IPORT:
    case dfgNode::SDFG_OPORT:
    case dfgNode::SDFG_PORT:
      edge_type_propagate_combi(node, nlook_list, nlook_set, nall_set);
      break;
    case dfgNode::SDFG_FF:
    case dfgNode::SDFG_LATCH:
      edge_type_propagate_reg(node, nlook_list, nlook_set, nall_set);
      break;
    default:
      assert(0 == "wrong SDFG node type!");
    }
  }
}

void SDFG::dfgGraph::edge_type_propagate_combi(shared_ptr<dfgNode> node, 
                                               list<shared_ptr<dfgNode> >& nlook_list,
                                               std::set<shared_ptr<dfgNode> >& nlook_set,
                                               std::set<shared_ptr<dfgNode> >& nall_set
                                               ) {
  if((node->type & dfgNode::SDFG_IPORT)  == dfgNode::SDFG_IPORT 
     && node->pg->father == NULL) return; // top level input
  
  BOOST_FOREACH(shared_ptr<dfgNode> n, node->get_in_nodes_cb(false)) {
    if(!nall_set.count(n)) {
      nlook_list.push_back(n);
      nlook_set.insert(n);
      nall_set.insert(n);
    }
  }

  if(node->type == dfgNode::SDFG_OPORT && node->pg->father == NULL) {
    // top level output
    return;
  }   

  // propagate to inputs
  int otype = node->get_out_edges_type_cb();
  if(0 == (otype & ~dfgEdge::SDFG_CTL_MASK)) {
    int etype;
    // fanouts are all control
    switch(otype) {
    case dfgEdge::SDFG_CMP: etype = dfgEdge::SDFG_CMP; break;
    case dfgEdge::SDFG_EQU: etype = dfgEdge::SDFG_EQU; break;
    case dfgEdge::SDFG_ADR: etype = dfgEdge::SDFG_ADR; break;
    default:
      etype = dfgEdge::SDFG_CTL;
    }

    BOOST_FOREACH(shared_ptr<dfgEdge> e, node->get_in_edges_cb(false)) {
      if(e->type & dfgEdge::SDFG_DAT_MASK) {
        e->type = dfgEdge::edge_type_t(dfgPath::cal_type(e->type, etype));
        shared_ptr<dfgNode> src = e->get_source_cb();
        
        if(src->type != dfgNode::SDFG_FF && 
           src->type != dfgNode::SDFG_LATCH &&
           !nlook_set.count(src)) {
          nlook_list.push_back(src);
          nlook_set.insert(src);
        }
      }
    }
  }

  // propagate to outputs
  int itype = node->get_in_edges_type_cb();
  if(0 == (itype & ~dfgEdge::SDFG_CTL_MASK)) {
    int etype;
    // fanouts are all control
    switch(itype) {
    case dfgEdge::SDFG_CMP: etype = dfgEdge::SDFG_CMP; break;
    case dfgEdge::SDFG_EQU: etype = dfgEdge::SDFG_EQU; break;
    case dfgEdge::SDFG_ADR: etype = dfgEdge::SDFG_ADR; break;
    default:
      etype = dfgEdge::SDFG_CTL;
    }

    BOOST_FOREACH(shared_ptr<dfgEdge> e, node->get_out_edges_cb(false)) {
      if(e->type & dfgEdge::SDFG_DAT_MASK) {
        e->type = dfgEdge::edge_type_t(dfgPath::cal_type(etype, e->type));
        shared_ptr<dfgNode> tar = e->get_target_cb().front();
        
        if(tar->type != dfgNode::SDFG_FF && 
           tar->type != dfgNode::SDFG_LATCH &&
           !nlook_set.count(tar)) {
          nlook_list.push_back(tar);
          nlook_set.insert(tar);
        }
      }
    }
  }
} 

void SDFG::dfgGraph::edge_type_propagate_reg(shared_ptr<dfgNode> node, 
                                             list<shared_ptr<dfgNode> >& nlook_list,
                                             std::set<shared_ptr<dfgNode> >& nlook_set,
                                             std::set<shared_ptr<dfgNode> >& nall_set
                                             ) {
  BOOST_FOREACH(shared_ptr<dfgNode> n, node->get_in_nodes_cb(false)) {
    if(!nall_set.count(n)) {
      nlook_list.push_back(n);
      nlook_set.insert(n);
      nall_set.insert(n);
    }

    if(n->type != dfgNode::SDFG_FF && 
       n->type != dfgNode::SDFG_LATCH &&
       !nlook_set.count(n)
       ) {
      nlook_list.push_back(n);
      nlook_set.insert(n);
    }
  }
}

shared_ptr<dfgGraph> SDFG::dfgGraph::extract_datapath_new(bool with_fsm, bool with_ctl, bool to_rrg) const {

  shared_ptr<dfgGraph> hier_rrg(deep_copy());

  hier_rrg->remove_control_nodes();
  
  if(to_rrg)
    return hier_rrg->get_RRG();
  else
    return hier_rrg;

}

void SDFG::dfgGraph::remove_control_nodes(bool hier) {

  list<shared_ptr<dfgEdge> > elook_list;
  
  BOOST_FOREACH(edges_type erec, edges)
    elook_list.push_back(erec.second);

  BOOST_FOREACH(shared_ptr<dfgEdge> e, elook_list) {
    if(e->type & (dfgEdge::SDFG_CTL_MASK|dfgEdge::SDFG_CR_MASK))
      remove_edge(e);
  }

  BOOST_FOREACH(shared_ptr<dfgNode> n, get_list_of_nodes(dfgNode::SDFG_MODULE))
    n->child->remove_control_nodes(false);

  remove_useless_nodes();
  check_integrity();
  edge_type_propagate();  

}


shared_ptr<dfgGraph> SDFG::dfgGraph::extract_datapath(bool with_fsm, bool with_ctl) const {

  // first get a hierarchical RRG from the SDFG
  shared_ptr<dfgGraph> hier_rrg = get_hier_RRG(false);
  hier_rrg->edge_type_propagate();

  //shared_ptr<dfgGraph> hier_rrg = get_RRG();

  list<shared_ptr<dfgNode> > nlist;          // nodes to be processed
  BOOST_FOREACH(nodes_type nn, hier_rrg->nodes) {
    nlist.push_back(nn.second);
  }
  
  // get all sub modules ready
  BOOST_FOREACH(shared_ptr<dfgNode> n, nlist) {
    if(n->type & dfgNode::SDFG_MODULE) {
      n->set_new_child(n->child->extract_datapath(with_fsm, with_ctl));
    }
  }

  // remove default nodes
  BOOST_FOREACH(shared_ptr<dfgNode> n, nlist) {
    if(n->type == dfgNode::SDFG_DF) {
      // remove all none data|control edges
      BOOST_FOREACH(shared_ptr<dfgEdge> e, n->get_in_edges(false)) {
        if(!(e->type & (dfgEdge::SDFG_DAT_MASK | dfgEdge::SDFG_CTL_MASK)))
          hier_rrg->remove_edge(e);
      }
      
      BOOST_FOREACH(shared_ptr<dfgEdge> e, n->get_out_edges(false)) {
        if(!(e->type & (dfgEdge::SDFG_DAT_MASK | dfgEdge::SDFG_CTL_MASK)))
          hier_rrg->remove_edge(e);
      }

      // remove the link to unused submodule ports
      BOOST_FOREACH(shared_ptr<dfgEdge> e, n->get_in_edges(false)) {
        shared_ptr<dfgNode> src = e->get_source();
        if(src->type == dfgNode::SDFG_MODULE && !src->sig2port.count(n->get_hier_name()))
          hier_rrg->remove_edge(e);
      }

      BOOST_FOREACH(shared_ptr<dfgEdge> e, n->get_out_edges(false)) {
        shared_ptr<dfgNode> tar = e->get_target();
        if(tar->type == dfgNode::SDFG_MODULE && !tar->sig2port.count(n->get_hier_name()))
          hier_rrg->remove_edge(e);
      }

      // get rid of the edges connected to non-existed submodule ports
      if((n->size_in_edges(false) == 0) || (n->size_out_edges(false) == 0))
        hier_rrg->remove_node(n);
    }
  }
  
  hier_rrg->remove_useless_nodes();
  hier_rrg->edge_type_propagate();
  //hier_rrg->check_integrity();

  BOOST_FOREACH(shared_ptr<dfgNode> n, nlist) {
    if((n->type & (dfgNode::SDFG_FF | dfgNode::SDFG_LATCH)) && hier_rrg->exist(n)) {
      int dp_type = 0;
      int itype = n->get_in_edges_type(false);
      int otype = n->get_out_edges_type(false);      

      if(n->is_fsm()) dp_type |= dfgNode::SDFG_DP_FSM;
      
      if((otype & dfgEdge::SDFG_DAT_MASK) ) // || 
                                            // ((itype & dfgEdge::SDFG_DAT_MASK) && 
                                            // (otype & (dfgEdge::SDFG_EQU))
                                            // | dfgEdge::SDFG_CMP | dfgEdge::SDFG_ADR))
                                            // )
                                            // )
        dp_type |= dfgNode::SDFG_DP_DATA;
      
      if(otype & dfgEdge::SDFG_CTL_MASK)
        dp_type |= dfgNode::SDFG_DP_CTL;

      n->dp_type = (dfgNode::datapath_type_t)(dp_type);
    } else if(n->type == dfgNode::SDFG_DF && hier_rrg->exist(n)) {
      // remove all none data edges
      BOOST_FOREACH(shared_ptr<dfgEdge> e, n->get_in_edges(false)) {
        if(!(e->type & (dfgEdge::SDFG_DAT_MASK))) // | dfgEdge::SDFG_EQU )))//| dfgEdge::SDFG_CMP | dfgEdge::SDFG_ADR)))
          hier_rrg->remove_edge(e);
      }
      
      BOOST_FOREACH(shared_ptr<dfgEdge> e, n->get_out_edges(false)) {
        if(!(e->type & (dfgEdge::SDFG_DAT_MASK))) // | dfgEdge::SDFG_EQU )))//| dfgEdge::SDFG_CMP | dfgEdge::SDFG_ADR)))
          hier_rrg->remove_edge(e);
      }
    }
  }

  // begin the trim
  BOOST_FOREACH(shared_ptr<dfgNode> n, nlist) {
    if((n->type & (dfgNode::SDFG_FF|dfgNode::SDFG_LATCH)) && hier_rrg->exist(n)) {
      if(!(n->dp_type & dfgNode::SDFG_DP_DATA) && (n->dp_type & dfgNode::SDFG_DP_CTL)) {
        if(with_fsm && (n->dp_type & dfgNode::SDFG_DP_FSM)) continue;
        bool keep = false;
        if(with_ctl) {
          BOOST_FOREACH(shared_ptr<dfgNode> nn, n->get_out_nodes(false)) {
            if(nn->dp_type & dfgNode::SDFG_DP_DATA) { keep = true; break; }
          }
        }
        if(!keep) hier_rrg->remove_node(n);
      }
    }else if(n->type == dfgNode::SDFG_DF && hier_rrg->exist(n)) {
      if((n->size_in_edges(false) == 0) || (n->size_out_edges(false) == 0))
        hier_rrg->remove_node(n);
    }
  }
  
  hier_rrg->remove_useless_nodes();
  hier_rrg->edge_type_propagate();
  //hier_rrg->check_integrity();
  
  // count from input
  std::set<shared_ptr<dfgNode> > nkeep;      // nodes to keep
  list<shared_ptr<dfgNode> > nlook = hier_rrg->get_list_of_nodes(dfgNode::SDFG_OPORT);
  std::set<shared_ptr<dfgNode> > nlook_set;
  BOOST_FOREACH(shared_ptr<dfgNode> n, nlook)
    nlook_set.insert(n);
  while(nlook.size()) {
    shared_ptr<dfgNode> n = nlook.front();
    nlook.pop_front();
    
    if(n->size_in_edges() > 0) {
      nkeep.insert(n);
      BOOST_FOREACH(shared_ptr<dfgNode> nn, n->get_in_nodes(false)) {
        if(!nkeep.count(nn)) {
          nkeep.insert(nn);
          if(((!(nn->type & (dfgNode::SDFG_FF|dfgNode::SDFG_LATCH))) 
              || (nn->dp_type & dfgNode::SDFG_DP_DATA)) 
             && !nlook_set.count(nn)) {
            nlook.push_back(nn);
            nlook_set.insert(nn);
          }
        }
      }
    }
  }
  

  // to finally trim the graph
  std::set<shared_ptr<dfgNode> > node_all;
  BOOST_FOREACH(nodes_type nn, hier_rrg->nodes) {
    node_all.insert(nn.second);
  }

  BOOST_FOREACH(shared_ptr<dfgNode> n, node_all) {
    if(!nkeep.count(n))
      hier_rrg->remove_node(n);
  }

  //hier_rrg->check_integrity();

  hier_rrg->remove_useless_nodes();
  hier_rrg->edge_type_propagate();
  hier_rrg->check_integrity();
  
  return hier_rrg;
}

shared_ptr<dfgGraph> SDFG::dfgGraph::get_hier_RRG(bool hier) const {

  // new register graph
  shared_ptr<dfgGraph> ng(new dfgGraph(name));
  
  // node to visit
  list<shared_ptr<dfgNode> > nlist = 
    get_list_of_nodes(dfgNode::SDFG_FF|dfgNode::SDFG_LATCH);

  // add node to the new graph
  BOOST_FOREACH(shared_ptr<dfgNode> nr, nlist)
    copy_a_node(ng, nr);

  // add modules
  list<shared_ptr<dfgNode> > mlist = 
    get_list_of_nodes(dfgNode::SDFG_MODULE);

  BOOST_FOREACH(shared_ptr<dfgNode> nr, mlist) {
    shared_ptr<dfgNode> nnode = copy_a_node(ng, nr);
    if(hier) {
      nnode->set_new_child(nr->child->get_hier_RRG(hier));
    }
    BOOST_FOREACH(shared_ptr<dfgNode> m, get_in_nodes(nr)) {
      if(!ng->exist(m->get_hier_name()))
        copy_a_node(ng, m);
      ng->add_edge(m->get_hier_name(), dfgEdge::SDFG_ASS, m->get_hier_name(), nr->get_hier_name());
    }
    BOOST_FOREACH(shared_ptr<dfgNode> m, get_out_nodes(nr)) {
      if(!ng->exist(m->get_hier_name())) {
        copy_a_node(ng, m);
        nlist.push_back(m);
        //assert(!(m->type & dfgNode::SDFG_MODULE));
      }
      ng->add_edge(nr->get_hier_name(), dfgEdge::SDFG_ASS, nr->get_hier_name(), m->get_hier_name());
    }
  }

  // add ports
  list<shared_ptr<dfgNode> > plist = 
    get_list_of_nodes(dfgNode::SDFG_PORT);

  BOOST_FOREACH(shared_ptr<dfgNode> nr, plist) {
    copy_a_node(ng, nr);
    BOOST_FOREACH(shared_ptr<dfgNode> m, get_in_nodes(nr)) {
      if(!ng->exist(m->get_hier_name()))
        copy_a_node(ng, m);
      ng->add_edge(m->get_hier_name(), dfgEdge::SDFG_ASS, m->get_hier_name(), nr->get_hier_name());
    }
    BOOST_FOREACH(shared_ptr<dfgNode> m, get_out_nodes(nr)) {
      if(!ng->exist(m->get_hier_name())) {
        copy_a_node(ng, m);
        nlist.push_back(m);
        //assert(!(m->type & dfgNode::SDFG_MODULE));
      }
      ng->add_edge(nr->get_hier_name(), dfgEdge::SDFG_ASS, nr->get_hier_name(), m->get_hier_name());
    }
  }

  // output paths
  while(!nlist.empty()) {
    shared_ptr<dfgNode> cn = nlist.front();
    nlist.pop_front();
    BOOST_FOREACH(shared_ptr<dfgPath> po, cn->get_out_paths_fast()) {
      if(!ng->exist(po->tar->get_hier_name())) {
        copy_a_node(ng, po->tar);
        nlist.push_back(po->tar);
        //assert(!(po->tar->type & dfgNode::SDFG_MODULE));
      }
      ng->add_edge_multi(cn->get_hier_name(), po->type, cn->get_hier_name(), po->tar->get_hier_name()); 
    }
  }

  //ng->check_integrity();
  return ng;

}

shared_ptr<dfgGraph> SDFG::dfgGraph::get_RRG() const {

  // new register graph
  shared_ptr<dfgGraph> ng(new dfgGraph(name));

  // node to visit
  list<shared_ptr<dfgNode> > nlist = get_list_of_nodes(dfgNode::SDFG_FF|dfgNode::SDFG_LATCH|dfgNode::SDFG_PORT);

  BOOST_FOREACH(shared_ptr<dfgNode> nr, nlist) 
    copy_a_node(ng, nr, true);
  
  while(!nlist.empty()) {
    shared_ptr<dfgNode> cn = nlist.front();
    nlist.pop_front();
    list<shared_ptr<dfgPath> > po = cn->get_out_paths_fast_cb();
    BOOST_FOREACH(shared_ptr<dfgPath>p, po) {
      if(!ng->exist(p->tar->get_full_name())) { // add the new node to the RRG
        copy_a_node(ng, p->tar, true);
        nlist.push_back(p->tar);
      }
      ng->add_edge_multi(cn->get_full_name(), p->type, cn->get_full_name(), p->tar->get_full_name());
    }
  }

  // return the graph
  return ng;
}

shared_ptr<dfgGraph> SDFG::dfgGraph::build_reg_graph(const std::set<shared_ptr<dfgNode> >& rlist) const {

  // new register graph
  shared_ptr<dfgGraph> ng(new dfgGraph(name));
  map<shared_ptr<dfgNode>, shared_ptr<dfgNode> > node_translate_map;

  // add all nodes to the graph
  BOOST_FOREACH(shared_ptr<dfgNode> nd, rlist)
    node_translate_map[nd] = copy_a_node(ng, nd);
  
  // connect the nodes
  BOOST_FOREACH(shared_ptr<dfgNode> nd, rlist) {
    list<shared_ptr<dfgPath> > plist = nd->get_out_paths_fast_cb();
    BOOST_FOREACH(shared_ptr<dfgPath> p, plist) {
      if(p->tar != nd && node_translate_map.count(p->tar))
        ng->add_edge(nd->get_full_name(), dfgEdge::SDFG_CTL, node_translate_map[nd], node_translate_map[p->tar]);
    }
  }
  return ng;
}

std::set<shared_ptr<dfgNode> > 
SDFG::dfgGraph::get_fsms(bool verbose, 
                         shared_ptr<dfgGraph> RRG,
                         unsigned int& num_n,
                         unsigned int& num_r,
                         unsigned int& num_pf) const {
  // find all registers who has self-loops
  list<shared_ptr<dfgNode> > nlist = get_list_of_nodes(dfgNode::SDFG_FF|dfgNode::SDFG_LATCH);
  unsigned int noden = nodes.size();   // number of nodes
  std::set<shared_ptr<dfgNode> > pfsm; // potential FSMs
  unsigned int regn = nlist.size();    // total number of registers
  unsigned int pfsmn = 0;              // total number of potential FSMs
  
  std::set<shared_ptr<dfgNode> > fakes_co, fakes_di; // for debug reasons
  while(!nlist.empty()) {
    shared_ptr<dfgNode> cn = nlist.front();
    nlist.pop_front();
    //if(verbose) std::cout << "analyse the paths of " << cn->get_hier_name() << std::endl;
    list<shared_ptr<dfgPath> > pathlist = cn->get_self_path_cb();
    BOOST_FOREACH(shared_ptr<dfgPath> p, pathlist) {
      if(p->type & (dfgEdge::SDFG_CTL_MASK|dfgEdge::SDFG_DAT_MASK)) {
        pfsm.insert(cn);
        pfsmn++;
        break;
      }
    }
  }

  // remove fake FSMs
  // must have control output to other node
  BOOST_FOREACH(shared_ptr<dfgNode> n, pfsm) {
    
    // check control output
    unsigned int etype = dfgEdge::SDFG_DF;
    BOOST_FOREACH(shared_ptr<dfgEdge> e, RRG->get_out_edges(n->get_full_name())) {
      if(RRG->get_target(e)->get_hier_name() != n->get_full_name()) 
        etype |= e->type;
    }

    if(!(etype & dfgEdge::SDFG_CTL_MASK)) {
      fakes_co.insert(n);
      continue;
    }

    etype = dfgEdge::SDFG_DF;
    BOOST_FOREACH(shared_ptr<dfgEdge> e, RRG->get_in_edges(n->get_full_name())) {
      if(RRG->get_source(e)->get_hier_name() != n->get_full_name()) 
        etype |= e->type;
    }
    
    if(etype & dfgEdge::SDFG_DAT_MASK) {
      fakes_di.insert(n);
      continue;
    }
  }
  
  // remove fake fsms
  BOOST_FOREACH(shared_ptr<dfgNode> n, fakes_co) {
    if(verbose) std::cout << n->get_full_name() << " is a fake FSM without control outputs." << std::endl;
    pfsm.erase(n);
  }
  
  BOOST_FOREACH(shared_ptr<dfgNode> n, fakes_di) {
    if(verbose) std::cout << n->get_full_name() << " is a fake FSM with data inputs." << std::endl;
    pfsm.erase(n);
  }

  num_n += noden;
  num_r += regn;
  num_pf += pfsmn;

  return pfsm;
}

map<shared_ptr<dfgNode>, int> SDFG::dfgGraph::get_fsms_new() const {
  map<shared_ptr<dfgNode>, int> rv;
  BOOST_FOREACH(nodes_type n, nodes) {
    int t = n.second->is_fsm();
    if(t)
      rv[n.second] = t;
  }
  
  return rv;
}

void SDFG::dfgGraph::fsm_simplify() {  // simplify the FSM connection graph
  std::set<shared_ptr<dfgNode> > node_set;
  std::list<shared_ptr<dfgNode> > node_list;
  typedef pair<const vertex_descriptor, shared_ptr<dfgNode> > node_record_type;
  BOOST_FOREACH(const node_record_type& n, nodes) {
    node_set.insert(n.second);
    node_list.push_back(n.second);
  }
  
  while(!node_list.empty()) {
    shared_ptr<dfgNode> node = node_list.front();
    node_list.pop_front();
    node_set.erase(node);
    shared_ptr<dfgNode> rvn = fsm_simplify_node(node);
    if(rvn && !node_set.count(rvn)) {
      node_set.insert(rvn);
      node_list.push_back(rvn);
    }
  }  
}

shared_ptr<dfgNode> SDFG::dfgGraph::fsm_simplify_node(shared_ptr<dfgNode> n) {  // simply the connection for a single FSM register
  std::set<shared_ptr<dfgNode> > rv;
  std::list<shared_ptr<dfgNode> > nlist = get_out_nodes(n);
  BOOST_FOREACH(shared_ptr<dfgNode> nxt, nlist) {
    if(exist(n, nxt) && exist(nxt, n)) {  // loop control
      rv.insert(nxt);  // rerun the next node
      
      // reconnect all input
      std::list<shared_ptr<dfgNode> > input_list = get_in_nodes(n);
      BOOST_FOREACH(shared_ptr<dfgNode> inp_node, input_list) {
        if(inp_node != nxt && !exist(inp_node, nxt))
          add_edge(inp_node->get_hier_name(), get_edge(inp_node, n)->type, inp_node, nxt);
        remove_edge(inp_node, n);
      }
      
      // reconnect all output
      BOOST_FOREACH(shared_ptr<dfgNode> outp_node, nlist) {
        if(outp_node != nxt && !exist(nxt, outp_node))
          add_edge(n->get_hier_name(), get_edge(n, outp_node)->type, nxt, outp_node);
        remove_edge(n, outp_node);
      }
      
      // connect this node to the next node
      remove_edge(n, nxt);
      remove_edge(nxt, n);
      add_edge(n->get_hier_name(), dfgEdge::SDFG_DF, n, nxt);
      return nxt;
    }
  }
  return shared_ptr<dfgNode>();
}

void SDFG::dfgGraph::annotate_toggle(shell::Env * gEnv, netlist::Module* pModule) {
  // annotate the toggle of all nodes
  BOOST_FOREACH(nodes_type n, nodes) {
    if(n.second->type == dfgNode::SDFG_MODULE) {
      if(n.second->child) {
        shared_ptr<netlist::Instance> instance = pModule->find_instance(n.second->name);
        assert(instance);
        shared_ptr<netlist::Module> module = gEnv->find_module(instance->mname);
        assert(module);
        n.second->child->annotate_toggle(gEnv, module.get());
      }
    } else {
      shared_ptr<netlist::Variable> var = pModule->find_var(n.second->name);
      if(var) {
        n.second->toggle_min = var->toggle_min.get_d() / var->toggle_duration.get_d();
        n.second->toggle_max = var->toggle_max.get_d() / var->toggle_duration.get_d();
      }
    }
  }
}

void SDFG::dfgGraph::annotate_rate() {
  // annotate the toggle of all nodes
  BOOST_FOREACH(nodes_type n, nodes) {
    if(n.second->type == dfgNode::SDFG_MODULE) {
      if(n.second->child)
        n.second->child->annotate_rate();
    } else {
      n.second->toggle_rate_min = -1.0;
      n.second->toggle_rate_max = -1.0;
      list<shared_ptr<dfgPath> > paths = n.second->get_in_paths_fast_cb();
      BOOST_FOREACH(shared_ptr<dfgPath> p, paths) {
        if(p->type & dfgEdge::SDFG_CLK) {
          n.second->toggle_rate_min = n.second->toggle_min / p->src->toggle_max;
          n.second->toggle_rate_max = n.second->toggle_max / p->src->toggle_min;
          break;
        }
      }
    }
  }
}
