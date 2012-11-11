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
 * the part of SDFG related to AVS
 * 01/10/2012   Wei Song
 *
 *
 */


#include "sdfg.hpp"
#include <boost/tuple/tuple.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <set>
#include <iostream>

#include "shell/env.h"

using namespace SDFG;
using boost::shared_ptr;
using std::map;
using std::string;
using std::pair;
using std::list;

/////////////////////////////////////////////////////////////////////////////
/********        Node                                               ********/
/////////////////////////////////////////////////////////////////////////////

list<shared_ptr<dfgPath> >& SDFG::dfgNode::get_out_paths() {
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
      list<shared_ptr<dfgNode> > tar_list = e->pg->get_target_cb(e->id);
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
      m.first->out_path_type_update(opath, p, rmap, dnode_set);
    }
  }
  return opath;
}

list<shared_ptr<dfgPath> > SDFG::dfgNode::get_out_paths_fast() {
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
      list<shared_ptr<dfgNode> > tar_list = e->pg->get_target_cb(e->id);
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
      m.first->out_path_type_update_fast(opath_f, p, rmap);
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

list<shared_ptr<dfgPath> >& SDFG::dfgNode::get_in_paths() {
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
      shared_ptr<dfgNode> src = e->pg->get_source_cb(e->id);
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
      p->tar = pn;
      m.first->in_path_type_update(ipath, p, rmap, dnode_set);
    }
  }
  return ipath;
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
    list<shared_ptr<dfgEdge> > ie_list = pg->get_in_edges_cb(id); // in edge list
    BOOST_FOREACH(shared_ptr<dfgEdge> e, ie_list) {
      shared_ptr<dfgNode> src = e->pg->get_source_cb(e->id);
      if(tmap.count(src))
        tmap[src] |= e->type;
      else
        tmap[src] = e->type;
    }

    // visit all in nodes
    BOOST_FOREACH(rmap_data_type& m, tmap) {
      shared_ptr<dfgPath> p(new dfgPath(*mp));
      p->push_front(pn, m.second);
      m.first->in_path_type_update_fast(ipath_f, p, rmap);
    }
  }

  list<shared_ptr<dfgPath> > rv;
  BOOST_FOREACH(rmap_data_type& m, ipath_f) {
    shared_ptr<dfgPath> mp(new dfgPath());
    mp->push_back(m.first, m.second);
    mp->tar = pn;
    rv.push_back(mp);
  }
  return rv;
}

void SDFG::dfgNode::out_path_type_update(list<shared_ptr<dfgPath> >& rv, // return path group
                                         shared_ptr<dfgPath>& cp, // current path
                                         map<shared_ptr<dfgNode>, map<shared_ptr<dfgNode>, int > >& rmap,
                                         std::set<shared_ptr<dfgNode> >& dnode_set) {
  // this node
  shared_ptr<dfgNode> pn = pg->get_node(id);
  
  // check node type
  if((pn->type & (SDFG_FF|SDFG_LATCH))         || // register
     (pn->type & SDFG_PORT && !pn->pg->father)    // top-level output
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
      list<shared_ptr<dfgNode> > tar_list = e->pg->get_target_cb(e->id);
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
    m.first->out_path_type_update(rv, p, rmap, dnode_set);
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
  //std::cout << get_full_name() << std::endl;
  
  // check node type
  if((pn->type & (SDFG_FF|SDFG_LATCH))         || // register
     (pn->type & SDFG_PORT && !pn->pg->father)    // top-level output
     ) {  // ending point
    if(rv.count(pn)) rv[pn] |= cp->type;
    else             rv[pn] = cp->type;
    rmap[cp->path.back().first][pn] = cp->path.back().second;
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
      list<shared_ptr<dfgNode> > tar_list = e->pg->get_target_cb(e->id);
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
      t.first->out_path_type_update_fast(rv, p, rmap);
      // update rmap
      BOOST_FOREACH(rmap_data_type& m, rmap[t.first]) {
        int new_type = 0;
        if(t.second & m.second & dfgEdge::SDFG_DP)
          new_type = t.second | m.second;
        else
          new_type = m.second;
        if(rmap[pn].count(m.first)) rmap[pn][m.first] = new_type;
        else                        rmap[pn][m.first] |= new_type;
      }
    }
  }
}

void SDFG::dfgNode::in_path_type_update(list<shared_ptr<dfgPath> >& rv, // return path group
                                        shared_ptr<dfgPath>& cp, // current path
                                        map<shared_ptr<dfgNode>, map<shared_ptr<dfgNode>, int > >& rmap,
                                        std::set<shared_ptr<dfgNode> >& dnode_set) {
  // this node
  shared_ptr<dfgNode> pn = pg->get_node(id);

  // check node type
  if((pn->type & (SDFG_FF|SDFG_LATCH))         || // register
     (pn->type & SDFG_PORT && !pn->pg->father)    // top-level input
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
      shared_ptr<dfgNode> src = e->pg->get_source_cb(e->id);
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
      m.first->in_path_type_update(rv, p, rmap, dnode_set);
    }
  }

  if(rv.size() == rv_size) {         // this is a dead node
    dnode_set.insert(pn);
    rmap.erase(pn);
  }
}

void SDFG::dfgNode::in_path_type_update_fast(map<shared_ptr<dfgNode>, int>& rv, // return path group
                                              shared_ptr<dfgPath>& cp, // current path
                                              map<shared_ptr<dfgNode>, map<shared_ptr<dfgNode>, int > >& rmap) {
  
  // this node
  shared_ptr<dfgNode> pn = pg->get_node(id);
  //std::cout << get_full_name() << std::endl;
  
  // check node type
  if((pn->type & (SDFG_FF|SDFG_LATCH))         || // register
     (pn->type & SDFG_PORT && !pn->pg->father)    // top-level input
     ) {  // ending point
    if(rv.count(pn)) rv[pn] |= cp->type;
    else             rv[pn] = cp->type;
    rmap[cp->path.back().first][pn] = cp->path.back().second;
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
      shared_ptr<dfgNode> src = e->pg->get_source_cb(e->id);
      if(tmap.count(src))
        tmap[src] |= e->type;
      else
        tmap[src] = e->type;
    }
    
    // visit all out nodes
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
          int new_type = 0;
          if(t.second & m.second & dfgEdge::SDFG_DP)
            new_type = t.second | m.second;
          else if(t.second == 0)
            new_type = m.second;
          if(rmap[pn].count(m.first)) rmap[pn][m.first] = new_type;
          else                        rmap[pn][m.first] |= new_type;
        }
      }
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
/********        Edge                                               ********/
/////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////
/********        Graph                                              ********/
/////////////////////////////////////////////////////////////////////////////


///////////////////////////////
// analyse functions
///////////////////////////////

shared_ptr<dfgGraph> SDFG::dfgGraph::get_reg_graph() const {

  // new register graph
  shared_ptr<dfgGraph> ng(new dfgGraph(name));

  // node to visit
  list<shared_ptr<dfgNode> > node_next;

  // find out all ports and put it in the new graph
  for_each(nodes.begin(), nodes.end(),
           [&](const pair<const vertex_descriptor, shared_ptr<dfgNode> >& m) {
             if(m.second->type & dfgNode::SDFG_PORT) {
               node_next.push_back(m.second);
               shared_ptr<dfgNode> nnode(m.second->copy());
               nnode->set_hier_name(m.second->get_full_name());
               ng->add_node(nnode);
             }
           });
  
  // buld a new graph
  while(!node_next.empty()) {
    // fetch current node
    shared_ptr<dfgNode> cnode = node_next.front();
    node_next.pop_front();

    // get the paths
    list<shared_ptr<dfgPath> >& plist = cnode->get_out_paths();
    BOOST_FOREACH(shared_ptr<dfgPath> p, plist) {
      if(p->type) {
        // add new node if it is new
        if(!ng->exist(p->tar->get_full_name())) { // new node
          node_next.push_back(p->tar);
          shared_ptr<dfgNode> nnode(p->tar->copy());
          nnode->set_hier_name(p->tar->get_full_name());
          ng->add_node(nnode);
        }
        
        // add path
        if((p->type & dfgEdge::SDFG_DP) == dfgEdge::SDFG_DP)
          ng->add_edge(cnode->get_full_name(), dfgEdge::SDFG_DP, cnode->get_full_name(), p->tar->get_full_name());
        else if(p->type & dfgEdge::SDFG_DF)
          ng->add_edge(cnode->get_full_name(), dfgEdge::SDFG_DF, cnode->get_full_name(), p->tar->get_full_name());

        if((p->type & dfgEdge::SDFG_RST) == dfgEdge::SDFG_RST)
          ng->add_edge(cnode->get_full_name(), dfgEdge::SDFG_RST, cnode->get_full_name(), p->tar->get_full_name());
        else if((p->type & dfgEdge::SDFG_CLK) == dfgEdge::SDFG_CLK)
          ng->add_edge(cnode->get_full_name(), dfgEdge::SDFG_CLK, cnode->get_full_name(), p->tar->get_full_name());
        else if(p->type & dfgEdge::SDFG_CTL)
          ng->add_edge(cnode->get_full_name(), dfgEdge::SDFG_CTL, cnode->get_full_name(), p->tar->get_full_name());
      }
    }
    
  }

  // return the graph
  return ng;
}

list<list<shared_ptr<dfgNode> > > SDFG::dfgGraph::get_fsm_groups(bool verbose) const {
  // find all registers who has self-loops
  list<shared_ptr<dfgNode> > nlist;
  std::set<shared_ptr<dfgNode> > pfsm; // potential FSMs
  unsigned int noden = 0;              // total number of nodes 
  unsigned int regn = 0;               // total number of registers
  unsigned int pfsmn = 0;              // total number of potential FSMs
  typedef pair<const vertex_descriptor, shared_ptr<dfgNode> > node_record_type;
  BOOST_FOREACH(node_record_type nr, nodes) {
    noden++;
    if(nr.second->type & (dfgNode::SDFG_FF|dfgNode::SDFG_LATCH|dfgNode::SDFG_MODULE))
      nlist.push_back(nr.second);
  }

  std::set<shared_ptr<dfgNode> > fakes; // for debug reasons
  while(!nlist.empty()) {
    shared_ptr<dfgNode> cn = nlist.front();
    nlist.pop_front();
    //if(verbose) std::cout << "analyse the paths of " << cn->get_hier_name() << std::endl;
    if(cn->type & (dfgNode::SDFG_FF|dfgNode::SDFG_LATCH)) { // register
      regn++;
      std::set<shared_ptr<dfgNode> > tar_set;
      tar_set.insert(cn);
      list<shared_ptr<dfgPath> >& pathlist = cn->get_out_paths();
      BOOST_FOREACH(shared_ptr<dfgPath> p, pathlist) {
        if((p->tar == cn) && (p->type & dfgEdge::SDFG_CTL)) {
          pfsm.insert(cn);
          pfsmn++;
          goto NODE_ACCEPTED;
        }
      }
      fakes.insert(cn); // for debug
    } else {
      // must be module
      if(cn->child) {
        BOOST_FOREACH(node_record_type nr, cn->child->nodes) {
          noden++;
          if(nr.second->type & (dfgNode::SDFG_FF|dfgNode::SDFG_LATCH|dfgNode::SDFG_MODULE))
            nlist.push_back(nr.second);
        }
      }
    }
  NODE_ACCEPTED:
    continue;
  }

  // remove fake FSMs
  // must have control output to other node
  std::set<shared_ptr<dfgNode> > ffsm; // fake FSMs
  BOOST_FOREACH(shared_ptr<dfgNode> n, pfsm) {
    list<shared_ptr<dfgPath> >& po = n->get_out_paths();
    BOOST_FOREACH(shared_ptr<dfgPath>p, po) {
      if((p->tar != n) && (p->type & dfgEdge::SDFG_CTL)) {
        goto FSM_HAS_OUT_CTL;
      }
    }
    // a true FSM should have jump out already
    ffsm.insert(n);
    continue;

  FSM_HAS_OUT_CTL:
    // all data input should be const(omitted), itself
    list<shared_ptr<dfgPath> >& pi = n->get_in_paths();
    BOOST_FOREACH(shared_ptr<dfgPath> p, pi) {
      if(p->type & dfgEdge::SDFG_DP) {
        if(p->src != n) {
          ffsm.insert(n);
          break;
        }
      }
    }
  }

  // report fakes, debug
  if(verbose) {
    BOOST_FOREACH(shared_ptr<dfgNode> n, fakes) {
      std::cout << n->get_hier_name() << " is a fake FSM." << std::endl;
    }
  }
  
  // remove fake fsms
  BOOST_FOREACH(shared_ptr<dfgNode> n, ffsm) {
    if(verbose) std::cout << n->get_hier_name() << " is a fake FSM." << std::endl;
    pfsm.erase(n);
  }

  // figure out the relations of FSMs
  unsigned int number_of_groups = 0;
  map<shared_ptr<dfgNode>, unsigned int> group_map;
  BOOST_FOREACH(shared_ptr<dfgNode> n, pfsm) {
    if(group_map.count(n)) continue;
 
    // get all the FSMs connected
    unsigned int gid = 0;
    std::set<shared_ptr<dfgNode> > connected_fsm;
    BOOST_FOREACH(shared_ptr<dfgPath> p, n->get_out_paths()) {
      if(pfsm.count(p->tar)) {  // it is a FSM
        connected_fsm.insert(p->tar);
        if(group_map.count(p->tar)) { // try to get a group id
          if(gid == 0 || gid >= group_map[p->tar])
            gid = group_map[p->tar];
        }
      }
    }
    BOOST_FOREACH(shared_ptr<dfgPath> p, n->get_in_paths()) {
      if(pfsm.count(p->src)) {  // it is a FSM
        connected_fsm.insert(p->tar);
        if(group_map.count(p->src)) { // try to get a group id
          if(gid == 0 || gid >= group_map[p->src])
            gid = group_map[p->tar];
        }
      }
    }
    if(gid == 0) gid = ++number_of_groups;
    group_map[n] = gid;
    BOOST_FOREACH(shared_ptr<dfgNode> n, connected_fsm) {
      group_map[n] = gid;
    }
  }

  map<unsigned int, list<shared_ptr<dfgNode> > > fmap;
  typedef pair<const shared_ptr<dfgNode>, unsigned int> group_map_type;
  BOOST_FOREACH(group_map_type gm, group_map) {
    fmap[gm.second].push_back(gm.first);
  }

  list<list<shared_ptr<dfgNode> > > rv;
  typedef pair<const unsigned int, list<shared_ptr<dfgNode> > > fmap_type;
  BOOST_FOREACH(fmap_type fm, fmap) {
    rv.push_back(fm.second);
  }

  // report:
  std::cout << "\n\nSUMMARY:" << std::endl;
  std::cout << "In a design of " << noden << " nodes, " << regn << " nodes are registers." << std::endl;
  std::cout << "Find " << pfsmn << " potential FSMs but finally reduce to " << pfsm.size() << " in "
            << rv.size() << " groups:" << std::endl; 

  return rv;
}

list<list<shared_ptr<dfgNode> > > SDFG::dfgGraph::get_fsm_groups_fast(bool verbose) const {
  // find all registers who has self-loops
  list<shared_ptr<dfgNode> > nlist;
  std::set<shared_ptr<dfgNode> > pfsm; // potential FSMs
  unsigned int noden = 0;              // total number of nodes 
  unsigned int regn = 0;               // total number of registers
  unsigned int pfsmn = 0;              // total number of potential FSMs
  typedef pair<const vertex_descriptor, shared_ptr<dfgNode> > node_record_type;
  BOOST_FOREACH(node_record_type nr, nodes) {
    noden++;
    if(nr.second->type & (dfgNode::SDFG_FF|dfgNode::SDFG_LATCH|dfgNode::SDFG_MODULE))
      nlist.push_back(nr.second);
  }

  std::set<shared_ptr<dfgNode> > fakes; // for debug reasons
  while(!nlist.empty()) {
    shared_ptr<dfgNode> cn = nlist.front();
    nlist.pop_front();
    //if(verbose) std::cout << "analyse the paths of " << cn->get_hier_name() << std::endl;
    if(cn->type & (dfgNode::SDFG_FF|dfgNode::SDFG_LATCH)) { // register
      regn++;
      std::set<shared_ptr<dfgNode> > tar_set;
      tar_set.insert(cn);
      list<shared_ptr<dfgPath> > pathlist = cn->get_out_paths_fast();
      BOOST_FOREACH(shared_ptr<dfgPath> p, pathlist) {
        if((p->tar == cn) && (p->type & dfgEdge::SDFG_CTL)) {
          pfsm.insert(cn);
          pfsmn++;
          goto NODE_ACCEPTED;
        }
      }
      fakes.insert(cn); // for debug
    } else {
      // must be module
      if(cn->child) {
        BOOST_FOREACH(node_record_type nr, cn->child->nodes) {
          noden++;
          if(nr.second->type & (dfgNode::SDFG_FF|dfgNode::SDFG_LATCH|dfgNode::SDFG_MODULE))
            nlist.push_back(nr.second);
        }
      }
    }
  NODE_ACCEPTED:
    continue;
  }

  // remove fake FSMs
  // must have control output to other node
  std::set<shared_ptr<dfgNode> > ffsm; // fake FSMs
  BOOST_FOREACH(shared_ptr<dfgNode> n, pfsm) {
    list<shared_ptr<dfgPath> > po = n->get_out_paths_fast();
    BOOST_FOREACH(shared_ptr<dfgPath>p, po) {
      if((p->tar != n) && (p->type & dfgEdge::SDFG_CTL)) {
        goto FSM_HAS_OUT_CTL;
      }
    }
    // a true FSM should have jump out already
    ffsm.insert(n);
    continue;

  FSM_HAS_OUT_CTL:
    // all data input should be const(omitted), itself
    list<shared_ptr<dfgPath> > pi = n->get_in_paths_fast();
    BOOST_FOREACH(shared_ptr<dfgPath> p, pi) {
      if(p->type & dfgEdge::SDFG_DP) {
        if(p->src != n) {
          ffsm.insert(n);
          break;
        }
      }
    }
  }

  // report fakes, debug
  if(verbose) {
    BOOST_FOREACH(shared_ptr<dfgNode> n, fakes) {
      std::cout << n->get_hier_name() << " is a fake FSM." << std::endl;
    }
  }
  
  // remove fake fsms
  BOOST_FOREACH(shared_ptr<dfgNode> n, ffsm) {
    if(verbose) std::cout << n->get_hier_name() << " is a fake FSM." << std::endl;
    pfsm.erase(n);
  }

  // figure out the relations of FSMs
  unsigned int number_of_groups = 0;
  map<shared_ptr<dfgNode>, unsigned int> group_map;
  BOOST_FOREACH(shared_ptr<dfgNode> n, pfsm) {
    if(group_map.count(n)) continue;
 
    // get all the FSMs connected
    unsigned int gid = 0;
    std::set<shared_ptr<dfgNode> > connected_fsm;
    BOOST_FOREACH(shared_ptr<dfgPath> p, n->get_out_paths_fast()) {
      if(pfsm.count(p->tar)) {  // it is a FSM
        connected_fsm.insert(p->tar);
        if(group_map.count(p->tar)) { // try to get a group id
          if(gid == 0 || gid >= group_map[p->tar])
            gid = group_map[p->tar];
        }
      }
    }
    BOOST_FOREACH(shared_ptr<dfgPath> p, n->get_in_paths_fast()) {
      if(pfsm.count(p->src)) {  // it is a FSM
        connected_fsm.insert(p->tar);
        if(group_map.count(p->src)) { // try to get a group id
          if(gid == 0 || gid >= group_map[p->src])
            gid = group_map[p->tar];
        }
      }
    }
    if(gid == 0) gid = ++number_of_groups;
    group_map[n] = gid;
    BOOST_FOREACH(shared_ptr<dfgNode> n, connected_fsm) {
      group_map[n] = gid;
    }
  }

  map<unsigned int, list<shared_ptr<dfgNode> > > fmap;
  typedef pair<const shared_ptr<dfgNode>, unsigned int> group_map_type;
  BOOST_FOREACH(group_map_type gm, group_map) {
    fmap[gm.second].push_back(gm.first);
  }

  list<list<shared_ptr<dfgNode> > > rv;
  typedef pair<const unsigned int, list<shared_ptr<dfgNode> > > fmap_type;
  BOOST_FOREACH(fmap_type fm, fmap) {
    rv.push_back(fm.second);
  }

  // report:
  std::cout << "\n\nSUMMARY:" << std::endl;
  std::cout << "In a design of " << noden << " nodes, " << regn << " nodes are registers." << std::endl;
  std::cout << "Find " << pfsmn << " potential FSMs but finally reduce to " << pfsm.size() << " in "
            << rv.size() << " groups:" << std::endl; 

  return rv;
}
