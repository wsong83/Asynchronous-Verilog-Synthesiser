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
    shared_ptr<dfgPath> p(new dfgPath(*mp));
    p->push_back(pn, m.second);
    m.first->out_path_type_update_fast(out_paths, p, rmap);
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
    mp->push_front(m.first, m.second);
    mp->tar = pn;
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
  //std::cout << get_full_name() << std::endl;
  
  // check node type
  if((pn->type & (SDFG_FF|SDFG_LATCH))         || // register
     (pn->type & SDFG_PORT)                       // output
     ) {  // ending point
    if(rv.count(pn)) rv[pn] |= cp->type;
    else             rv[pn] = cp->type;
    if(cp->path.back().first != pn)
      rmap[cp->path.back().first][pn] = cp->path.back().second;
    //std::cout << "    " << pn->get_hier_name()  << " : " << rv.size() << ":" << cp->path.size() << std::endl;
    return;
  }

  if(pn->type & SDFG_MODULE) {  // module
    std::cout << get_full_name() << std::endl;
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
      shared_ptr<dfgPath> p(new dfgPath(*cp));
      p->push_back(pn, t.second);
      t.first->out_path_type_update_fast(rv, p, rmap);
      // update rmap
      BOOST_FOREACH(rmap_data_type& m, rmap[t.first]) {
        if(m.first) {
          if(rmap[pn].count(m.first)) rmap[pn][m.first] |= dfgPath::cal_type(t.second, m.second);
          else                        rmap[pn][m.first] = dfgPath::cal_type(t.second, m.second);
        } else {
          if(rmap[pn].count(t.first)) rmap[pn][t.first] |= t.second;
          else                        rmap[pn][t.first] = t.second;
        }
      }
      if(t.first->type & SDFG_MODULE) { // module, put this node as a terminal
        rmap[pn][shared_ptr<dfgNode>()] = SDFG_DF;
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
  if((pn->type & (SDFG_FF|SDFG_LATCH))         || // register
     (pn->type & SDFG_PORT && !pn->pg->father)    // top-level output
     ) {  // ending point
    if(rv.count(pn)) rv[pn] |= cp->type;
    else             rv[pn] = cp->type;
    if(cp->path.back().first != pn)
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
      // update rmap
      BOOST_FOREACH(rmap_data_type& m, rmap[t.first]) {
        if(rmap[pn].count(m.first)) rmap[pn][m.first] |= dfgPath::cal_type(t.second, m.second);
        else                        rmap[pn][m.first] = dfgPath::cal_type(t.second, m.second);
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
  if((pn->type & (SDFG_FF|SDFG_LATCH))         || // register
     (pn->type & SDFG_PORT && level==0)    // top-level output
     ) {  // ending point
    if(rv.count(pn)) rv[pn] |= cp->type;
    else             rv[pn] = cp->type;
    if(cp->path.back().first != pn)
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
  if((pn->type & (SDFG_FF|SDFG_LATCH))         || // register
     (pn->type & SDFG_PORT && !pn->pg->father)    // top-level input
     ) {  // ending point
    if(rv.count(pn)) rv[pn] |= cp->type;
    else             rv[pn] = cp->type;
    if(cp->path.back().first != pn)
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

void SDFG::dfgNode::in_path_type_update_fast_im(map<shared_ptr<dfgNode>, int>& rv, // return path group
                                                shared_ptr<dfgPath>& cp, // current path
                                                map<shared_ptr<dfgNode>, map<shared_ptr<dfgNode>, int > >& rmap,
                                                unsigned int level) {
  
  // this node
  shared_ptr<dfgNode> pn = pg->get_node(id);
  //std::cout << *cp << std::endl;
  
  // check node type
  if((pn->type & (SDFG_FF|SDFG_LATCH))         || // register
     (pn->type & SDFG_PORT && level==0)    // top-level input
     ) {  // ending point
    if(rv.count(pn)) rv[pn] |= cp->type;
    else             rv[pn] = cp->type;
    if(cp->path.back().first != pn)
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
     (pn->type & SDFG_PORT && level==0)           // top-level output
     ) {  // ending point
    if(pn == cp->src) {
      if(rv.count(pn)) rv[pn] |= cp->type;        // save it into rv
      else             rv[pn] = cp->type;
      if(cp->path.back().first != pn)
        rmap[cp->path.back().first][pn] = cp->path.back().second;  // rmap should record a type for the source
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

/////////////////////////////////////////////////////////////////////////////
/********        Edge                                               ********/
/////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////
/********        Graph                                              ********/
/////////////////////////////////////////////////////////////////////////////


///////////////////////////////
// analyse functions
///////////////////////////////

shared_ptr<dfgGraph> SDFG::dfgGraph::get_datapath() const {

  // new register graph
  shared_ptr<dfgGraph> ng(new dfgGraph(name));

  // iterate all nodes for data nodes
  std::set<shared_ptr<dfgNode> > data_nodes; 
  typedef pair<const vertex_descriptor, shared_ptr<dfgNode> > node_record_type;
  BOOST_FOREACH(shared_ptr<dfgNode> n, 
                get_list_of_nodes(dfgNode::SDFG_COMB|dfgNode::SDFG_FF|dfgNode::SDFG_LATCH|dfgNode::SDFG_GATE)) {
    bool keep = false;
    BOOST_FOREACH(shared_ptr<dfgEdge> e, get_in_edges(n)) {
      if(e->type & dfgEdge::SDFG_DP) {
        keep = true;
        break;
      }
    }
    if(!keep) {
      BOOST_FOREACH(shared_ptr<dfgEdge> e, get_out_edges(n)) {
        if(e->type & dfgEdge::SDFG_DP) {
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
  BOOST_FOREACH(node_record_type n, nodes) {
    if(data_nodes.count(n.second)) {
      related_nodes.insert(n.second);
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
    if(n->type & dfgNode::SDFG_MODULE)
      nnode->child = nnode->child->get_datapath();
    ng->add_node(nnode);
  }
  BOOST_FOREACH(shared_ptr<dfgNode> n, related_nodes) {
    BOOST_FOREACH(shared_ptr<dfgEdge> e, get_in_edges(n)) {
      if((e->type != dfgEdge::SDFG_RST) && (e->type != dfgEdge::SDFG_CLK) && related_nodes.count(get_source(e))) {
        ng->add_edge(e->name, e->type, get_source(e)->get_hier_name(), n->get_hier_name());
      }
    }
    BOOST_FOREACH(shared_ptr<dfgEdge> e, get_out_edges(n)) {
      if((e->type != dfgEdge::SDFG_RST) && (e->type != dfgEdge::SDFG_CLK) && related_nodes.count(get_target(e))) {
        ng->add_edge(e->name, e->type, n->get_hier_name(), get_target(e)->get_hier_name());
      }
    }    
  }
  return ng->get_hier_RRG();
}

shared_ptr<dfgGraph> SDFG::dfgGraph::get_hier_RRG() const {

  // new register graph
  shared_ptr<dfgGraph> ng(new dfgGraph(name));
  
  // node to visit
  list<shared_ptr<dfgNode> > nlist = 
    get_list_of_nodes(dfgNode::SDFG_FF|dfgNode::SDFG_LATCH|dfgNode::SDFG_PORT);
  list<shared_ptr<dfgNode> > mlist = 
    get_list_of_nodes(dfgNode::SDFG_MODULE);

  // add node to the new graph
  BOOST_FOREACH(shared_ptr<dfgNode> nr, nlist)
    copy_a_node(ng, nr);

  // add modules
  BOOST_FOREACH(shared_ptr<dfgNode> nr, mlist) {
    shared_ptr<dfgNode> nnode = copy_a_node(ng, nr);
    nnode->child = nr->child->get_hier_RRG();
    BOOST_FOREACH(shared_ptr<dfgNode> m, get_in_nodes(nr)) {
      if(!ng->exist(m->get_full_name()))
        copy_a_node(ng, m);
      ng->add_edge(m->get_full_name(), dfgEdge::SDFG_DF, m->get_full_name(), nr->get_full_name());
    }
    BOOST_FOREACH(shared_ptr<dfgNode> m, get_out_nodes(nr)) {
      if(!ng->exist(m->get_full_name())) {
        copy_a_node(ng, m);
        nlist.push_back(m);
      }
      ng->add_edge(nr->get_full_name(), dfgEdge::SDFG_DF, nr->get_full_name(), m->get_full_name());
    }    
  }

  while(!nlist.empty()) {
    shared_ptr<dfgNode> cn = nlist.front();
    nlist.pop_front();
    if(cn->type != dfgNode::SDFG_OPORT){ // IPORT, FF and Latch
      BOOST_FOREACH(shared_ptr<dfgPath> po, cn->get_out_paths_fast()) {
        // add arcs
        if(po->type & dfgEdge::SDFG_DP)
          ng->add_edge(cn->get_full_name(), dfgEdge::SDFG_DP, cn->get_full_name(), po->tar->get_full_name());
        else if(po->type & dfgEdge::SDFG_DDP)
          ng->add_edge(cn->get_full_name(), dfgEdge::SDFG_DDP, cn->get_full_name(), po->tar->get_full_name());
        
        if(po->type & dfgEdge::SDFG_CTL)
          ng->add_edge(cn->get_full_name(), dfgEdge::SDFG_CTL, cn->get_full_name(), po->tar->get_full_name());

        if(po->type == 0)
          ng->add_edge(cn->get_full_name(), dfgEdge::SDFG_DF, cn->get_full_name(), po->tar->get_full_name());
      }
    }  
  }

  return ng;

}

shared_ptr<dfgGraph> SDFG::dfgGraph::get_RRG() const {

  // new register graph
  shared_ptr<dfgGraph> ng(new dfgGraph(name));

  // node to visit
  list<shared_ptr<dfgNode> > nlist = get_list_of_nodes(dfgNode::SDFG_FF|dfgNode::SDFG_LATCH|dfgNode::SDFG_PORT);

  BOOST_FOREACH(shared_ptr<dfgNode> nr, nlist) 
    copy_a_node(ng, nr);
  
  while(!nlist.empty()) {
    shared_ptr<dfgNode> cn = nlist.front();
    nlist.pop_front();
    list<shared_ptr<dfgPath> > po = cn->get_out_paths_fast_cb();
    BOOST_FOREACH(shared_ptr<dfgPath>p, po) {
      if(!ng->exist(p->tar->get_full_name())) { // add the new node to the RRG
        copy_a_node(ng, p->tar);
        nlist.push_back(p->tar);
      }

      // add arcs
      if(p->type & dfgEdge::SDFG_DP)
        ng->add_edge(cn->get_full_name(), dfgEdge::SDFG_DP, cn->get_full_name(), p->tar->get_full_name());
      else if(p->type & dfgEdge::SDFG_DDP)
        ng->add_edge(cn->get_full_name(), dfgEdge::SDFG_DDP, cn->get_full_name(), p->tar->get_full_name());

      if((p->type & dfgEdge::SDFG_RST) == dfgEdge::SDFG_RST)
        ng->add_edge(cn->get_full_name(), dfgEdge::SDFG_RST, cn->get_full_name(), p->tar->get_full_name());
      else if((p->type & dfgEdge::SDFG_CLK) == dfgEdge::SDFG_CLK)
        ng->add_edge(cn->get_full_name(), dfgEdge::SDFG_CLK, cn->get_full_name(), p->tar->get_full_name());
      else if(p->type & dfgEdge::SDFG_CTL)
        ng->add_edge(cn->get_full_name(), dfgEdge::SDFG_CTL, cn->get_full_name(), p->tar->get_full_name());

      if(p->type == 0)
        ng->add_edge(cn->get_full_name(), dfgEdge::SDFG_DF, cn->get_full_name(), p->tar->get_full_name());
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

std::set<shared_ptr<dfgNode> > SDFG::dfgGraph::get_fsm_groups(bool verbose, shared_ptr<dfgGraph> RRG) const {
  // find all registers who has self-loops
  list<shared_ptr<dfgNode> > nlist = get_list_of_nodes(dfgNode::SDFG_FF|dfgNode::SDFG_LATCH|dfgNode::SDFG_MODULE);
  unsigned int noden = nlist.size();   // number of nodes
  std::set<shared_ptr<dfgNode> > pfsm; // potential FSMs
  unsigned int regn = 0;               // total number of registers
  unsigned int pfsmn = 0;              // total number of potential FSMs
  
  typedef pair<const vertex_descriptor, shared_ptr<dfgNode> > node_record_type;
  BOOST_FOREACH(node_record_type nr, nodes) {
    noden++;
    if(nr.second->type & (dfgNode::SDFG_FF|dfgNode::SDFG_LATCH|dfgNode::SDFG_MODULE))
      nlist.push_back(nr.second);
  }

  std::set<shared_ptr<dfgNode> > fakes_co, fakes_di; // for debug reasons
  while(!nlist.empty()) {
    shared_ptr<dfgNode> cn = nlist.front();
    nlist.pop_front();
    //if(verbose) std::cout << "analyse the paths of " << cn->get_hier_name() << std::endl;
    if(cn->type & (dfgNode::SDFG_FF|dfgNode::SDFG_LATCH)) { // register
      regn++;
      list<shared_ptr<dfgPath> > pathlist = cn->get_self_path_cb();
      BOOST_FOREACH(shared_ptr<dfgPath> p, pathlist) {
        if(p->type & (dfgEdge::SDFG_CTL|dfgEdge::SDFG_DP)) {
          pfsm.insert(cn);
          pfsmn++;
          break;
        }
      }
    } else {
      // must be module
      if(cn->child) {
        list<shared_ptr<dfgNode> > m_list = 
          get_list_of_nodes(dfgNode::SDFG_FF|dfgNode::SDFG_LATCH|dfgNode::SDFG_MODULE, *(cn->child));
        noden += m_list.size();
        nlist.splice(nlist.end(), m_list);
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

    if(!(etype & dfgEdge::SDFG_CTL)) {
      fakes_co.insert(n);
      continue;
    }

    etype = dfgEdge::SDFG_DF;
    BOOST_FOREACH(shared_ptr<dfgEdge> e, RRG->get_in_edges(n->get_full_name())) {
      if(RRG->get_source(e)->get_hier_name() != n->get_full_name()) 
        etype |= e->type;
    }
    
    if(etype & dfgEdge::SDFG_DP) {
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

  // report:
  std::cout << "\n\nSUMMARY:" << std::endl;
  std::cout << "In a design of " << noden << " nodes, " << regn << " nodes are registers." << std::endl;
  std::cout << "Find " << pfsmn << " potential FSMs but finally reduce to " << pfsm.size() << " FSM registers." << std::endl; 

  return pfsm;
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
      
list<shared_ptr<dfgNode> > SDFG::dfgGraph::get_list_of_nodes(unsigned int types) const {
  list<shared_ptr<dfgNode> > nlist;
  typedef pair<const vertex_descriptor, shared_ptr<dfgNode> > node_record_type;
  BOOST_FOREACH(node_record_type nr, nodes) {
    if(nr.second->type & types) nlist.push_back(nr.second);
  }
  return nlist;
}

list<shared_ptr<dfgNode> > SDFG::dfgGraph::get_list_of_nodes(unsigned int types, const dfgGraph& G) const {
  return G.get_list_of_nodes(types);
}

shared_ptr<dfgNode> SDFG::dfgGraph::copy_a_node(shared_ptr<dfgGraph> G, shared_ptr<dfgNode> cn) const {
  shared_ptr<dfgNode> nnode(cn->copy());
  nnode->set_hier_name(cn->get_full_name());
  G->add_node(nnode);
  return nnode;
}
