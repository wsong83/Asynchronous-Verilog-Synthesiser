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
#include <algorithm>
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

list<shared_ptr<dfgPath> > SDFG::dfgNode::get_out_paths(unsigned int pmax, const std::set<shared_ptr<dfgNode> >& targets) const {
  // return value and the main path
  list<shared_ptr<dfgPath> > rv;
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
  for_each(rmap[pn].begin(), rmap[pn].end(),
           [&](pair<const shared_ptr<dfgNode>, int>& m) {
             if(pmax == 0  || rv.size() < pmax) {
               shared_ptr<dfgPath> p(new dfgPath(*mp));
               p->push_back(pn, m.second);
               m.first->out_path_type_update(rv, p, pmax, targets, rmap, dnode_set);
             }
           });
  
  return rv;
}

list<shared_ptr<dfgPath> > SDFG::dfgNode::get_in_paths(unsigned int pmax, const std::set<shared_ptr<dfgNode> >& sources) const {
  // return value and the main path
  list<shared_ptr<dfgPath> > rv;
  shared_ptr<dfgPath> mp(new dfgPath());       // main path
  shared_ptr<dfgNode> pn = pg->get_node(id); // this node

  // cache
  map<shared_ptr<dfgNode>, map<shared_ptr<dfgNode>, int > > rmap; // node relation map
  std::set<shared_ptr<dfgNode> > dnode_set;                   // dead node set to store the node do not lead to the target

  // initial operation
  // build up the relation map
  list<shared_ptr<dfgEdge> > oe_list = pg->get_in_edges_cb(id); // out edge list
  BOOST_FOREACH(shared_ptr<dfgEdge> e, oe_list) {
    shared_ptr<dfgNode> src = e->pg->get_source_cb(e->id);
    if(rmap[pn].count(src))
      rmap[pn][src] |= e->type;
    else
      rmap[pn][src] = e->type;
  }

  // visit all in nodes
  for_each(rmap[pn].begin(), rmap[pn].end(),
           [&](pair<const shared_ptr<dfgNode>, int>& m) {
             if(pmax == 0  || rv.size() < pmax) {
               shared_ptr<dfgPath> p(new dfgPath(*mp));
               p->push_front(m.first, m.second);
               p->tar = pn;
               m.first->in_path_type_update(rv, p, pmax, sources, rmap, dnode_set);
             }
           });
  
  return rv;
}

void SDFG::dfgNode::out_path_type_update(list<shared_ptr<dfgPath> >& rv, // return path group
                                         shared_ptr<dfgPath>& cp, // current path
                                         unsigned int pmax,       // maximal number of path to be returned
                                         const std::set<shared_ptr<dfgNode> >& targets, // target nodes
                                         map<shared_ptr<dfgNode>, map<shared_ptr<dfgNode>, int > >& rmap,
                                         std::set<shared_ptr<dfgNode> >& dnode_set) const {
  // check whether need to go forward
  if(pmax != 0 && rv.size() >= pmax) return; // already have enough number of paths
  
  // this node
  shared_ptr<dfgNode> pn = pg->get_node(id);

  // check node type
  if((pn->type & (SDFG_FF|SDFG_LATCH))         || // register
     (pn->type & SDFG_PORT && !pn->pg->father)    // top-level output
     ) {  // ending point
    if(targets.empty() || targets.count(pn)) {
      cp->tar = pn;
      rv.push_back(cp);
    }
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

  unsigned int rv_size = rv.size();
  for_each(rmap[pn].begin(), rmap[pn].end(),
           [&](pair<const shared_ptr<dfgNode>, int>& m) {
             shared_ptr<dfgPath> p(new dfgPath(*cp));
             p->push_back(pn, m.second);
             m.first->out_path_type_update(rv, p, pmax, targets, rmap, dnode_set);
           });

  if(rv.size() == rv_size) {         // this is a dead node
    dnode_set.insert(pn);
    rmap.erase(pn);
  }

}

void SDFG::dfgNode::in_path_type_update(list<shared_ptr<dfgPath> >& rv, // return path group
                                        shared_ptr<dfgPath>& cp, // current path
                                        unsigned int pmax,       // maximal number of path to be returned
                                        const std::set<shared_ptr<dfgNode> >& sources, // source nodes
                                        map<shared_ptr<dfgNode>, map<shared_ptr<dfgNode>, int > >& rmap,
                                        std::set<shared_ptr<dfgNode> >& dnode_set) const {
  // check whether need to go forward
  if(pmax != 0 && rv.size() >= pmax) return; // already have enough number of paths
  
  // this node
  shared_ptr<dfgNode> pn = pg->get_node(id);

  // check node type
  if((pn->type & (SDFG_FF|SDFG_LATCH))         || // register
     (pn->type & SDFG_PORT && !pn->pg->father)    // top-level output
     ) {  // ending point
    if(sources.empty() || sources.count(pn)) {
      cp->src = pn;
      rv.push_back(cp);
    }
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
  for_each(rmap[pn].begin(), rmap[pn].end(),
           [&](pair<const shared_ptr<dfgNode>, int>& m) {
             // no loop assert
             if(cp->node_set.count(m.first)) {
               G_ENV->error("SDFG-ANALYSE-0", toString(*cp));
               return;
             } else {
               shared_ptr<dfgPath> p(new dfgPath(*cp));
               p->push_front(m.first, m.second);
               m.first->in_path_type_update(rv, p, pmax, sources, rmap, dnode_set);
             }
           });

  if(rv.size() == rv_size) {         // this is a dead node
    dnode_set.insert(pn);
    rmap.erase(pn);
  }

}

void SDFG::dfgNode::simplify(std::set<boost::shared_ptr<dfgNode> >& proc_set, bool quiet) {

  if(!pg) return;               // this node is already deleted

  // remove internal node without output edges
  if(pg->size_out_edges_cb(id) == 0) {
    if(!((type & SDFG_PORT) && (type != SDFG_IPORT) && (pg->father == NULL))) {
      BOOST_FOREACH(shared_ptr<dfgNode> m, pg->get_in_nodes_cb(id)) {
        assert(m);
        proc_set.insert(m);
      }
      
      if((type & SDFG_PORT) && (type != SDFG_OPORT) && (pg->father)) {// if it is an input, the whole module may be useless
        assert(pg->father->pg->nodes[pg->father->id]);
        proc_set.insert(pg->father->pg->nodes[pg->father->id]);
      }
      
      if(!quiet)
        G_ENV->error("SDFG-SIMPLIFY-0", pg->name + "/" + get_hier_name());

      pg->remove_node(id);        // remove it

      return;
    }
  }

  // remove the node that has only one input and it is a comb or unknown
  if(pg->size_in_edges(id) == 1 && (type == SDFG_COMB || type == SDFG_DF)) {
    // get its source and target
    shared_ptr<dfgEdge> src = pg->get_in_edges(id).front();
    list<shared_ptr<dfgEdge> > tar_list = pg->get_out_edges(id);
    bool no_module = true;
    BOOST_FOREACH(shared_ptr<dfgEdge> m, tar_list) {
      shared_ptr<dfgNode> tar_node = pg->get_target(m);
      no_module &= tar_node->type != SDFG_MODULE;
    }
    shared_ptr<dfgNode> src_node = pg->get_source(src);
    no_module &= src_node->type != SDFG_MODULE;

    // make sure it is not a signal connected to a module/iport (remove it will make the graph to crowed)
    if(!(src_node->type & SDFG_MODULE) && no_module) {
      // choosing the target edge type
      dfgEdge::edge_type_t etype;
      
      // add edges to by pass the node
      BOOST_FOREACH(shared_ptr<dfgEdge> m, tar_list) {
        // specify the type
        if(m->type == dfgEdge::SDFG_DF)
          etype = src->type;   // use src type if tar type is unknown
        else
          etype = m->type;     // always use the tar type if it is available

        // add the new path
        shared_ptr<dfgNode> tar_node = m->pg->get_target(m);
        tar_node->pg->add_edge(src->name, etype, src_node->id, tar_node->id);

        // process target again later
        assert(tar_node);
        proc_set.insert(tar_node);
      }

      // process source again later
      assert(src_node);
      proc_set.insert(src_node);
      
      if(!quiet)
        G_ENV->error("SDFG-SIMPLIFY-1", 
                     pg->name + "/" + get_hier_name(),
                     src->pg->name + "/" + src_node->get_hier_name()); 

      // remove the node
      pg->remove_node(id);

      return;
    }
  }

  // remove the node that has only one output, and it is a comb or unknown
  if(pg->size_out_edges(id) == 1 && (type == SDFG_COMB || type == SDFG_DF)) {
    // get its source and target
    list<shared_ptr<dfgEdge> > src_list = pg->get_in_edges(id);
    bool no_module = true;
    BOOST_FOREACH(shared_ptr<dfgEdge> m, src_list) {
      shared_ptr<dfgNode> src_node = m->pg->get_source(m);
      no_module &= src_node->type != SDFG_MODULE;
    }
    shared_ptr<dfgEdge> tar = pg->get_out_edges(id).front();
    shared_ptr<dfgNode> tar_node = pg->get_target(tar);
    no_module &= tar_node->type != SDFG_MODULE;

    // make sure it is not a signal connected to a module/oport (remove it will make the graph to crowed)
    if(!(tar_node->type & SDFG_MODULE) && no_module) {
      // choosing the target edge type
      dfgEdge::edge_type_t etype;

      // add edges to by pass the node
      BOOST_FOREACH(shared_ptr<dfgEdge> m, src_list) {
        // specify the type
        if(tar->type == dfgEdge::SDFG_DF)
          etype = m->type;   // use src type if tar type is unknown
        else
          etype = tar->type;     // always use the tar type if it is available

        // add the new path
        shared_ptr<dfgNode> src_node = m->pg->get_source(m);
        src_node->pg->add_edge(m->name, etype, src_node->id, tar_node->id);

        // process source again later
        assert(src_node);
        proc_set.insert(src_node);
      }

      // process source again later
      assert(tar_node);
      proc_set.insert(tar_node);
      
      if(!quiet)
        G_ENV->error("SDFG-SIMPLIFY-2",
                     pg->name + "/" + get_hier_name(),
                     tar->pg->name + "/" + tar_node->get_hier_name()); 

      // remove the node
      pg->remove_node(id);
      
      return;
    }
  }

  // remove through wires
  if(type == SDFG_OPORT && pg->size_in_edges(id) == 1 && pg->father) {
    shared_ptr<dfgNode> iport = pg->get_in_nodes(id).front();
    if(pg->size_out_edges(iport) == 1 && iport->type == SDFG_IPORT) {
      // get source and target
      shared_ptr<dfgNode> src = pg->get_in_nodes_cb(iport->id).front();
      shared_ptr<dfgNode> tar = pg->get_out_nodes_cb(id).front();
      
      // set up the bypass
      if(src && tar)
        pg->father->pg->add_edge(src->get_hier_name(), pg->get_in_edges(id).front()->type, src->id, tar->id);
      
      // add src and tar to proc_set
      if(src) proc_set.insert(src);
      if(tar) proc_set.insert(tar);
      proc_set.insert(pg->father->pg->nodes[pg->father->id]);
      
      // remove iport from proc_set if it is in the list
      proc_set.erase(iport);
      
      if(!quiet)
        G_ENV->error("SDFG-SIMPLIFY-3", 
                     pg->name + "/" + iport->get_hier_name(),
                     pg->name + "/" + get_hier_name(), 
                     pg->father->pg->name); 

      // remove the through wire
      pg->remove_node(iport);
      pg->remove_node(id);
      
      return;
    }
  }

  // move output driver to upper
  if(pg->size_in_edges(id) == 0 && type == SDFG_OPORT && pg->father) {
    // flatten the node
    shared_ptr<dfgNode> nnode = flatten();
    
    // duplicate the edges
    shared_ptr<dfgNode> tar = pg->get_out_nodes_cb(id).front();
    if(tar) {
      tar->pg->add_edge(nnode->get_hier_name(), pg->get_out_edges_cb(id).front()->type, nnode->id, tar->id);
      proc_set.insert(tar);
    }
    proc_set.insert(nnode);
    proc_set.insert(pg->father->pg->nodes[pg->father->id]);

    if(!quiet)
      G_ENV->error("SDFG-SIMPLIFY-4",
                   pg->name + "/" + get_hier_name(),
                   pg->father->pg->name); 

    // remove the node
    pg->remove_node(id);
    return;

  }

  // go lower if it is module node
  if(type == SDFG_MODULE && child) {
    child->simplify(proc_set, quiet);
    
    // flatten small modules
    if(0 == child->size_of_regs() && 10 > child->size_of_nodes()) { // flatten the module
      if(!quiet)
        G_ENV->error("SDFG-SIMPLIFY-5", child->name);
      list<shared_ptr<dfgNode> > nlist = child->flatten();
      list<shared_ptr<dfgNode> > slist = pg->get_in_nodes(id);
      list<shared_ptr<dfgNode> > tlist = pg->get_out_nodes(id);
      shared_ptr<dfgNode> pn = pg->get_node(id);
      pg->remove_node(id);
      proc_set.erase(pn);

      BOOST_FOREACH(shared_ptr<dfgNode> m, nlist) {
        assert(m);
        proc_set.insert(m);
      }
      BOOST_FOREACH(shared_ptr<dfgNode> m, slist) {
        assert(m);
        proc_set.insert(m);
      }
      BOOST_FOREACH(shared_ptr<dfgNode> m, tlist) {
        assert(m);
        proc_set.insert(m);
      }
    }
  }
}

void SDFG::dfgNode::path_deduction(std::set<boost::shared_ptr<dfgNode> >& proc_set, bool quiet) {
  if(!pg) return;               // this node is already deleted
  if(type & (SDFG_FF|SDFG_LATCH)) return; // only deduct wires in a path
  
  if(type == SDFG_MODULE) {
    if(child)
      child->path_deduction(proc_set, quiet);
    return;
  }
  
  int oetype = 0;
  int ietype = 0;
  list<shared_ptr<dfgEdge> > oelist = pg->get_out_edges_cb(id);
  list<shared_ptr<dfgEdge> > ielist = pg->get_in_edges_cb(id);
  
  BOOST_FOREACH(shared_ptr<dfgEdge> m, oelist) {
    oetype |= m->type;
  }
  
  BOOST_FOREACH(shared_ptr<dfgEdge> m, ielist) {
    ietype |= m->type;
  }
  
  if(oetype == dfgEdge::SDFG_CLK) { // clock signal
    if(!quiet)
      G_ENV->error("SDFG-DEDUCTION-0", pg->name + "/" + get_hier_name());
    
    BOOST_FOREACH(shared_ptr<dfgEdge> m, ielist) {
      if(m->type != dfgEdge::SDFG_CLK) {
        if(!m->pg->exist(m->pg->get_source(m), m->pg->get_target(m), dfgEdge::SDFG_CLK))
          m->type = dfgEdge::SDFG_CLK;
        else
          m->pg->remove_edge(m->id);
        proc_set.insert(m->pg->get_source_cb(m->id));
      }
    }
  }
  
  if(oetype == dfgEdge::SDFG_RST) { // reset signal
    if(!quiet)
      G_ENV->error("SDFG-DEDUCTION-1", pg->name + "/" + get_hier_name());
    
    BOOST_FOREACH(shared_ptr<dfgEdge> m, ielist) {
      if(m->type != dfgEdge::SDFG_RST) {
        if(!m->pg->exist(m->pg->get_source(m), m->pg->get_target(m), dfgEdge::SDFG_RST))
          m->type = dfgEdge::SDFG_RST;
        else
          m->pg->remove_edge(m->id);
        proc_set.insert(m->pg->get_source_cb(m->id));
      }
    }
  }

  if(oetype == dfgEdge::SDFG_CTL) { // control signal
    if(!quiet)
      G_ENV->error("SDFG-DEDUCTION-2", pg->name + "/" + get_hier_name());
    
    BOOST_FOREACH(shared_ptr<dfgEdge> m, ielist) {
      if(m->type != dfgEdge::SDFG_CTL) {
        if(!m->pg->exist(m->pg->get_source(m), m->pg->get_target(m), dfgEdge::SDFG_CTL))
          m->type = dfgEdge::SDFG_CTL;
        else
          m->pg->remove_edge(m->id);
        proc_set.insert(m->pg->get_source_cb(m->id));
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
void SDFG::dfgGraph::simplify(bool quiet) {
  // node cache
  std::set<shared_ptr<dfgNode> > proc_set;

  // in the first round, traverse all nodes
  simplify(proc_set, quiet);

  // handle the nodes need further operation
  while(!proc_set.empty()) {
    shared_ptr<dfgNode> pn = *(proc_set.begin());
    assert(pn);
    proc_set.erase(pn);
    pn->simplify(proc_set, quiet);
  }
}

void SDFG::dfgGraph::simplify(std::set<shared_ptr<dfgNode> >& proc_set, bool quiet) {
  // make a local copy of the node map, as nodes may be erased during the process
  map<vertex_descriptor, shared_ptr<dfgNode> > local_node_map = nodes;
  
  // do the simplification
  for_each(local_node_map.begin(), local_node_map.end(),
           [&](pair<const vertex_descriptor, shared_ptr<dfgNode> >& m) {
             m.second->simplify(proc_set, quiet);
           });
}

void SDFG::dfgGraph::path_deduction(bool quiet) {
  // node cache
  std::set<shared_ptr<dfgNode> > proc_set;

  // in the first round, traverse all nodes
  path_deduction(proc_set, quiet);

  // handle the nodes need further operation
  while(!proc_set.empty()) {
    shared_ptr<dfgNode> pn = *(proc_set.begin());
    assert(pn);
    proc_set.erase(pn);
    pn->path_deduction(proc_set, quiet);
  }
}

void SDFG::dfgGraph::path_deduction(std::set<shared_ptr<dfgNode> >& proc_set, bool quiet) {
  // make a local copy of the node map, as nodes may be erased during the process
  map<vertex_descriptor, shared_ptr<dfgNode> > local_node_map = nodes;
  
  // do the simplification
  for_each(local_node_map.begin(), local_node_map.end(),
           [&](pair<const vertex_descriptor, shared_ptr<dfgNode> >& m) {
             m.second->path_deduction(proc_set, quiet);
           });
}

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
    list<shared_ptr<dfgPath> > plist = cnode->get_out_paths(0, std::set<shared_ptr<dfgNode> >() );
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

list<list<shared_ptr<dfgNode> > > SDFG::dfgGraph::get_fsm_groups() const {
  // find all registers who has self-loops
  list<shared_ptr<dfgNode> > nlist;
  std::set<shared_ptr<dfgNode> > pfsm; // potential FSMs
  unsigned int regn = 0;        // total number of registers
  unsigned int pfsmn = 0;       // total number of potential FSMs
  typedef pair<const vertex_descriptor, shared_ptr<dfgNode> > node_record_type;
  BOOST_FOREACH(node_record_type nr, nodes) {
    if(nr.second->type & (dfgNode::SDFG_FF|dfgNode::SDFG_LATCH|dfgNode::SDFG_MODULE))
      nlist.push_back(nr.second);
  }

  while(!nlist.empty()) {
    shared_ptr<dfgNode> cn = nlist.front();
    nlist.pop_front();

    if(cn->type & (dfgNode::SDFG_FF|dfgNode::SDFG_LATCH)) { // register
      regn++;
      std::set<shared_ptr<dfgNode> > tar_set;
      tar_set.insert(cn);
      list<shared_ptr<dfgPath> > pathlist = cn->get_out_paths(0, tar_set);
      BOOST_FOREACH(shared_ptr<dfgPath> p, pathlist) {
        if(p->type & dfgEdge::SDFG_CTL) {
          list<shared_ptr<dfgNode> >::iterator pre, it, end;  
          list<int>::iterator tit;
          pre = p->path.begin();
          it = p->path.begin();
          tit = p->path_type.begin();
          end = p->path.end();
          for(++it; it!=end; pre=it++, ++tit) {
            shared_ptr<dfgNode> cnode = *it;
            shared_ptr<dfgNode> pnode = *pre;
            if(*tit & dfgEdge::SDFG_CTL) {
              self_ctl_loop = true;
              break;
            } else if(cnode->pg->size_in_edges(cnode->id) > 1)
              break;
          }
          
          if(self_ctl_loop) 
            break;
          else {
            shared_ptr<dfgNode> cnode = gnode;
            shared_ptr<dfgNode> pnode = *pre;
            if(*tit & dfgEdge::SDFG_CTL) {
              self_ctl_loop = true;
              break;
            }
          }

          pfsm.insert(cn);
          pfsmn++;
          break;
        }
      }
    } else {
      // must be module
      if(cn->child) {
        BOOST_FOREACH(node_record_type nr, cn->child->nodes) {
          if(nr.second->type & (dfgNode::SDFG_FF|dfgNode::SDFG_LATCH|dfgNode::SDFG_MODULE))
            nlist.push_back(nr.second);
        }
      }
    }
  }

  
          
        


  // scan all registers in the regg for registers who have self-control loops
  std::set<shared_ptr<dfgNode> > pfsm; // potential FSMs
  map<vertex_descriptor, shared_ptr<dfgNode> >::const_iterator it, end;
  for(it=regg->nodes.begin(), end=regg->nodes.end(); it!=end; ++it) {
    if(regg->exist(it->second, it->second, dfgEdge::SDFG_CTL))
      pfsm.insert(it->second);
  }

  // check the potential fsms
  std::set<shared_ptr<dfgNode> > ffsm; // fake fsm
  BOOST_FOREACH(shared_ptr<dfgNode> n, pfsm) {
    // remove the registers which is not a fsm
    // get the in paths and out paths
    std::set<shared_ptr<dfgNode> > dummyset;
    // the node in DFG
    shared_ptr<dfgNode> gnode = search_node(n->get_hier_name());
    list<shared_ptr<dfgPath> > po = gnode->get_out_paths(0, dummyset);
    
    // control self loop
    bool self_ctl_loop = false;
    BOOST_FOREACH(shared_ptr<dfgPath>p, po) {
      if(p->tar == gnode) {
        list<shared_ptr<dfgNode> >::iterator pre, it, end;  
        list<int>::iterator tit;
        pre = p->path.begin();
        it = p->path.begin();
        tit = p->path_type.begin();
        end = p->path.end();
        for(++it; it!=end; pre=it++, ++tit) {
          shared_ptr<dfgNode> cnode = *it;
          shared_ptr<dfgNode> pnode = *pre;
          if(*tit & dfgEdge::SDFG_CTL) {
            self_ctl_loop = true;
            break;
          } else if(cnode->pg->size_in_edges(cnode->id) > 1)
            break;
        }

        if(self_ctl_loop) 
          break;
        else {
          shared_ptr<dfgNode> cnode = gnode;
          shared_ptr<dfgNode> pnode = *pre;
          if(*tit & dfgEdge::SDFG_CTL) {
            self_ctl_loop = true;
            break;
          }
        }
      }
    }        
    if(!self_ctl_loop) { ffsm.insert(n); continue;}

    // must have control output to other node
    bool ctl_output = false;
    BOOST_FOREACH(shared_ptr<dfgPath>p, po) {
      if(p->tar != gnode) {
        list<shared_ptr<dfgNode> >::iterator pre, it, end;
        list<int>::iterator tit;
        pre = p->path.begin();
        it = p->path.begin();
        tit = p->path_type.begin();
        end = p->path.end();
        for(++it; it!=end; pre=it++, ++tit) {
          shared_ptr<dfgNode> cnode = *it;
          shared_ptr<dfgNode> pnode = *pre;
          if(*tit & dfgEdge::SDFG_CTL) {
            ctl_output = true;
            break;
          } else if(cnode->pg->size_in_edges(cnode->id) > 1)
            break;
        }

        if(ctl_output) 
          break;
        else {
          shared_ptr<dfgNode> cnode = p->tar;
          shared_ptr<dfgNode> pnode = *pre;
          if(*tit & dfgEdge::SDFG_CTL) {
            ctl_output = true;
            break;
          }
        }
      }
    }        
    if(!ctl_output) { ffsm.insert(n); continue;}
    

    // all data input should be const(omitted), itself
    list<shared_ptr<dfgPath> > pi = n->get_in_paths(0, dummyset);
    BOOST_FOREACH(shared_ptr<dfgPath> p, pi) {
      if(p->type & dfgEdge::SDFG_DP) {
        if(p->src != n) {
          ffsm.insert(n);
          break;
        }
      }
    }
  }

  // remove fake fsms
  BOOST_FOREACH(shared_ptr<dfgNode> n, ffsm) {
    std::cout << n->get_hier_name() << " is a fake FSM." << std::endl;
    pfsm.erase(n);
  }

  // generate the return value
  list<list<shared_ptr<dfgNode> > > rv;
  BOOST_FOREACH(shared_ptr<dfgNode> n, pfsm) {
    list<shared_ptr<dfgNode> > fsml;
    fsml.push_back(n);
    rv.push_back(fsml);
  }

  return rv;
}
