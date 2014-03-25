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
 * the part of a SDFG node related to AVS
 * 01/10/2012   Wei Song
 *
 *
 */

#include "dfg_node.hpp"
#include "dfg_edge.hpp"
#include "dfg_path.hpp"
#include "dfg_graph.hpp"

#include "shell/env.h"

#include <boost/foreach.hpp>

using namespace SDFG;
using boost::shared_ptr;
using std::map;
using std::string;
using std::pair;
using std::list;

bool SDFG::dfgNode::is_const() {
  BOOST_FOREACH(shared_ptr<dfgPath> p, get_in_paths_fast_cb()) {
    if(p->type == dfgEdge::SDFG_CLK || p->type == dfgEdge::SDFG_RST)
      continue;                 // clock and reset paths
    else if(p->type == dfgEdge::SDFG_DDP && p->src->id == id)
      continue;                 // self default path
    else
      return false;
  }
  return true;
}

int SDFG::dfgNode::is_fsm() const {
  if(type != SDFG_FF) return SDFG_FSM_NONE;
  list<shared_ptr<dfgEdge> > in_edges = pg->get_in_edges(id);
  list<shared_ptr<dfgEdge> > out_edges = pg->get_out_edges(id);
  
  int in_path_type = 0;
  int self_loop_type = 0;
  int out_path_type = 0;
  
  BOOST_FOREACH(shared_ptr<dfgEdge> e, in_edges) {
    if(pg->get_source_id(e->id) == id)
      self_loop_type |= e->type;
    else
      in_path_type |= e->type;
  }

  BOOST_FOREACH(shared_ptr<dfgEdge> e, out_edges) {
    if(pg->get_target_id(e->id) != id)
      out_path_type |= e->type;
  }

  int fsm_type = SDFG_FSM_NONE;

  if((self_loop_type != 0) && 
     (out_path_type & dfgEdge::SDFG_CTL_MASK) &&
     !(in_path_type & dfgEdge::SDFG_DAT_MASK)
     ) { // all fsm
    fsm_type |= SDFG_FSM_OTHER;
  }

  if((self_loop_type & dfgEdge::SDFG_EQU) && 
     (out_path_type & dfgEdge::SDFG_EQU) &&
     !(in_path_type & dfgEdge::SDFG_DAT_MASK)
     ) { // fsm
    fsm_type |= SDFG_FSM_FSM;
  }

  if((self_loop_type & dfgEdge::SDFG_CAL) &&
     (out_path_type & (dfgEdge::SDFG_EQU|dfgEdge::SDFG_CMP|dfgEdge::SDFG_LOG)) &&
     !(in_path_type & dfgEdge::SDFG_DAT_MASK)
     ) { // 
    fsm_type |= SDFG_FSM_CNT;
  }

  if((self_loop_type != 0) && 
     (out_path_type & (dfgEdge::SDFG_ADR)) && 
     !(in_path_type & dfgEdge::SDFG_DAT_MASK)
     ) { // 
    fsm_type |= SDFG_FSM_ADR;
  }

  if((self_loop_type != 0) && 
     (out_path_type & dfgEdge::SDFG_LOG) &&
     !(in_path_type & dfgEdge::SDFG_DAT_MASK)
     ) { // flags
    fsm_type |= SDFG_FSM_FLAG;
  }

  //std::cout << std::hex 
  //          << ":" << self_loop_type 
  //          << ":" <<  in_path_type
  //          << ":" <<  out_path_type
  //          << std::dec << " ";

  return fsm_type;
}

string SDFG::dfgNode::get_fsm_type(int t) {
  string rv("");
  if(t & SDFG_FSM_FSM) rv += "FSM|";
  if(t & SDFG_FSM_CNT) rv += "CNT|";
  if(t & SDFG_FSM_ADR) rv += "ADR|";
  if(t & SDFG_FSM_FLAG) rv += "FLAG|";
  if(t == SDFG_FSM_OTHER) rv = "OTHER|";
  rv.erase(rv.size()-1);
  return rv;
}  

string SDFG::dfgNode::get_fsm_type() const {
  return get_fsm_type(is_fsm());
}

list<shared_ptr<dfgPath> >& SDFG::dfgNode::get_out_paths_cb() {
  if(opath.empty()) {
    // return value and the main path
    shared_ptr<dfgPath> mp(new dfgPath());       // main path
    shared_ptr<dfgNode> pn = pg->get_node(id); // this node
    
    // cache
    map<shared_ptr<dfgNode>, map<shared_ptr<dfgNode>, int > > rmap; // node relation map
    std::set<shared_ptr<dfgNode> > dnode_set;                   // dead node set to store the node do not lead to the target
    
    // initial operation
    // build up the relation map
    list<shared_ptr<dfgEdge> > oe_list = pg->get_out_edges_cb(id); // out edge list
    BOOST_FOREACH(shared_ptr<dfgEdge> e, oe_list) {
      list<shared_ptr<dfgNode> > tar_list = e->pg->get_target_cb(e);
      BOOST_FOREACH(shared_ptr<dfgNode> n, tar_list) {
        if(rmap[pn].count(n))
          rmap[pn][n] |= e->type;
        else
          rmap[pn][n] = e->type;
      }
    }
    
    // visit all out nodes
    typedef pair<const shared_ptr<dfgNode>, int> rmap_data_type;
    BOOST_FOREACH(rmap_data_type& m, rmap[pn]) {
      shared_ptr<dfgPath> p(new dfgPath(*mp));
      p->push_back(pn, m.second);
      m.first->out_path_type_update_cb(opath, p, rmap, dnode_set);
    }
  }
  return opath;
}

list<shared_ptr<dfgPath> > SDFG::dfgNode::get_out_paths_fast() {
  assert(type != SDFG_MODULE);
  typedef pair<const shared_ptr<dfgNode>, int> rmap_data_type;
  shared_ptr<dfgNode> pn = pg->get_node(id);   // this node
  std::map<boost::shared_ptr<dfgNode>, int> out_paths;
  shared_ptr<dfgPath> mp(new dfgPath());       // main path 
  
  // cache
  map<shared_ptr<dfgNode>, map<shared_ptr<dfgNode>, int > > rmap; // node relation map
  
  // initial operation
  map<shared_ptr<dfgNode>, int> tmap; // type map for next nodes
  list<shared_ptr<dfgEdge> > oe_list = pg->get_out_edges(id); // out edge list
  BOOST_FOREACH(shared_ptr<dfgEdge> e, oe_list) {
    shared_ptr<dfgNode> tar = e->pg->get_target(e);
    if(tmap.count(tar)) tmap[tar] |= e->type;
    else                tmap[tar] = e->type;
  }
  
  // visit all out nodes
  BOOST_FOREACH(rmap_data_type& m, tmap) {
    if(!(m.first->type & (SDFG_MODULE|SDFG_OPORT))) {
      shared_ptr<dfgPath> p(new dfgPath(*mp));
      p->push_back(pn, m.second);
      m.first->out_path_type_update_fast(out_paths, p, rmap);
    }
  }
  
  list<shared_ptr<dfgPath> > rv;
  BOOST_FOREACH(rmap_data_type& m, out_paths) {
    shared_ptr<dfgPath> mp(new dfgPath());
    mp->push_back(pn, m.second);
    mp->tar = m.first;
    rv.push_back(mp);
  }
  return rv;
}

list<shared_ptr<dfgPath> > SDFG::dfgNode::get_out_paths_fast_cb() {
  typedef pair<const shared_ptr<dfgNode>, int> rmap_data_type;
  shared_ptr<dfgNode> pn = pg->get_node(id);   // this node
  if(opath_f.empty()) {
    shared_ptr<dfgPath> mp(new dfgPath());       // main path 
    
    // cache
    map<shared_ptr<dfgNode>, map<shared_ptr<dfgNode>, int > > rmap; // node relation map
    
    // initial operation
    map<shared_ptr<dfgNode>, int> tmap; // type map for next nodes
    list<shared_ptr<dfgEdge> > oe_list = pg->get_out_edges_cb(id); // out edge list
    BOOST_FOREACH(shared_ptr<dfgEdge> e, oe_list) {
      list<shared_ptr<dfgNode> > tar_list = e->pg->get_target_cb(e);
      BOOST_FOREACH(shared_ptr<dfgNode> n, tar_list) {
        if(tmap.count(n))
          tmap[n] |= e->type;
        else
          tmap[n] = e->type;
      }
    }

    // visit all out nodes
    BOOST_FOREACH(rmap_data_type& m, tmap) {
      shared_ptr<dfgPath> p(new dfgPath(*mp));
      p->push_back(pn, m.second);
      m.first->out_path_type_update_fast_cb(opath_f, p, rmap);
    }
  }

  list<shared_ptr<dfgPath> > rv;
  BOOST_FOREACH(rmap_data_type& m, opath_f) {
    shared_ptr<dfgPath> mp(new dfgPath());
    mp->push_back(pn, m.second);
    mp->tar = m.first;
    rv.push_back(mp);
  }
  return rv;
}

list<shared_ptr<dfgPath> > SDFG::dfgNode::get_out_paths_fast_im() {
  typedef pair<const shared_ptr<dfgNode>, int> rmap_data_type;
  shared_ptr<dfgNode> pn = pg->get_node(id);   // this node
  shared_ptr<dfgPath> mp(new dfgPath());       // main path 
  std::map<boost::shared_ptr<dfgNode>, int> impath;  // path list

  // cache
  map<shared_ptr<dfgNode>, map<shared_ptr<dfgNode>, int > > rmap; // node relation map
  
  // initial operation
  map<shared_ptr<dfgNode>, int> tmap; // type map for next nodes
  list<shared_ptr<dfgEdge> > oe_list = pg->get_out_edges_cb(id); // out edge list
  BOOST_FOREACH(shared_ptr<dfgEdge> e, oe_list) {
    list<shared_ptr<dfgNode> > tar_list = e->pg->get_target_cb(e);
    BOOST_FOREACH(shared_ptr<dfgNode> n, tar_list) {
      if(tmap.count(n))
        tmap[n] |= e->type;
      else
        tmap[n] = e->type;
    }
  }

  // visit all out nodes
  BOOST_FOREACH(rmap_data_type& m, tmap) {
    shared_ptr<dfgPath> p(new dfgPath(*mp));
    p->push_back(pn, m.second);
    m.first->out_path_type_update_fast_im(impath, p, rmap, 0);
  }

  list<shared_ptr<dfgPath> > rv;
  BOOST_FOREACH(rmap_data_type& m, impath) {
    shared_ptr<dfgPath> mp(new dfgPath());
    mp->push_back(pn, m.second);
    mp->tar = m.first;
    rv.push_back(mp);
  }
  return rv;
}

list<shared_ptr<dfgPath> >& SDFG::dfgNode::get_in_paths_cb() {
  if(ipath.empty()) {
    // return value and the main path
    shared_ptr<dfgPath> mp(new dfgPath());       // main path
    shared_ptr<dfgNode> pn = pg->get_node(id); // this node
    mp->tar = pn;

    // cache
    map<shared_ptr<dfgNode>, map<shared_ptr<dfgNode>, int > > rmap; // node relation map
    std::set<shared_ptr<dfgNode> > dnode_set;                   // dead node set to store the node do not lead to the target

    // initial operation
    // build up the relation map
    list<shared_ptr<dfgEdge> > ie_list = pg->get_in_edges_cb(id); // in edge list
    BOOST_FOREACH(shared_ptr<dfgEdge> e, ie_list) {
      shared_ptr<dfgNode> src = e->pg->get_source_cb(e);
      if(rmap[pn].count(src))
        rmap[pn][src] |= e->type;
      else
        rmap[pn][src] = e->type;
    }

    // visit all in nodes
    typedef pair<const shared_ptr<dfgNode>, int> rmap_data_type;
    BOOST_FOREACH(rmap_data_type& m, rmap[pn]) {
      shared_ptr<dfgPath> p(new dfgPath(*mp));
      p->push_front(m.first, m.second);
      m.first->in_path_type_update_cb(ipath, p, rmap, dnode_set);
    }
  }
  return ipath;
}

list<shared_ptr<dfgPath> > SDFG::dfgNode::get_in_paths_fast_cb() {

  typedef pair<const shared_ptr<dfgNode>, int> rmap_data_type;
  shared_ptr<dfgNode> pn = pg->get_node(id);   // this node
  if(ipath_f.empty()) {
    shared_ptr<dfgPath> mp(new dfgPath());       // main path 
    mp->tar = pn;
    
    // cache
    map<shared_ptr<dfgNode>, map<shared_ptr<dfgNode>, int > > rmap; // node relation map
    
    // initial operation
    map<shared_ptr<dfgNode>, int> tmap; // type map for next nodes
    list<shared_ptr<dfgEdge> > ie_list = pg->get_in_edges_cb(id); // in edge list
    BOOST_FOREACH(shared_ptr<dfgEdge> e, ie_list) {
      shared_ptr<dfgNode> src = e->pg->get_source_cb(e);
      if(tmap.count(src))
        tmap[src] |= e->type;
      else
        tmap[src] = e->type;
    }

    // visit all in nodes
    BOOST_FOREACH(rmap_data_type& m, tmap) {
      shared_ptr<dfgPath> p(new dfgPath(*mp));
      p->push_front(m.first, m.second);
      m.first->in_path_type_update_fast_cb(ipath_f, p, rmap);
    }
  }

  list<shared_ptr<dfgPath> > rv;
  BOOST_FOREACH(rmap_data_type& m, ipath_f) {
    shared_ptr<dfgPath> mp(new dfgPath());
    mp->tar = pn;
    mp->push_front(m.first, m.second);
    rv.push_back(mp);
  }
  return rv;
}

list<shared_ptr<dfgPath> > SDFG::dfgNode::get_in_paths_fast() {

  typedef pair<const shared_ptr<dfgNode>, int> rmap_data_type;
  shared_ptr<dfgNode> pn = pg->get_node(id);   // this node
  if(ipath_f.empty()) {
    shared_ptr<dfgPath> mp(new dfgPath());       // main path 
    mp->tar = pn;
    
    // cache
    map<shared_ptr<dfgNode>, map<shared_ptr<dfgNode>, int > > rmap; // node relation map
    
    // initial operation
    map<shared_ptr<dfgNode>, int> tmap; // type map for next nodes
    list<shared_ptr<dfgEdge> > ie_list = pg->get_in_edges(id); // in edge list
    BOOST_FOREACH(shared_ptr<dfgEdge> e, ie_list) {
      shared_ptr<dfgNode> src = e->pg->get_source(e);
      if(tmap.count(src))
        tmap[src] |= e->type;
      else
        tmap[src] = e->type;
    }

    // visit all in nodes
    BOOST_FOREACH(rmap_data_type& m, tmap) {
      shared_ptr<dfgPath> p(new dfgPath(*mp));
      p->push_front(m.first, m.second);
      m.first->in_path_type_update_fast(ipath_f, p, rmap);
    }
  }

  list<shared_ptr<dfgPath> > rv;
  BOOST_FOREACH(rmap_data_type& m, ipath_f) {
    shared_ptr<dfgPath> mp(new dfgPath());
    mp->tar = pn;
    mp->push_front(m.first, m.second);
    rv.push_back(mp);
  }
  return rv;
}

list<shared_ptr<dfgPath> > SDFG::dfgNode::get_in_paths_fast_im() {
  typedef pair<const shared_ptr<dfgNode>, int> rmap_data_type;
  shared_ptr<dfgNode> pn = pg->get_node(id);   // this node
  shared_ptr<dfgPath> mp(new dfgPath());       // main path 
  mp->tar = pn;
  std::map<boost::shared_ptr<dfgNode>, int> impath;    // path list
    
  // cache
  map<shared_ptr<dfgNode>, map<shared_ptr<dfgNode>, int > > rmap; // node relation map
    
  // initial operation
  map<shared_ptr<dfgNode>, int> tmap; // type map for next nodes
  list<shared_ptr<dfgEdge> > ie_list = pg->get_in_edges_cb(id); // in edge list
  BOOST_FOREACH(shared_ptr<dfgEdge> e, ie_list) {
    shared_ptr<dfgNode> src = e->pg->get_source_cb(e);
    if(tmap.count(src))
      tmap[src] |= e->type;
    else
      tmap[src] = e->type;
  }
  
  // visit all in nodes
  BOOST_FOREACH(rmap_data_type& m, tmap) {
    shared_ptr<dfgPath> p(new dfgPath(*mp));
    p->push_front(m.first, m.second);
    m.first->in_path_type_update_fast_im(impath, p, rmap, 0);
  }
  
  list<shared_ptr<dfgPath> > rv;
  BOOST_FOREACH(rmap_data_type& m, impath) {
    shared_ptr<dfgPath> mp(new dfgPath());
    mp->push_front(m.first, m.second);
    mp->tar = pn;
    rv.push_back(mp);
  }
  return rv;
}

list<shared_ptr<dfgPath> > SDFG::dfgNode::get_self_path_cb() {
  typedef pair<const shared_ptr<dfgNode>, int> rmap_data_type;
  shared_ptr<dfgNode> pn = pg->get_node(id);   // this node
  shared_ptr<dfgPath> mp(new dfgPath());       // main path 
  if(self_f.empty()) {
    // cache
    map<shared_ptr<dfgNode>, map<shared_ptr<dfgNode>, int > > rmap; // node relation map
    
    // initial operation
    map<shared_ptr<dfgNode>, int> tmap; // type map for next nodes
    list<shared_ptr<dfgEdge> > oe_list = pg->get_out_edges_cb(id); // out edge list
    BOOST_FOREACH(shared_ptr<dfgEdge> e, oe_list) {
      list<shared_ptr<dfgNode> > tar_list = e->pg->get_target_cb(e);
      BOOST_FOREACH(shared_ptr<dfgNode> n, tar_list) {
        //if(n != pn) {  // remove self loop
          if(tmap.count(n)) tmap[n] |= e->type;
          else              tmap[n] = e->type;
        //}
      }
    }

    // visit all out nodes
    BOOST_FOREACH(rmap_data_type& m, tmap) {
      shared_ptr<dfgPath> p(new dfgPath(*mp));
      p->push_back(pn, m.second);
      m.first->self_path_update_cb(self_f, p, rmap,0);
    }
  }

  list<shared_ptr<dfgPath> > rv;
  BOOST_FOREACH(rmap_data_type& m, self_f) {
    shared_ptr<dfgPath> mp(new dfgPath());
    mp->push_back(pn, m.second);
    mp->tar = m.first;
    rv.push_back(mp);
  }
  return rv;
}

void SDFG::dfgNode::out_path_type_update_cb(list<shared_ptr<dfgPath> >& rv, // return path group
                                         shared_ptr<dfgPath>& cp, // current path
                                         map<shared_ptr<dfgNode>, map<shared_ptr<dfgNode>, int > >& rmap,
                                         std::set<shared_ptr<dfgNode> >& dnode_set) {
  // this node
  shared_ptr<dfgNode> pn = pg->get_node(id);
  
  // check node type
  if((type & (SDFG_FF|SDFG_LATCH))         || // register
     pg->size_out_edges_cb(id) == 0           // top-level output
     ) {  // ending point
    cp->tar = pn;
    cp->node_set.clear();
    rv.push_back(cp);
    //std::cout << "    " << pn->get_hier_name()  << " : " << rv.size() << ":" << cp->path.size() << std::endl;
    return;
  }
  
  // no loop assert
  if(cp->node_set.count(pn)) {
    cp->tar = pn;
    G_ENV->error("SDFG-ANALYSE-0", toString(*cp));
    return;
  }
  
  // check whether it is dead
  if(dnode_set.count(pn)) return;

  // expand it
  if(!rmap.count(pn)) {         // new node
    list<shared_ptr<dfgEdge> > oe_list = pg->get_out_edges_cb(id); // out edge list
    BOOST_FOREACH(shared_ptr<dfgEdge> e, oe_list) {
      list<shared_ptr<dfgNode> > tar_list = e->pg->get_target_cb(e);
      BOOST_FOREACH(shared_ptr<dfgNode> n, tar_list) {
        if(rmap[pn].count(n))
          rmap[pn][n] |= e->type;
        else
          rmap[pn][n] = e->type;
      }
    }
  }

  typedef pair<const shared_ptr<dfgNode>, int> rmap_data_type;
  unsigned int rv_size = rv.size();
  BOOST_FOREACH(rmap_data_type& m, rmap[pn]) {
    shared_ptr<dfgPath> p(new dfgPath(*cp));
    p->push_back(pn, m.second);
    m.first->out_path_type_update_cb(rv, p, rmap, dnode_set);
  }

  if(rv.size() == rv_size) {         // this is a dead node
    dnode_set.insert(pn);
    rmap.erase(pn);
  }

}

void SDFG::dfgNode::out_path_type_update_fast(map<shared_ptr<dfgNode>, int>& rv, // return path group
                                              shared_ptr<dfgPath>& cp, // current path
                                              map<shared_ptr<dfgNode>, map<shared_ptr<dfgNode>, int > >& rmap) {
  
  // this node
  shared_ptr<dfgNode> pn = pg->get_node(id);
  assert(!(pn->type & SDFG_MODULE));
  //std::cout << get_full_name() << std::endl;
  
  // check node type
  if((type & (SDFG_FF|SDFG_LATCH))         || // register
     pg->size_out_edges(id) == 0              // no load
     ) {  // ending point
    if(rv.count(pn)) rv[pn] |= cp->type;
    else             rv[pn] = cp->type;
    if(cp->get_2nd_back() != pn) {
      if(rmap[cp->get_2nd_back()].count(pn))
        rmap[cp->get_2nd_back()][pn] |= cp->path.back().second;
      else
        rmap[cp->get_2nd_back()][pn] = cp->path.back().second;
    }
    return;
  }

  // no loop assert
  if(cp->node_set.count(pn)) {
    cp->tar = pn;
    G_ENV->error("SDFG-ANALYSE-0", toString(*cp));
    return;
  }

  typedef pair<const shared_ptr<dfgNode>, int> rmap_data_type;
  if(rmap.count(pn)) {          // visited
    BOOST_FOREACH(rmap_data_type& t, rmap[pn]) {
      if(t.first) {
        shared_ptr<dfgPath> p(new dfgPath(*cp));
        p->push_back(t.first, t.second);
        rv[t.first] |= p->type;
      } else {
        rv[pn] |= cp->type;
      }
    }
  } else {         // new node
    map<shared_ptr<dfgNode>, int> tmap; // type map for next nodes
    list<shared_ptr<dfgEdge> > oe_list = pg->get_out_edges(id); // out edge list
    BOOST_FOREACH(shared_ptr<dfgEdge> e, oe_list) {
      shared_ptr<dfgNode> tar = e->pg->get_target(e);
      if(tmap.count(tar)) tmap[tar] |= e->type;
      else                tmap[tar] = e->type;
    }
    
    // visit all out nodes
    BOOST_FOREACH(rmap_data_type& t, tmap) {
      if(t.first->type & (SDFG_MODULE|SDFG_OPORT)) {  // module or output port
        if(pn != cp->src) {
          if(rv.count(pn)) rv[pn] |= cp->type;
          else             rv[pn] = cp->type;
        }
      } else {
        shared_ptr<dfgPath> p(new dfgPath(*cp));
        p->push_back(pn, t.second);
        t.first->out_path_type_update_fast(rv, p, rmap);
        // update rmap
        if(t.first != p->src) {
          BOOST_FOREACH(rmap_data_type& m, rmap[t.first]) {
            if(m.first) {
              if(rmap[pn].count(m.first)) rmap[pn][m.first] |= dfgPath::cal_type(t.second, m.second);
              else                        rmap[pn][m.first] = dfgPath::cal_type(t.second, m.second);
            } else {
              if(rmap[pn].count(t.first)) rmap[pn][t.first] |= t.second;
              else                        rmap[pn][t.first] = t.second;
            }
          }
        }
      }
    }
  }
}

void SDFG::dfgNode::out_path_type_update_fast_cb(map<shared_ptr<dfgNode>, int>& rv, // return path group
                                              shared_ptr<dfgPath>& cp, // current path
                                              map<shared_ptr<dfgNode>, map<shared_ptr<dfgNode>, int > >& rmap) {
  
  // this node
  shared_ptr<dfgNode> pn = pg->get_node(id);
  //std::cout << get_full_name() << std::endl;
  
  // check node type
  if((type & (SDFG_FF|SDFG_LATCH))         || // register
     pg->size_out_edges_cb(id) == 0           // top-level output
     ) {  // ending point
    if(rv.count(pn)) rv[pn] |= cp->type;
    else             rv[pn] = cp->type;
    if(cp->get_2nd_back() != pn) {
      if(rmap[cp->get_2nd_back()].count(pn))
        rmap[cp->get_2nd_back()][pn] |= cp->path.back().second;
      else
        rmap[cp->get_2nd_back()][pn] = cp->path.back().second;
    }
    //std::cout << "    " << pn->get_hier_name()  << " : " << rv.size() << ":" << cp->path.size() << std::endl;
    return;
  }

  // no loop assert
  if(cp->node_set.count(pn)) {
    cp->tar = pn;
    G_ENV->error("SDFG-ANALYSE-0", toString(*cp));
    return;
  }

  typedef pair<const shared_ptr<dfgNode>, int> rmap_data_type;
  if(rmap.count(pn)) {          // visited
    BOOST_FOREACH(rmap_data_type& t, rmap[pn]) {
      shared_ptr<dfgPath> p(new dfgPath(*cp));
      p->push_back(t.first, t.second);
      rv[t.first] |= p->type;
    }
  } else {         // new node
    map<shared_ptr<dfgNode>, int> tmap; // type map for next nodes
    list<shared_ptr<dfgEdge> > oe_list = pg->get_out_edges_cb(id); // out edge list
    BOOST_FOREACH(shared_ptr<dfgEdge> e, oe_list) {
      list<shared_ptr<dfgNode> > tar_list = e->pg->get_target_cb(e);
      BOOST_FOREACH(shared_ptr<dfgNode> n, tar_list) {
        if(tmap.count(n))
          tmap[n] |= e->type;
        else
          tmap[n] = e->type;
      }
    }
    
    // visit all out nodes
    BOOST_FOREACH(rmap_data_type& t, tmap) {
      shared_ptr<dfgPath> p(new dfgPath(*cp));
      p->push_back(pn, t.second);
      t.first->out_path_type_update_fast_cb(rv, p, rmap);
      if(t.first != p->src) {
        // update rmap
        BOOST_FOREACH(rmap_data_type& m, rmap[t.first]) {
          if(rmap[pn].count(m.first)) rmap[pn][m.first] |= dfgPath::cal_type(t.second, m.second);
          else                        rmap[pn][m.first] = dfgPath::cal_type(t.second, m.second);
        }
      }
    }
  }
}

void SDFG::dfgNode::out_path_type_update_fast_im(map<shared_ptr<dfgNode>, int>& rv, // return path group
                                                 shared_ptr<dfgPath>& cp, // current path
                                                 map<shared_ptr<dfgNode>, map<shared_ptr<dfgNode>, int > >& rmap,
                                                 unsigned int level) {
  
  // this node
  shared_ptr<dfgNode> pn = pg->get_node(id);
  //std::cout << get_full_name() << std::endl;
  
  // check node type
  if((type & (SDFG_FF|SDFG_LATCH))         || // register
     (type & SDFG_PORT && level==0)        || // top-level output
     pg->size_out_edges_cb(id) == 0           // top-level output
     ) {  // ending point
    if(rv.count(pn)) rv[pn] |= cp->type;
    else             rv[pn] = cp->type;
    if(cp->get_2nd_back() != pn) {
      if(rmap[cp->get_2nd_back()].count(pn))
        rmap[cp->get_2nd_back()][pn] |= cp->path.back().second;
      else
        rmap[cp->get_2nd_back()][pn] = cp->path.back().second;
    }
    //std::cout << "    " << pn->get_hier_name()  << " : " << rv.size() << ":" << cp->path.size() << std::endl;
    return;
  }

  // no loop assert
  if(cp->node_set.count(pn)) {
    cp->tar = pn;
    G_ENV->error("SDFG-ANALYSE-0", toString(*cp));
    return;
  }

  typedef pair<const shared_ptr<dfgNode>, int> rmap_data_type;
  if(rmap.count(pn)) {          // visited
    BOOST_FOREACH(rmap_data_type& t, rmap[pn]) {
      shared_ptr<dfgPath> p(new dfgPath(*cp));
      p->push_back(t.first, t.second);
      rv[t.first] |= p->type;
    }
  } else {         // new node
    map<shared_ptr<dfgNode>, int> tmap; // type map for next nodes
    list<shared_ptr<dfgEdge> > oe_list = pg->get_out_edges_cb(id); // out edge list
    BOOST_FOREACH(shared_ptr<dfgEdge> e, oe_list) {
      list<shared_ptr<dfgNode> > tar_list = e->pg->get_target_cb(e);
      BOOST_FOREACH(shared_ptr<dfgNode> n, tar_list) {
        if(tmap.count(n))
          tmap[n] |= e->type;
        else
          tmap[n] = e->type;
      }
    }
    
    // visit all out nodes
    BOOST_FOREACH(rmap_data_type& t, tmap) {
      shared_ptr<dfgPath> p(new dfgPath(*cp));
      p->push_back(pn, t.second);
      if(pn->pg->exist(pn, t.first))
        t.first->out_path_type_update_fast_im(rv, p, rmap, level);
      else if(pn->type & SDFG_PORT)
        t.first->out_path_type_update_fast_im(rv, p, rmap, level-1);
      else
        t.first->out_path_type_update_fast_im(rv, p, rmap, level+1);

      // update rmap
      BOOST_FOREACH(rmap_data_type& m, rmap[t.first]) {
        if(rmap[pn].count(m.first)) rmap[pn][m.first] |= dfgPath::cal_type(t.second, m.second);
        else                        rmap[pn][m.first] = dfgPath::cal_type(t.second, m.second);
      }
    }
  }
}

void SDFG::dfgNode::in_path_type_update_cb(list<shared_ptr<dfgPath> >& rv, // return path group
                                        shared_ptr<dfgPath>& cp, // current path
                                        map<shared_ptr<dfgNode>, map<shared_ptr<dfgNode>, int > >& rmap,
                                        std::set<shared_ptr<dfgNode> >& dnode_set) {
  // this node
  shared_ptr<dfgNode> pn = pg->get_node(id);

  // check node type
  if((type & (SDFG_FF|SDFG_LATCH))         || // register
     pg->size_in_edges_cb(id) == 0            // top-level input
     ) {  // ending point
    cp->src = pn;
    cp->node_set.clear();
    rv.push_back(cp);
    return;
  }

  // check whether it is dead
  if(dnode_set.count(pn)) return;

  // expand it
  if(!rmap.count(pn)) {         // new node
    list<shared_ptr<dfgEdge> > ie_list = pg->get_in_edges_cb(id); // out edge list
    BOOST_FOREACH(shared_ptr<dfgEdge> e, ie_list) {
      shared_ptr<dfgNode> src = e->pg->get_source_cb(e);
      if(rmap[pn].count(src))
        rmap[pn][src] |= e->type;
      else
        rmap[pn][src] = e->type;
    }
  }

  unsigned int rv_size = rv.size();
  typedef pair<const shared_ptr<dfgNode>, int> rmap_data_type;
  BOOST_FOREACH(rmap_data_type& m, rmap[pn]) {
    // no loop assert
    if(cp->node_set.count(m.first)) {
      G_ENV->error("SDFG-ANALYSE-0", toString(*cp));
      continue;
    } else {
      shared_ptr<dfgPath> p(new dfgPath(*cp));
      p->push_front(m.first, m.second);
      m.first->in_path_type_update_cb(rv, p, rmap, dnode_set);
    }
  }

  if(rv.size() == rv_size) {         // this is a dead node
    dnode_set.insert(pn);
    rmap.erase(pn);
  }
}

void SDFG::dfgNode::in_path_type_update_fast_cb(map<shared_ptr<dfgNode>, int>& rv, // return path group
                                              shared_ptr<dfgPath>& cp, // current path
                                              map<shared_ptr<dfgNode>, map<shared_ptr<dfgNode>, int > >& rmap) {
  
  // this node
  shared_ptr<dfgNode> pn = pg->get_node(id);
  //std::cout << *cp << std::endl;
  
  // check node type
  if((type & (SDFG_FF|SDFG_LATCH))         || // register
     pg->size_in_edges_cb(id) == 0            // top-level input
     ) {  // ending point
    if(rv.count(pn)) rv[pn] |= cp->type;
    else             rv[pn] = cp->type;
    if(cp->get_2nd_front() != pn) {
      if(rmap[cp->get_2nd_front()].count(pn))
        rmap[cp->get_2nd_front()][pn] |= cp->path.front().second;
      else
        rmap[cp->get_2nd_front()][pn] = cp->path.front().second;
    }
    //std::cout << "    " << pn->get_hier_name()  << " : " << rv.size() << ":" << cp->path.size() << std::endl;
    return;
  }

  typedef pair<const shared_ptr<dfgNode>, int> rmap_data_type;
  if(rmap.count(pn)) {          // visited
    BOOST_FOREACH(rmap_data_type& t, rmap[pn]) {
      shared_ptr<dfgPath> p(new dfgPath(*cp));
      p->push_front(t.first, t.second);
      rv[t.first] |= p->type;
    }
  } else {         // new node
    map<shared_ptr<dfgNode>, int> tmap; // type map for next nodes
    list<shared_ptr<dfgEdge> > ie_list = pg->get_in_edges_cb(id); // out edge list
    BOOST_FOREACH(shared_ptr<dfgEdge> e, ie_list) {
      shared_ptr<dfgNode> src = e->pg->get_source_cb(e);
      if(tmap.count(src))
        tmap[src] |= e->type;
      else
        tmap[src] = e->type;
    }
    
    // visit all in nodes
    BOOST_FOREACH(rmap_data_type& t, tmap) {
      if(cp->node_set.count(t.first)) {
        G_ENV->error("SDFG-ANALYSE-0", toString(*cp));
        continue;
      } else {
        shared_ptr<dfgPath> p(new dfgPath(*cp));
        p->push_front(t.first, t.second);
        t.first->in_path_type_update_fast_cb(rv, p, rmap);
        // update rmap
        BOOST_FOREACH(rmap_data_type& m, rmap[t.first]) {
          if(rmap[pn].count(m.first)) rmap[pn][m.first] |= dfgPath::cal_type(m.second, t.second);
          else                        rmap[pn][m.first] = dfgPath::cal_type(m.second, t.second);
        }
      }
    }
  }
}

void SDFG::dfgNode::in_path_type_update_fast(map<shared_ptr<dfgNode>, int>& rv, // return path group
                                             shared_ptr<dfgPath>& cp, // current path
                                             map<shared_ptr<dfgNode>, map<shared_ptr<dfgNode>, int > >& rmap) {
  
  // this node
  shared_ptr<dfgNode> pn = pg->get_node(id);
  //std::cout << *cp << std::endl;
  
  // check node type
  if((type & (SDFG_FF|SDFG_LATCH))         || // register
     pg->size_in_edges(id) == 0            // top-level input
     ) {  // ending point
    if(rv.count(pn)) rv[pn] |= cp->type;
    else             rv[pn] = cp->type;
    if(cp->get_2nd_front() != pn) {
      if(rmap[cp->get_2nd_front()].count(pn))
        rmap[cp->get_2nd_front()][pn] |= cp->path.front().second;
      else
        rmap[cp->get_2nd_front()][pn] = cp->path.front().second;
    }
    //std::cout << "    " << pn->get_hier_name()  << " : " << rv.size() << ":" << cp->path.size() << std::endl;
    return;
  }

  typedef pair<const shared_ptr<dfgNode>, int> rmap_data_type;
  if(rmap.count(pn)) {          // visited
    BOOST_FOREACH(rmap_data_type& t, rmap[pn]) {
      shared_ptr<dfgPath> p(new dfgPath(*cp));
      p->push_front(t.first, t.second);
      rv[t.first] |= p->type;
    }
  } else {         // new node
    map<shared_ptr<dfgNode>, int> tmap; // type map for next nodes
    list<shared_ptr<dfgEdge> > ie_list = pg->get_in_edges(id); // out edge list
    BOOST_FOREACH(shared_ptr<dfgEdge> e, ie_list) {
      shared_ptr<dfgNode> src = e->pg->get_source(e);
      if(tmap.count(src))
        tmap[src] |= e->type;
      else
        tmap[src] = e->type;
    }
    
    // visit all in nodes
    BOOST_FOREACH(rmap_data_type& t, tmap) {
      if(cp->node_set.count(t.first)) {
        G_ENV->error("SDFG-ANALYSE-0", toString(*cp));
        continue;
      } else {
        shared_ptr<dfgPath> p(new dfgPath(*cp));
        p->push_front(t.first, t.second);
        t.first->in_path_type_update_fast(rv, p, rmap);
        // update rmap
        BOOST_FOREACH(rmap_data_type& m, rmap[t.first]) {
          if(rmap[pn].count(m.first)) rmap[pn][m.first] |= dfgPath::cal_type(m.second, t.second);
          else                        rmap[pn][m.first] = dfgPath::cal_type(m.second, t.second);
        }
      }
    }
  }
}

void SDFG::dfgNode::in_path_type_update_fast_im(map<shared_ptr<dfgNode>, int>& rv, // return path group
                                                shared_ptr<dfgPath>& cp, // current path
                                                map<shared_ptr<dfgNode>, map<shared_ptr<dfgNode>, int > >& rmap,
                                                unsigned int level) {
  
  // this node
  shared_ptr<dfgNode> pn = pg->get_node(id);
  //std::cout << *cp << std::endl;
  
  // check node type
  if((type & (SDFG_FF|SDFG_LATCH))         || // register
     (type & SDFG_PORT && level==0)        || // top-level input
     pg->size_in_edges_cb(id) == 0 
     ) {  // ending point
    if(rv.count(pn)) rv[pn] |= cp->type;
    else             rv[pn] = cp->type;
    if(cp->get_2nd_front() != pn) {
      if(rmap[cp->get_2nd_front()].count(pn))
        rmap[cp->get_2nd_front()][pn] |= cp->path.front().second;
      else
        rmap[cp->get_2nd_front()][pn] = cp->path.front().second;
    }
    //std::cout << "    " << pn->get_hier_name()  << " : " << rv.size() << ":" << cp->path.size() << std::endl;
    return;
  }

  typedef pair<const shared_ptr<dfgNode>, int> rmap_data_type;
  if(rmap.count(pn)) {          // visited
    BOOST_FOREACH(rmap_data_type& t, rmap[pn]) {
      shared_ptr<dfgPath> p(new dfgPath(*cp));
      p->push_front(t.first, t.second);
      rv[t.first] |= p->type;
    }
  } else {         // new node
    map<shared_ptr<dfgNode>, int> tmap; // type map for next nodes
    list<shared_ptr<dfgEdge> > ie_list = pg->get_in_edges_cb(id); // out edge list
    BOOST_FOREACH(shared_ptr<dfgEdge> e, ie_list) {
      shared_ptr<dfgNode> src = e->pg->get_source_cb(e);
      if(tmap.count(src))
        tmap[src] |= e->type;
      else
        tmap[src] = e->type;
    }
    
    // visit all in nodes
    BOOST_FOREACH(rmap_data_type& t, tmap) {
      if(cp->node_set.count(t.first)) {
        G_ENV->error("SDFG-ANALYSE-0", toString(*cp));
        continue;
      } else {
        shared_ptr<dfgPath> p(new dfgPath(*cp));
        p->push_front(t.first, t.second);
        if(pn->pg->exist(t.first, pn))
          t.first->in_path_type_update_fast_im(rv, p, rmap, level);
        else if(t.first->type & SDFG_PORT)
          t.first->in_path_type_update_fast_im(rv, p, rmap, level-1);
        else
          t.first->in_path_type_update_fast_im(rv, p, rmap, level+1);

        // update rmap
        BOOST_FOREACH(rmap_data_type& m, rmap[t.first]) {
          if(rmap[pn].count(m.first)) rmap[pn][m.first] |= dfgPath::cal_type(m.second, t.second);
          else                        rmap[pn][m.first] = dfgPath::cal_type(m.second, t.second);
        }
      }
    }
  }
}

void SDFG::dfgNode::self_path_update_cb(map<shared_ptr<dfgNode>, int>& rv, // return path group
				     shared_ptr<dfgPath>& cp, // current path
				     map<shared_ptr<dfgNode>, map<shared_ptr<dfgNode>, int > >& rmap,
				     unsigned int level) {
  
  // this node
  shared_ptr<dfgNode> pn = pg->get_node(id);
  
  // check node type
  if((pn->type & (SDFG_FF|SDFG_LATCH))         || // register
     (pn->type & SDFG_PORT && level==0)        || // top-level output
     pg->size_out_edges_cb(id) == 0               // top-level output
     ) {  // ending point
    if(pn == cp->src) {
      if(rv.count(pn)) rv[pn] |= cp->type;        // save it into rv
      else             rv[pn] = cp->type;
      if(cp->get_2nd_back() != pn) {
        if(rmap[cp->get_2nd_back()].count(pn))
          rmap[cp->get_2nd_back()][pn] |= cp->path.back().second;
        else
          rmap[cp->get_2nd_back()][pn] = cp->path.back().second;
      }
    }
    return;
  }

  // no loop assert
  if(cp->node_set.count(pn)) {
    cp->tar = pn;
    G_ENV->error("SDFG-ANALYSE-0", toString(*cp));
    return;
  }

  typedef pair<const shared_ptr<dfgNode>, int> rmap_data_type;
  if(rmap.count(pn)) {          // visited
    BOOST_FOREACH(rmap_data_type& t, rmap[pn]) {
      shared_ptr<dfgPath> p(new dfgPath(*cp));
      p->push_back(t.first, t.second);
      rv[t.first] |= p->type;
    }
  } else {         // new node
    map<shared_ptr<dfgNode>, int> tmap; // type map for next nodes
    list<shared_ptr<dfgEdge> > oe_list = pg->get_out_edges_cb(id); // out edge list
    BOOST_FOREACH(shared_ptr<dfgEdge> e, oe_list) {
      list<shared_ptr<dfgNode> > tar_list = e->pg->get_target_cb(e);
      BOOST_FOREACH(shared_ptr<dfgNode> n, tar_list) {
        if(tmap.count(n))
          tmap[n] |= e->type;
        else
          tmap[n] = e->type;
      }
    }
    
    // visit all out nodes
    BOOST_FOREACH(rmap_data_type& t, tmap) {
      shared_ptr<dfgPath> p(new dfgPath(*cp));
      p->push_back(pn, t.second);
      if(pn->pg->exist(pn, t.first))
        t.first->self_path_update_cb(rv, p, rmap, level);
      else if(pn->type & SDFG_PORT)
        t.first->self_path_update_cb(rv, p, rmap, level-1);
      else
        t.first->self_path_update_cb(rv, p, rmap, level+1);

      // update rmap
      BOOST_FOREACH(rmap_data_type& m, rmap[t.first]) {
        if(rmap[pn].count(m.first)) rmap[pn][m.first] |= dfgPath::cal_type(t.second, m.second);
        else                        rmap[pn][m.first] = dfgPath::cal_type(t.second, m.second);
      }
    }
  }
}
