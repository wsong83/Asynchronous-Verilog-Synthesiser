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

using namespace SDFG;
using boost::shared_ptr;
using std::map;
using std::string;
using std::pair;
using std::list;

shared_ptr<dfgGraph> SDFG::dfgGraph::extract_datapath(bool with_fsm, bool with_ctl) const {

  // first get a hierarchical RRG from the SDFG
  shared_ptr<dfgGraph> hier_rrg = get_hier_RRG(false);

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
      BOOST_FOREACH(shared_ptr<dfgEdge> e, n->get_in_edges()) {
        if(!(e->type & (dfgEdge::SDFG_DAT_MASK | dfgEdge::SDFG_CTL_MASK)))
          hier_rrg->remove_edge(e);
      }
      
      BOOST_FOREACH(shared_ptr<dfgEdge> e, n->get_out_edges()) {
        if(!(e->type & (dfgEdge::SDFG_DAT_MASK | dfgEdge::SDFG_CTL_MASK)))
          hier_rrg->remove_edge(e);
      }

      // remove the link to unused submodule ports
      BOOST_FOREACH(shared_ptr<dfgEdge> e, n->get_in_edges()) {
        shared_ptr<dfgNode> src = e->get_source();
        if(src->type == dfgNode::SDFG_MODULE && !src->sig2port.count(n->get_hier_name()))
          hier_rrg->remove_edge(e);
      }

      BOOST_FOREACH(shared_ptr<dfgEdge> e, n->get_out_edges()) {
        shared_ptr<dfgNode> tar = e->get_target();
        if(tar->type == dfgNode::SDFG_MODULE && !tar->sig2port.count(n->get_hier_name()))
          hier_rrg->remove_edge(e);
      }

      // get rid of the edges connected to non-existed submodule ports
      if((n->size_in_edges() == 0) || (n->size_out_edges() == 0))
        hier_rrg->remove_node(n);
    }
  }
  //hier_rrg->check_integrity();

  BOOST_FOREACH(shared_ptr<dfgNode> n, nlist) {
    if(n->type & (dfgNode::SDFG_FF | dfgNode::SDFG_LATCH) ) {
      int dp_type = 0;
      int itype = n->get_in_edges_type();
      int otype = n->get_out_edges_type();      

      if(n->is_fsm()) dp_type |= dfgNode::SDFG_DP_FSM;
      
      if((otype & dfgEdge::SDFG_DAT_MASK) || 
         ((itype & dfgEdge::SDFG_DAT_MASK) && 
          (otype & (dfgEdge::SDFG_CMP | dfgEdge::SDFG_EQU | dfgEdge::SDFG_ADR))
          )
         )
        dp_type |= dfgNode::SDFG_DP_DATA;
      
      if(otype & dfgEdge::SDFG_CTL_MASK)
        dp_type |= dfgNode::SDFG_DP_CTL;

      n->dp_type = (dfgNode::datapath_type_t)(dp_type);
    } else if(n->type == dfgNode::SDFG_DF && hier_rrg->exist(n)) {
      // remove all none data edges
      BOOST_FOREACH(shared_ptr<dfgEdge> e, n->get_in_edges()) {
        if(!(e->type & (dfgEdge::SDFG_DAT_MASK | dfgEdge::SDFG_CMP | dfgEdge::SDFG_EQU | dfgEdge::SDFG_ADR)))
          hier_rrg->remove_edge(e);
      }
      
      BOOST_FOREACH(shared_ptr<dfgEdge> e, n->get_out_edges()) {
        if(!(e->type & (dfgEdge::SDFG_DAT_MASK | dfgEdge::SDFG_CMP | dfgEdge::SDFG_EQU | dfgEdge::SDFG_ADR)))
          hier_rrg->remove_edge(e);
      }
    }
  }

  // begin the trim
  BOOST_FOREACH(shared_ptr<dfgNode> n, nlist) {
    if(n->type & (dfgNode::SDFG_FF|dfgNode::SDFG_LATCH)) {
      if(!(n->dp_type & dfgNode::SDFG_DP_DATA) && (n->dp_type & dfgNode::SDFG_DP_CTL)) {
        if(with_fsm && (n->dp_type & dfgNode::SDFG_DP_FSM)) continue;
        bool keep = false;
        if(with_ctl) {
          BOOST_FOREACH(shared_ptr<dfgNode> nn, n->get_out_nodes()) {
            if(nn->dp_type & dfgNode::SDFG_DP_DATA) { keep = true; break; }
          }
        }
        if(!keep) hier_rrg->remove_node(n);
      }
    }else if(n->type == dfgNode::SDFG_DF && hier_rrg->exist(n)) {
      if((n->size_in_edges() == 0) || (n->size_out_edges() == 0))
        hier_rrg->remove_node(n);
    }
  }
  //hier_rrg->check_integrity();
  
  // count from input
  std::set<shared_ptr<dfgNode> > nkeep;      // nodes to keep
  // list<shared_ptr<dfgNode> > nlook = hier_rrg->get_list_of_nodes(dfgNode::SDFG_IPORT); // nodes to be processed
  // std::set<shared_ptr<dfgNode> > nlook_set;
  // BOOST_FOREACH(shared_ptr<dfgNode> n, nlook)
  //   nlook_set.insert(n);

  // while(nlook.size()) {
  //   shared_ptr<dfgNode> n = nlook.front();
  //   nlook.pop_front();
    
  //   if(n->size_out_edges() > 0) {
  //     nkeep.insert(n);
  //     BOOST_FOREACH(shared_ptr<dfgNode> nn, n->get_out_nodes()) {
  //       if(!nkeep.count(nn)) {
  //         nkeep.insert(nn);
  //         if(!nlook_set.count(nn)) {
  //           nlook.push_back(nn);
  //           nlook_set.insert(nn);
  //         }
  //         BOOST_FOREACH(shared_ptr<dfgNode> nnn, nn->get_in_nodes())
  //           nkeep.insert(nnn);
  //       }
  //     }
  //   }
  // }

  list<shared_ptr<dfgNode> > nlook = hier_rrg->get_list_of_nodes(dfgNode::SDFG_OPORT);
  std::set<shared_ptr<dfgNode> > nlook_set;
  BOOST_FOREACH(shared_ptr<dfgNode> n, nlook)
    nlook_set.insert(n);
  while(nlook.size()) {
    shared_ptr<dfgNode> n = nlook.front();
    nlook.pop_front();
    
    if(n->size_in_edges() > 0) {
      nkeep.insert(n);
      BOOST_FOREACH(shared_ptr<dfgNode> nn, n->get_in_nodes()) {
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
  hier_rrg->check_integrity();
  
  return hier_rrg;
}

shared_ptr<dfgGraph> SDFG::dfgGraph::get_datapath() const {
  assert(pModule != NULL);
  if(!pModule->DataDFG) {
    
    // new register graph
    shared_ptr<dfgGraph> ng(new dfgGraph(name));
    
    // iterate all nodes for data nodes
    std::set<shared_ptr<dfgNode> > data_nodes; 
    BOOST_FOREACH(shared_ptr<dfgNode> n, 
                  get_list_of_nodes(dfgNode::SDFG_COMB|dfgNode::SDFG_FF|dfgNode::SDFG_LATCH|dfgNode::SDFG_GATE)) {
      bool keep = false;
      BOOST_FOREACH(shared_ptr<dfgEdge> e, get_in_edges(n)) {
        if(e->type & dfgEdge::SDFG_DAT_MASK) {
        keep = true;
        break;
        }
      }
      if(!keep) {
        BOOST_FOREACH(shared_ptr<dfgEdge> e, get_out_edges(n)) {
          if(e->type & dfgEdge::SDFG_DAT_MASK) {
            keep = true;
            break;
          }
        }
      }
      if(keep)
        data_nodes.insert(n);
    } 
    
    // also add module and ports to the graph
    list<shared_ptr<dfgNode> > m_list = get_list_of_nodes(dfgNode::SDFG_MODULE|dfgNode::SDFG_PORT);
    data_nodes.insert(m_list.begin(), m_list.end());
    
    // get all related nodes
    std::set<shared_ptr<dfgNode> > related_nodes;
    BOOST_FOREACH(nodes_type n, nodes) {
      if(data_nodes.count(n.second)) {
        related_nodes.insert(n.second);
        continue;
      } 
      
      bool keep = false;
      BOOST_FOREACH(shared_ptr<dfgNode> nn, get_in_nodes(n.second)) {
        if(data_nodes.count(nn)) {
          keep = true;
          break;
        }
      }
      if(!keep) {
        BOOST_FOREACH(shared_ptr<dfgNode> nn, get_out_nodes(n.second)) {
          if(data_nodes.count(nn)) {
            keep = true;
            break;
          }
        }
      }
      if(keep)
        related_nodes.insert(n.second);
    }
  
    // rebuild the graph
    BOOST_FOREACH(shared_ptr<dfgNode> n, related_nodes) {
      shared_ptr<dfgNode> nnode(n->copy());
      if(n->type & dfgNode::SDFG_MODULE) {
        nnode->set_new_child(nnode->child->get_datapath());
      }
      ng->add_node(nnode);
    }
    BOOST_FOREACH(shared_ptr<dfgNode> n, related_nodes) {
      BOOST_FOREACH(shared_ptr<dfgEdge> e, get_in_edges(n)) {
        ng->add_edge_multi(e->name, e->type, get_source(e)->get_hier_name(), n->get_hier_name());
      }
      BOOST_FOREACH(shared_ptr<dfgEdge> e, get_out_edges(n)) {
        ng->add_edge_multi(e->name, e->type, n->get_hier_name(), get_target(e)->get_hier_name());
      }    
    }
    
    // remove useless nodes
    //ng->remove_useless_nodes();
    //ng->check_integrity();
    pModule->DataDFG = ng;
  }
  
  return pModule->DataDFG;
}

shared_ptr<dfgGraph> SDFG::dfgGraph::get_hier_RRG(bool hier) const {

  // new register graph
  shared_ptr<dfgGraph> ng(new dfgGraph(name));
  
  // node to visit
  list<shared_ptr<dfgNode> > nlist = 
    get_list_of_nodes(dfgNode::SDFG_FF|dfgNode::SDFG_LATCH|dfgNode::SDFG_PORT);

  // add node to the new graph
  BOOST_FOREACH(shared_ptr<dfgNode> nr, nlist)
    copy_a_node(ng, nr);

  // add modules
  list<shared_ptr<dfgNode> > mlist = 
    get_list_of_nodes(dfgNode::SDFG_MODULE);

  BOOST_FOREACH(shared_ptr<dfgNode> nr, mlist) {
    shared_ptr<dfgNode> nnode = copy_a_node(ng, nr);
    if(hier) {
      nnode->set_new_child(nr->child->get_hier_RRG());
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
      }
      ng->add_edge(nr->get_hier_name(), dfgEdge::SDFG_ASS, nr->get_hier_name(), m->get_hier_name());
    }    
  }

  // output paths
  while(!nlist.empty()) {
    shared_ptr<dfgNode> cn = nlist.front();
    nlist.pop_front();
    if(cn->type != dfgNode::SDFG_OPORT){ // IPORT, FF and Latch
      BOOST_FOREACH(shared_ptr<dfgPath> po, cn->get_out_paths_fast()) {
        if(!ng->exist(po->tar->get_hier_name())) {
          copy_a_node(ng, po->tar);
          nlist.push_back(po->tar);
        }
        ng->add_edge_multi(cn->get_hier_name(), po->type, cn->get_hier_name(), po->tar->get_hier_name());
      }
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
      
shared_ptr<dfgNode> SDFG::dfgGraph::copy_a_node(shared_ptr<dfgGraph> G, shared_ptr<dfgNode> cn, bool use_full_name) const {
  shared_ptr<dfgNode> nnode(cn->copy());
  if(use_full_name)
    nnode->set_hier_name(cn->get_full_name());
  else
    nnode->set_hier_name(cn->get_hier_name());
  G->add_node(nnode);
  return nnode;
}
