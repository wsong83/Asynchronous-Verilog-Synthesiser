/*
 * Copyright (c) 2011-2014 Wei Song <songw@cs.man.ac.uk> 
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
 * A graph in the SDFG library
 * 17/09/2012   Wei Song
 *
 *
 */

#include "dfg_node.hpp"
#include "dfg_edge.hpp"
#include "dfg_path.hpp"
#include "dfg_graph.hpp"

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include <cmath>


#include <ogdf/layered/SugiyamaLayout.h>
#include <ogdf/layered/FastHierarchyLayout.h>
#include <ogdf/layered/LongestPathRanking.h>
#include <ogdf/layered/OptimalRanking.h>
//#include <ogdf/layered/CoffmanGrahamRanking.h>
#include <ogdf/layered/GreedyCycleRemoval.h>
//#include <ogdf/layered/SplitHeuristic.h>
//#include <ogdf/layered/MedianHeuristic.h>
//#include <ogdf/layered/GreedyInsertHeuristic.h>

using namespace SDFG;
using boost::shared_ptr;
using std::map;
using std::string;
using std::pair;
using std::list;

namespace SDFG {

static const double G_NODE_H = 17.6;
static const double G_FONT_RATIO = 3.6;
static const double G_LAYER_DIST = 2;
static const double G_NODE_DIST = 3;

static const double PI = 3.14159265;

  double d2r(double degree) {
    return degree*PI/180.0;
  }

}

///////////////////////////////
// convert other types to vertex_decriptor
///////////////////////////////
vertex_descriptor SDFG::dfgGraph::to_id(const string& sname) const {
  if(node_map.count(sname)) return node_map.find(sname)->second;
  else                       return NULL;
} 

vertex_descriptor SDFG::dfgGraph::to_id(shared_ptr<dfgNode> pnode) const {
  if(pnode) return pnode->id;
  else      return NULL;
}

vertex_descriptor SDFG::dfgGraph::to_id(const vertex_descriptor& nid) const {
  return nid;
}

edge_descriptor SDFG::dfgGraph::to_id(shared_ptr<dfgEdge> pedge) const {
  if(pedge) return pedge->id;
  else      return edge_descriptor();
}

edge_descriptor SDFG::dfgGraph::to_id(const edge_descriptor& eid) const {
  return eid;
}

///////////////////////////////
// copy
///////////////////////////////

dfgGraph* SDFG::dfgGraph::deep_copy() const {

  dfgGraph * G(new dfgGraph(name));

  BOOST_FOREACH(nodes_type nrec, nodes) {
    shared_ptr<dfgNode> node(nrec.second->copy());
    node->set_hier_name(nrec.second->get_hier_name());
    G->add_node(node);
    if(node->type == dfgNode::SDFG_MODULE) {
      node->child = shared_ptr<dfgGraph>(nrec.second->child->deep_copy());
      node->child->father = node.get();
    }
  }

  BOOST_FOREACH(edges_type erec, edges) {
    G->add_edge(erec.second->name, erec.second->type, 
                erec.second->get_source()->get_hier_name(),
                erec.second->get_target()->get_hier_name());
  }
  
  return G;
}


///////////////////////////////
// add nodes and edges
///////////////////////////////
void SDFG::dfgGraph::add_node(shared_ptr<dfgNode> node) {
  assert(node);
  node->id = boost::add_vertex(bg_);
  nodes[node->id] = node;
  node_map[node->get_hier_name()] = node->id;
  node->pg = this;
  // generate and store an index
  unsigned int m_index = shash(node->get_hier_name());
  while(index_map.count(m_index)) ++m_index;
  node->node_index = m_index;
  index_map[m_index] = node->id;
}

shared_ptr<dfgNode> SDFG::dfgGraph::add_node(const string& n, dfgNode::node_type_t t) {
  shared_ptr<dfgNode> node(new dfgNode(n, t));
  add_node(node);
  return node;
}

void SDFG::dfgGraph::add_edge(shared_ptr<dfgEdge> edge, vertex_descriptor src, vertex_descriptor snk) {
  assert(edge);
  if(exist(src, snk, edge->type)) return; // do not insert a edge between the same nodes with the same type
  bool added;
  boost::tie(edge->id, added) = boost::add_edge(src, snk, bg_);
  edges[edge->id] = edge;
  edge->pg = this;
  unsigned int m_index = shash(nodes[src]->get_hier_name() + nodes[snk]->get_hier_name());
  while(edge_map.count(m_index)) ++m_index;
  edge->edge_index = m_index;
  edge_map[m_index] = edge->id;
}

shared_ptr<dfgEdge> SDFG::dfgGraph::add_edge(const string& n, dfgEdge::edge_type_t t, vertex_descriptor src, vertex_descriptor snk) {
  shared_ptr<dfgEdge> edge(new dfgEdge(n, t));
  add_edge(edge, src, snk);
  return edge;
}

void SDFG::dfgGraph::add_edge_multi(const string& n, int t, vertex_descriptor src, vertex_descriptor snk) {
    if(t == dfgEdge::SDFG_DF) add_edge(n, dfgEdge::SDFG_DF,  src, snk);
    if(t & dfgEdge::SDFG_DDP) add_edge(n, dfgEdge::SDFG_DDP, src, snk);
    if(t & dfgEdge::SDFG_CAL) add_edge(n, dfgEdge::SDFG_CAL, src, snk);
    if(t & dfgEdge::SDFG_ASS) add_edge(n, dfgEdge::SDFG_ASS, src, snk);
    if(t & dfgEdge::SDFG_DAT) add_edge(n, dfgEdge::SDFG_DAT, src, snk);
    if(t & dfgEdge::SDFG_LOG) add_edge(n, dfgEdge::SDFG_LOG, src, snk);
    if(t & dfgEdge::SDFG_ADR) add_edge(n, dfgEdge::SDFG_ADR, src, snk);
    if(t & dfgEdge::SDFG_CTL) add_edge(n, dfgEdge::SDFG_CTL, src, snk);
    if(t & dfgEdge::SDFG_CMP) add_edge(n, dfgEdge::SDFG_CMP, src, snk);
    if(t & dfgEdge::SDFG_EQU) add_edge(n, dfgEdge::SDFG_EQU, src, snk);
    if(t & dfgEdge::SDFG_CLK) add_edge(n, dfgEdge::SDFG_CLK, src, snk);
    if(t & dfgEdge::SDFG_RST) add_edge(n, dfgEdge::SDFG_RST, src, snk);
}
  

void SDFG::dfgGraph::add_path(shared_ptr<dfgPath> p) {
  shared_ptr<dfgNode> s, t;
  unsigned int ptype;
  if(p->path.empty()) {         // path generated by fast algorithm
    if(exist(p->src->get_hier_name())) {
      s = get_node(p->src->get_hier_name());
    } else {
      s.reset(p->src->copy());
      add_node(s);
    }
    if(exist(p->tar->get_hier_name())) {
      t = get_node(p->tar->get_hier_name());
    } else {
      t.reset(p->tar->copy());
      add_node(t);
    }

    add_edge_multi(s->name, p->type, s, t);
  } else {                      // normal path
    list<dfgPath::path_type> m_path = p->path;
    if(exist(p->src->get_hier_name())) {
      s = get_node(p->src->get_hier_name());
    } else {
      s.reset(p->src->copy());
      add_node(s);
    }
    while(!m_path.empty()) {
      if(exist(m_path.front().first->get_hier_name())) {
        s = get_node(m_path.front().first->get_hier_name());
      } else {
        s.reset(m_path.front().first->copy());
        add_node(s);
      }
      ptype = m_path.front().second;
      m_path.pop_front();
      shared_ptr<dfgNode> tar = m_path.empty() ? p->tar : m_path.front().first;
      if(exist(tar->get_hier_name())) {
        t = get_node(tar->get_hier_name());
      } else {
        t.reset(tar->copy());
        add_node(t);
      }

      add_edge_multi(s->name, ptype,  s, t);
    }
  }
}

///////////////////////////////
// remove nodes and edges
///////////////////////////////
void SDFG::dfgGraph::remove_node(vertex_descriptor nid) {
  if(!nodes.count(nid)) return;
  shared_ptr<dfgNode> pn = nodes[nid];

  //std::cout << "remove node: " << *pn << std::endl;
  //std::cout << "remove node: " << pn->get_full_name() << std::endl;


  // remove all output edges
  {
    list<shared_ptr<dfgEdge> > elist = get_out_edges(nid);
    BOOST_FOREACH(shared_ptr<dfgEdge> m, elist)
      remove_edge(m);
  }

  // remove all input edges
  {
    list<shared_ptr<dfgEdge> > elist = get_in_edges(nid);
    BOOST_FOREACH(shared_ptr<dfgEdge> m, elist)
      remove_edge(m);
  }

  // remove the node
  node_map.erase(pn->get_hier_name());
  nodes.erase(nid);
  index_map.erase(pn->node_index);
  boost::remove_vertex(nid, bg_);
  pn->pg = NULL;                    // make sure it cannot access graph

  if(pn->type & dfgNode::SDFG_PORT) { // port
    remove_port(*pn);
  }

  if(pn->type & dfgNode::SDFG_MODULE) { // module
    if(pn->child) {
      // clear maps to avoid double delete
      pn->port2sig.clear();
      pn->sig2port.clear();
      // remove all nodes
      map<vertex_descriptor, shared_ptr<dfgNode> > nlist = pn->child->nodes;
      for_each(nlist.begin(), nlist.end(), 
             [&](pair<const vertex_descriptor, shared_ptr<dfgNode> >& m) {
               pn->child->remove_node(m.first);
             });
    }
  }
}

void SDFG::dfgGraph::remove_edge(vertex_descriptor src, vertex_descriptor tar) {
  edge_descriptor eid;
  bool found;
  boost::tie(eid, found) = boost::edge(src, tar, bg_);
  while(found) {
    remove_edge(eid);
    boost::tie(eid, found) = boost::edge(src, tar, bg_);
  }
}

void SDFG::dfgGraph::remove_edge(vertex_descriptor src, vertex_descriptor tar, dfgEdge::edge_type_t etype) {
  shared_ptr<dfgEdge> pe = get_edge(src, tar, etype);
  if(pe) remove_edge(pe->id);
}

void SDFG::dfgGraph::remove_edge(edge_descriptor eid) {
  if(edges.count(eid)) {
    shared_ptr<dfgEdge> pe = edges[eid];
    shared_ptr<dfgNode> src = get_source(eid);
    shared_ptr<dfgNode> tar = get_target(eid);
    
    if(src->type == dfgNode::SDFG_MODULE) // remove a port signal
      src->remove_port_sig(tar->get_hier_name(), 1); // output

    if(tar->type == dfgNode::SDFG_MODULE)
      tar->remove_port_sig(src->get_hier_name(), -1); // input

    edges.erase(eid);
    edge_map.erase(pe->edge_index);
    boost::remove_edge(eid, bg_);
    pe->pg = NULL;
  }
}

void SDFG::dfgGraph::remove_port(const dfgNode& port) {
  if(father != NULL && father->port2sig.count(port.get_hier_name())) {
    string sname = father->port2sig[port.get_hier_name()];
    if(sname.size() > 0) {
      // remove the port map connection
      father->sig2port[sname].erase(port.get_hier_name());

      // scan it again to get input/output count
      switch(port.type) {
      case dfgNode::SDFG_IPORT:
      case dfgNode::SDFG_IPORT_CLK:
        father->pg->remove_edge(sname, father->get_hier_name());
        break;
      case dfgNode::SDFG_OPORT: 
        father->pg->remove_edge(father->get_hier_name(), sname);
        break;
      case dfgNode::SDFG_PORT:
        father->pg->remove_edge(sname, father->get_hier_name());
        father->pg->remove_edge(father->get_hier_name(), sname);
        break;
      default:
        assert(0 == "node type wrong!");
      }
    }
    father->port2sig.erase(port.get_hier_name());
  }
}

///////////////////////////////
// other modifications
///////////////////////////////
list<shared_ptr<dfgNode> > SDFG::dfgGraph::flatten() const{
  // flatten all nodes
  map<vertex_descriptor, shared_ptr<dfgNode> > fnodes; // to store all flattened nodes
  list<shared_ptr<dfgNode> > rv;                       // return all new nodes generated

  if(!father) return rv;           // when it is the top, it is impossible to flatten it

  for_each(nodes.begin(), nodes.end(),
           [&](const pair<const vertex_descriptor, shared_ptr<dfgNode> >& m) {
               fnodes[m.first] = m.second->flatten();
               rv.push_back(fnodes[m.first]);
             });

  // duplicate all internal edges
  for_each(edges.begin(), edges.end(),
           [&](const pair<const edge_descriptor, shared_ptr<dfgEdge> >& m) {
             father->pg->add_edge(fnodes[get_source_id(m.first)]->get_hier_name(), 
                                  m.second->type, 
                                  fnodes[get_source_id(m.first)],
                                  fnodes[get_target_id(m.first)]
                                  );
           });

  // connect all ports
  for_each(nodes.begin(), nodes.end(),
           [&](const pair<const vertex_descriptor, shared_ptr<dfgNode> >& m) {
               if(m.second->type & dfgNode::SDFG_IPORT == dfgNode::SDFG_IPORT) {
                 shared_ptr<dfgNode> src = get_in_nodes_cb(m.second).front();
                 father->pg->add_edge(src->get_hier_name(), 
                                      get_in_edges_cb(m.second).front()->type,
                                      src, fnodes[m.first]);
               }
               if(m.second->type == dfgNode::SDFG_OPORT) {
                 shared_ptr<dfgNode> tar = get_out_nodes_cb(m.second).front();
                 father->pg->add_edge(fnodes[m.first]->get_hier_name(), 
                                      get_out_edges_cb(m.second).front()->type,
                                      fnodes[m.first], tar->id);
               }
               if(m.second->type == dfgNode::SDFG_PORT) {
                 // manually find out all the edges and connect them
                 if(father->port2sig.count(m.second->get_hier_name())) {
                   string sname = father->port2sig.find(m.second->get_hier_name())->second;
                   shared_ptr<dfgNode> snode = father->pg->get_node(sname);
                   GraphTraits::out_edge_iterator oit, oend;
                   // output
                   for(boost::tie(oit, oend) = 
                         boost::edge_range(
                                           father->id,
                                           snode->id, 
                                           father->pg->bg_);
                       oit != oend; ++oit) { 
                     father->pg->add_edge(fnodes[m.first]->get_hier_name(), 
                                          father->pg->edges.find(*oit)->second->type,
                                          fnodes[m.first], snode);
                   }
                   // input
                   for(boost::tie(oit, oend) = 
                         boost::edge_range(snode->id,
                                           father->id,
                                           father->pg->bg_);
                       oit != oend; ++oit) { 
                     father->pg->add_edge(snode->get_hier_name(), 
                                          father->pg->edges.find(*oit)->second->type,
                                          snode, fnodes[m.first]);
                   }
                 }
               }             
             });

  return rv;
}


///////////////////////////////
// get nodes and edges
///////////////////////////////
shared_ptr<dfgEdge> SDFG::dfgGraph::get_edge(edge_descriptor eid) const{
  if(edges.count(eid))
    return edges.find(eid)->second;
  else
    return shared_ptr<dfgEdge>();
}

shared_ptr<dfgNode> SDFG::dfgGraph::get_node(vertex_descriptor nid) const{
  if(nodes.count(nid))
    return nodes.find(nid)->second;
  else
    return shared_ptr<dfgNode>();
}

shared_ptr<dfgEdge> SDFG::dfgGraph::get_edge(vertex_descriptor src, vertex_descriptor tar) const{
  if(exist(src, tar))
    return edges.find(boost::edge(get_node(src)->id, get_node(tar)->id, bg_).first)->second;
  else
    return shared_ptr<dfgEdge>();
}

shared_ptr<dfgEdge> SDFG::dfgGraph::get_edge(vertex_descriptor src, vertex_descriptor tar, dfgEdge::edge_type_t tt) const{
  GraphTraits::out_edge_iterator eit, eend;
  for(boost::tie(eit, eend) = boost::edge_range(src, tar, bg_);
      eit != eend; ++eit) { 
    if(get_edge(*eit)->type == tt) return edges.find(*eit)->second;
  }
  return shared_ptr<dfgEdge>();
}

shared_ptr<dfgNode> SDFG::dfgGraph::get_source(edge_descriptor eid) const {
  if(exist(eid))
    return nodes.find(boost::source(eid, bg_))->second;
  else
    return shared_ptr<dfgNode>();
}

shared_ptr<dfgNode> SDFG::dfgGraph::get_source_cb(edge_descriptor eid) const {
  shared_ptr<dfgNode> inode = get_source(eid);
  if(inode->type == dfgNode::SDFG_MODULE && inode->child) {
    std::set<string> plist = inode->sig2port.find(get_target(eid)->get_hier_name())->second;
    if(!plist.empty()) {
      BOOST_FOREACH(const string& m, plist) {
        shared_ptr<dfgNode> n = inode->child->get_node(m);
        if(n->type & dfgNode::SDFG_OPORT && (n->type & dfgNode::SDFG_IPORT) != dfgNode::SDFG_IPORT)
          return n;
      }
      return shared_ptr<dfgNode>();
    }
    return shared_ptr<dfgNode>();
  } else
    return inode;
}

shared_ptr<dfgNode> SDFG::dfgGraph::get_target(edge_descriptor eid) const {
  if(exist(eid))
    return nodes.find(boost::target(eid, bg_))->second;
  else
    return shared_ptr<dfgNode>();
}

list<shared_ptr<dfgNode> > SDFG::dfgGraph::get_target_cb(edge_descriptor eid) const {
  list<shared_ptr<dfgNode> > rv;
  shared_ptr<dfgNode> onode = get_target(eid);
  if(onode->type == dfgNode::SDFG_MODULE && onode->child) {
    std::set<string> plist = onode->sig2port.find(get_source(eid)->get_hier_name())->second;
    if(!plist.empty()) {
      BOOST_FOREACH(const string& m, plist) {
        shared_ptr<dfgNode> n = onode->child->get_node(m);
        if(n->type & dfgNode::SDFG_IPORT && (n->type & dfgNode::SDFG_IPORT) != dfgNode::SDFG_OPORT)
          rv.push_back(n);
      }
    }
  } else
    rv.push_back(onode);

  return rv;
}

vertex_descriptor SDFG::dfgGraph::get_source_id(const edge_descriptor& eid) const {
  if(exist(eid))
    return boost::source(eid, bg_);
  else
    return NULL;
}

vertex_descriptor SDFG::dfgGraph::get_source_id_cb(const edge_descriptor& eid) const {
  shared_ptr<dfgNode> n = get_source_cb(eid);
  if(n)
    return n->id;
  else
    return NULL;
}

vertex_descriptor SDFG::dfgGraph::get_target_id(const edge_descriptor& eid) const {
  if(exist(eid))
    return boost::target(eid, bg_);
  else
    return NULL;
}


///////////////////////////////
// hierarchical search
///////////////////////////////
shared_ptr<dfgNode> SDFG::dfgGraph::search_node(const string& nname) const {
  shared_ptr<dfgNode> dummy(new dfgNode());
  dummy->set_hier_name(nname);

  if(exist(dummy->get_hier_name())) // check node name
    return get_node(dummy->get_hier_name());
  else if(!dummy->hier.empty()) { // check hierarchical names
    string gname = dummy->hier.front();
    dummy->hier.pop_front();
    if(exist(gname) && get_node(gname)->type == dfgNode::SDFG_MODULE && get_node(gname)->child)
      return get_node(gname)->child->search_node(dummy->get_hier_name());
    else
      return shared_ptr<dfgNode>(); // failed, return empty
  } else
    return shared_ptr<dfgNode>(); // failed, return empty
}

///////////////////////////////
// clean up useless nodes
///////////////////////////////
void SDFG::dfgGraph::remove_useless_nodes() {
  //std::cout << "remove useless nodes in " << get_full_name() << std::endl;

  std::set<shared_ptr<dfgNode> > node_set;    // all the nodes to be checked
  std::list<shared_ptr<dfgNode> > node_list;  // the list store the same set 

  // put all nodes into the set
  BOOST_FOREACH(const nodes_type& n, nodes) {
    node_set.insert(n.second);
    node_list.push_back(n.second);
  }
  
  while(!node_set.empty()) {
    // get the ndoe to be checked
    shared_ptr<dfgNode> n = node_list.front();
    node_list.pop_front();
    node_set.erase(n);
    
    if(n->type & dfgNode::SDFG_IPORT == dfgNode::SDFG_IPORT) { // input port
      if((size_out_edges(n, false) == 0) ||          // an input port with no load should be removed
         (father != NULL && 
          (!father->port2sig.count(n->name) ||            // port removed
           !father->port2sig.find(n->name)->second.size() > 0      // port open or const
           )
          )
         ) {
        list<shared_ptr<dfgNode> > onodes = get_out_nodes(n);
        remove_node(n);
        BOOST_FOREACH(shared_ptr<dfgNode> onode, onodes) {
          if(!node_set.count(onode)) {
            node_set.insert(onode);
            node_list.push_back(onode);
          }
        }        
      }
    } else if(n->type == dfgNode::SDFG_OPORT) { // output port
      if((size_in_edges(n) == 0) ||            // an output port with no source should be removed
         (father != NULL && 
          (!father->port2sig.count(n->name) ||                // port removed
           !father->port2sig.find(n->name)->second.size() > 0          // port open or const
           )
          )
         ) {
        list<shared_ptr<dfgNode> > inodes = get_in_nodes(n);
        remove_node(n);
        BOOST_FOREACH(shared_ptr<dfgNode> inode, inodes) {
          if(!node_set.count(inode)) {
            node_set.insert(inode);
            node_list.push_back(inode);
          }
        }
      }
    } else if(n->type == dfgNode::SDFG_PORT) { // inout port
      if((size_out_edges(n) == 0 && size_in_edges(n) == 0) || // an inout port with no source and no load should be removed
         (father != NULL && 
          (!father->port2sig.count(n->name) ||                // port removed
           !father->port2sig.find(n->name)->second.size() > 0          // port open or const
           )
          )
         ) {
        list<shared_ptr<dfgNode> > onodes = get_out_nodes(n);
        list<shared_ptr<dfgNode> > inodes = get_in_nodes(n);
        remove_node(n);
        BOOST_FOREACH(shared_ptr<dfgNode> onode, onodes) {
          if(!node_set.count(onode)) {
            node_set.insert(onode);
            node_list.push_back(onode);
          }
        }        
        BOOST_FOREACH(shared_ptr<dfgNode> inode, inodes) {
          if(!node_set.count(inode)) {
            node_set.insert(inode);
            node_list.push_back(inode);
          }
        }
      }
    } else if(n->type & dfgNode::SDFG_MODULE) { // a module entity
      // process the child module
      list<shared_ptr<dfgNode> > inodes = get_in_nodes(n);
      list<shared_ptr<dfgNode> > onodes = get_out_nodes(n);

      // process the child module
      if(n->remove_useless_ports()) {
        n->child->remove_useless_nodes();
      }
      
      BOOST_FOREACH(shared_ptr<dfgNode> inode, inodes) {
        if(!n->sig2port.count(inode->get_hier_name())) {
          remove_edge(inode, n);
          if(!node_set.count(inode)) {
            node_set.insert(inode);
            node_list.push_back(inode);
          }
        }
      }
      
      BOOST_FOREACH(shared_ptr<dfgNode> onode, onodes) {
        if(!n->sig2port.count(onode->get_hier_name())) {
          remove_edge(n, onode);
          if(!node_set.count(onode)) {
            node_set.insert(onode);
            node_list.push_back(onode);
          }
        }
      }
      
      // remove the module if it is empty
      if(n->sig2port.size() == 0 || n->size_out_edges() == 0) {
        remove_node(n);
        BOOST_FOREACH(shared_ptr<dfgNode> inode, inodes) {
          if(!node_set.count(inode)) {
            node_set.insert(inode);
            node_list.push_back(inode);
          }
        }
        BOOST_FOREACH(shared_ptr<dfgNode> onode, onodes) {
          if(!node_set.count(onode)) {
            node_set.insert(onode);
            node_list.push_back(onode);
          }
        }
      }
    
    } else { // all other cases
      if(size_out_edges(n, false) == 0) {
        // a node with no source or no load is useless
        list<shared_ptr<dfgNode> > inodes = get_in_nodes(n, false);
        remove_node(n);
        BOOST_FOREACH(shared_ptr<dfgNode> inode, inodes) {
          if(!node_set.count(inode)) {
            node_set.insert(inode);
            node_list.push_back(inode);
          }
        }
      } else if(!(n->type & (dfgNode::SDFG_FF|dfgNode::SDFG_LATCH)) && 
                size_in_edges(n, false) == 0) {
        // a combi node with no source
        list<shared_ptr<dfgNode> > onodes = get_out_nodes(n, false);
        remove_node(n);
        BOOST_FOREACH(shared_ptr<dfgNode> onode, onodes) {
          if(!node_set.count(onode)) {
            node_set.insert(onode);
            node_list.push_back(onode);
          }
        }
      }
    }
  }
}

void SDFG::dfgGraph::remove_unlisted_nodes(const std::set<shared_ptr<dfgNode> >& nlist, bool hier) {
  list<shared_ptr<dfgNode> > nlook_list;
  BOOST_FOREACH(nodes_type npair, nodes) {
    nlook_list.push_back(npair.second);
  }

  BOOST_FOREACH(shared_ptr<dfgNode> node, nlook_list) {
    if((node->type & (dfgNode::SDFG_FF|dfgNode::SDFG_LATCH)) ||
       ((father == NULL) && (node->type & dfgNode::SDFG_PORT)) ) {
      if(!nlist.count(node))
        remove_node(node);
    } else if(hier && (node->type & dfgNode::SDFG_MODULE) && node->child)
      node->child->remove_unlisted_nodes(nlist, hier);
  }
}

///////////////////////////////
// existance check
///////////////////////////////
bool SDFG::dfgGraph::exist(vertex_descriptor src, vertex_descriptor tar) const {
  return boost::edge(src, tar, bg_).second;
}

bool SDFG::dfgGraph::exist(vertex_descriptor src, vertex_descriptor tar, dfgEdge::edge_type_t tt) const {
  GraphTraits::out_edge_iterator eit, eend;
  for(boost::tie(eit, eend) = edge_range(src, tar, bg_);
      eit != eend; ++eit) { 
    if(get_edge(*eit)->type == tt) return true;
  }
  return false;
}

bool SDFG::dfgGraph::exist(edge_descriptor eid) const {
  return edges.count(eid);
}

bool SDFG::dfgGraph::exist(vertex_descriptor nid) const {
  return nodes.count(nid);
}    


///////////////////////////////
// traverse
///////////////////////////////
unsigned int SDFG::dfgGraph::size_out_edges(vertex_descriptor nid, bool bself) const {
  if(nodes.count(nid)) {
    if(bself)
      return boost::out_degree(nid, bg_);
    else {
      unsigned int rv = 0;
      GraphTraits::out_edge_iterator eit, eend;
      for(boost::tie(eit, eend) = boost::out_edges(nid, bg_);
          eit != eend; ++eit) {
        if(boost::target(*eit, bg_) != nid) rv++;
      }
      return rv;
    }
  } else
    return 0;
}

unsigned int SDFG::dfgGraph::size_out_edges_cb(vertex_descriptor nid, bool bself) const {
  if(nodes.count(nid)) {
    shared_ptr<dfgNode> pn = nodes.find(nid)->second;
    if(pn->type == dfgNode::SDFG_OPORT) { // output port
      if(father && father->port2sig.count(pn->get_hier_name()))
        return 1;
      else
        return 0;
    } else if(pn->type == dfgNode::SDFG_PORT) { // I/O port
      if(father && father->port2sig.count(pn->get_hier_name()))
        return 1 + size_out_edges(nid, bself);
      else
        return size_out_edges(nid, bself);
    } else
      return size_out_edges(nid, bself);
  } else
    return 0;
}

unsigned int SDFG::dfgGraph::size_in_edges(vertex_descriptor nid, bool bself) const {
  if(nodes.count(nid)) {
    if(bself)
      return boost::in_degree(nid, bg_);
    else {
      unsigned int rv = 0;
      GraphTraits::in_edge_iterator eit, eend;
      for(boost::tie(eit, eend) = boost::in_edges(nid, bg_);
          eit != eend; ++eit) {
        if(boost::source(*eit, bg_) != nid) rv++;
      }
      return rv;
    }
  } else
    return 0;
}

unsigned int SDFG::dfgGraph::size_in_edges_cb(vertex_descriptor nid, bool bself) const {
  if(nodes.count(nid)) {
    shared_ptr<dfgNode> pn = nodes.find(nid)->second;
    if(pn->type & dfgNode::SDFG_IPORT == dfgNode::SDFG_IPORT) { // input port
      if(father && father->port2sig.count(pn->get_hier_name()))
        return 1;
      else
        return 0;
    } else if(pn->type == dfgNode::SDFG_PORT) { // I/O port
      if(father && father->port2sig.count(pn->get_hier_name()))
        return 1 + size_in_edges(nid, bself);
      else
        return size_in_edges(nid, bself);
    } else
      return size_in_edges(nid, bself);
  } else
    return 0;
}

list<shared_ptr<dfgNode> > SDFG::dfgGraph::get_out_nodes(vertex_descriptor nid, bool bself) const {
  list<shared_ptr<dfgNode> > rv;
  GraphTraits::adjacency_iterator nit, nend;
  for(boost::tie(nit, nend) = boost::adjacent_vertices(nid, bg_);
      nit != nend; ++nit) {
    shared_ptr<dfgNode> mn = nodes.find(*nit)->second;
    if((mn->id != nid) || bself)
      rv.push_back(nodes.find(*nit)->second);
  }
  return rv;
}

list<shared_ptr<dfgNode> > SDFG::dfgGraph::get_out_nodes_cb(vertex_descriptor nid, bool bself) const {
  list<shared_ptr<dfgNode> > rv;
  if(!nodes.count(nid)) return rv;
  shared_ptr<dfgNode> pn = nodes.find(nid)->second;
  if(pn->type == dfgNode::SDFG_OPORT || pn->type == dfgNode::SDFG_PORT) { // output or I/O port
    if(father && father->port2sig.count(pn->get_hier_name())) {
      if(father->port2sig.find(pn->get_hier_name())->second.size() > 0)
        rv.push_back(father->pg->get_node(father->port2sig.find(pn->get_hier_name())->second));
    }
  } 

  if(pn->type != dfgNode::SDFG_OPORT) { // have internal outputs
    GraphTraits::adjacency_iterator nit, nend;
    for(boost::tie(nit, nend) = boost::adjacent_vertices(nid, bg_);
        nit != nend; ++nit) {
      shared_ptr<dfgNode> osrc = nodes.find(*nit)->second;
      if(osrc->type == dfgNode::SDFG_MODULE) { // it is a module entity, go further
        BOOST_FOREACH(const string& sname, osrc->sig2port.find(pn->name)->second) {
          rv.push_back(osrc->child->get_node(sname));
        }
      } else if((osrc->id != nid) || bself) {
        rv.push_back(osrc);
      }
    }
  }
  return rv;
}

list<shared_ptr<dfgNode> > SDFG::dfgGraph::get_in_nodes(vertex_descriptor nid, bool bself) const {
  list<shared_ptr<dfgNode> > rv;
  GType::inv_adjacency_iterator nit, nend; //!! why inv_adjacency_iterator is in Graph instead of Trait?
  for(boost::tie(nit, nend) = boost::inv_adjacent_vertices(nid, bg_);
      nit != nend; ++nit) {
    shared_ptr<dfgNode> mn = nodes.find(*nit)->second;
    if((mn->id != nid) || bself)
      rv.push_back(nodes.find(*nit)->second);
  }
  return rv;
}

list<shared_ptr<dfgNode> > SDFG::dfgGraph::get_in_nodes_cb(vertex_descriptor nid, bool bself) const {
  list<shared_ptr<dfgNode> > rv;
  if(!nodes.count(nid)) return rv;
  shared_ptr<dfgNode> pn = nodes.find(nid)->second;
  if(pn->type & dfgNode::SDFG_IPORT == dfgNode::SDFG_IPORT 
     || pn->type == dfgNode::SDFG_PORT) { // input or I/O port
    if(father && father->port2sig.count(pn->get_hier_name())) {
      if(father->port2sig.find(pn->get_hier_name())->second.size() > 0)
        rv.push_back(father->pg->get_node(father->port2sig.find(pn->get_hier_name())->second));
    }
  } 
  
  if(pn->type & dfgNode::SDFG_IPORT != dfgNode::SDFG_IPORT) { // have internal inputs
    GType::inv_adjacency_iterator nit, nend; //!! why inv_adjacency_iterator is in Graph instead of Trait?
    for(boost::tie(nit, nend) = boost::inv_adjacent_vertices(nid, bg_);
        nit != nend; ++nit) {
      shared_ptr<dfgNode> isrc = nodes.find(*nit)->second;
      if(isrc->type == dfgNode::SDFG_MODULE) { // it is a module entity, go further
        BOOST_FOREACH(const string& sname, isrc->sig2port.find(pn->name)->second) {
          rv.push_back(isrc->child->get_node(sname));
        }
      } else if((isrc->id != nid) || bself) {
        rv.push_back(isrc);
      }
    }
  }

  return rv;
}

list<shared_ptr<dfgEdge> > SDFG::dfgGraph::get_out_edges(vertex_descriptor nid, bool bself) const {
  list<shared_ptr<dfgEdge> > rv;
  if(nodes.count(nid)) {
    GraphTraits::out_edge_iterator eit, eend;
    for(boost::tie(eit, eend) = boost::out_edges(nid, bg_);
        eit != eend; ++eit) {
      shared_ptr<dfgEdge> e = edges.find(*eit)->second;
      if(e->get_target_id() != nid || bself)
        rv.push_back(e);
    }
  }
  return rv;
}

list<shared_ptr<dfgEdge> > SDFG::dfgGraph::get_out_edges_cb(vertex_descriptor nid, bool bself) const {
  list<shared_ptr<dfgEdge> > rv;
  if(nodes.count(nid)) {
    shared_ptr<dfgNode> pn = nodes.find(nid)->second;
    if(pn->type == dfgNode::SDFG_OPORT || pn->type == dfgNode::SDFG_PORT) { // output or I/O port
      if(father && father->port2sig.count(pn->get_hier_name())) {
        string tar_name = father->port2sig.find(pn->get_hier_name())->second;
        if(tar_name.size() > 0) {
          GraphTraits::out_edge_iterator eit, eend;
          for(boost::tie(eit, eend) = 
                boost::edge_range(
                                  father->id, 
                                  father->pg->node_map.find(tar_name)->second, 
                                  father->pg->bg_);
              eit != eend; ++eit) { 
            rv.push_back(father->pg->edges.find(*eit)->second);
          }
        }
      }
    } 

    if(pn->type != dfgNode::SDFG_OPORT) { // have internal outputs
      GraphTraits::out_edge_iterator eit, eend;
      for(boost::tie(eit, eend) = boost::out_edges(nid, bg_);
          eit != eend; ++eit) {
        shared_ptr<dfgEdge> e = edges.find(*eit)->second;
        if(e->get_target_id() != nid || bself)
          rv.push_back(e);
      }
    }
  }
  return rv;
}

list<shared_ptr<dfgEdge> > SDFG::dfgGraph::get_in_edges(vertex_descriptor nid, bool bself) const {
  list<shared_ptr<dfgEdge> > rv;
  if(nodes.count(nid)) {
    GraphTraits::in_edge_iterator eit, eend;
    for(boost::tie(eit, eend) = boost::in_edges(nid, bg_);
        eit != eend; ++eit) {
      shared_ptr<dfgEdge> e = edges.find(*eit)->second;
      if(e->get_source_id() != nid || bself)
        rv.push_back(e);
    }
  }
  return rv;
}

list<shared_ptr<dfgEdge> > SDFG::dfgGraph::get_in_edges_cb(vertex_descriptor nid, bool bself) const {
  list<shared_ptr<dfgEdge> > rv;
  if(nodes.count(nid)) {
    shared_ptr<dfgNode> pn = nodes.find(nid)->second;
    if(pn->type & dfgNode::SDFG_IPORT == dfgNode::SDFG_IPORT || pn->type == dfgNode::SDFG_PORT) { // input or I/O port
      if(pn && father && father->port2sig.count(pn->get_hier_name())) {
        string tar_name = father->port2sig.find(pn->get_hier_name())->second;
        if(tar_name.size() > 0) {
          GraphTraits::out_edge_iterator eit, eend;
          for(boost::tie(eit, eend) = 
                boost::edge_range(
                                  father->pg->node_map.find(tar_name)->second, 
                                father->id, 
                                  father->pg->bg_);
              eit != eend; ++eit) { 
            rv.push_back(father->pg->edges.find(*eit)->second);
          }
        }
      }
    } 

    if(pn->type & dfgNode::SDFG_IPORT != dfgNode::SDFG_IPORT) { // have internal outputs
      GraphTraits::in_edge_iterator eit, eend;
      for(boost::tie(eit, eend) = boost::in_edges(nid, bg_);
          eit != eend; ++eit) {
        shared_ptr<dfgEdge> e = edges.find(*eit)->second;
        if(e->get_source_id() != nid || bself)
          rv.push_back(e);
      }
    }
  }
  return rv;
}

int SDFG::dfgGraph::get_out_edges_type(vertex_descriptor nid, bool bself) const {
  int rv = 0;
  BOOST_FOREACH(shared_ptr<dfgEdge> e, get_out_edges(nid)) {
    if(e->get_target_id() != nid || bself)
      rv |= e->type;
  }
  return rv;
}

int SDFG::dfgGraph::get_out_edges_type_cb(vertex_descriptor nid, bool bself) const {
  int rv = 0;
  BOOST_FOREACH(shared_ptr<dfgEdge> e, get_out_edges_cb(nid)) {
    if(bself)
      rv |= e->type;
    else {
      shared_ptr<dfgNode> n = e->get_target();
      if(n->type & dfgNode::SDFG_MODULE || n->id != nid)
        rv |= e->type;        
    }
  }
  return rv;
}

int SDFG::dfgGraph::get_in_edges_type(vertex_descriptor nid, bool bself) const {
  int rv = 0;
  BOOST_FOREACH(shared_ptr<dfgEdge> e, get_in_edges(nid)) {
    if(e->get_source_id() != nid || bself)
      rv |= e->type;
  }
  return rv;
}

int SDFG::dfgGraph::get_in_edges_type_cb(vertex_descriptor nid, bool bself) const {
  int rv = 0;
  BOOST_FOREACH(shared_ptr<dfgEdge> e, get_in_edges_cb(nid)) {
    if(e->get_source_id_cb() != nid || bself)
      rv |= e->type;
  }
  return rv;
}

///////////////////////////////
// graphic property
///////////////////////////////
unsigned int SDFG::dfgGraph::size_of_nodes(bool hier) const {
  unsigned int cnt = 0;
  if(hier) {
    BOOST_FOREACH(shared_ptr<dfgNode> m, get_list_of_nodes(dfgNode::SDFG_MODULE)) {
      if(m->child) cnt += m->child->size_of_nodes(true);
    }
  }
  return cnt + nodes.size();
}

unsigned int SDFG::dfgGraph::size_of_regs(bool hier) const {
  unsigned int cnt = 0;
  if(hier) {
    BOOST_FOREACH(shared_ptr<dfgNode> m, get_list_of_nodes(dfgNode::SDFG_MODULE)) {
      if(m->child) cnt += m->child->size_of_regs(true);
    }
  }
  return cnt + get_list_of_nodes(dfgNode::SDFG_FF|dfgNode::SDFG_LATCH).size();
}

unsigned int SDFG::dfgGraph::size_of_combs(bool hier) const {
  unsigned int cnt = 0;
  if(hier) {
    BOOST_FOREACH(shared_ptr<dfgNode> m, get_list_of_nodes(dfgNode::SDFG_MODULE)) {
      if(m->child) cnt += m->child->size_of_combs(true);
    }
  }
  return cnt 
    + nodes.size() 
    - get_list_of_nodes(dfgNode::SDFG_MODULE|dfgNode::SDFG_FF|dfgNode::SDFG_LATCH).size();
}

unsigned int SDFG::dfgGraph::size_of_modules(bool hier) const {
  unsigned int cnt = 0;
  if(hier) {
    BOOST_FOREACH(shared_ptr<dfgNode> m, get_list_of_nodes(dfgNode::SDFG_MODULE)) {
      if(m->child) cnt += m->child->size_of_modules(true);
    }
  }
  return cnt + get_list_of_nodes(dfgNode::SDFG_MODULE).size();
}


///////////////////////////////
// graphic
///////////////////////////////
bool SDFG::dfgGraph::layout() {
  ogdf::Graph g;
  ogdf::GraphAttributes ga;
  write(&g, &ga);
  return layout(&g, &ga);
}

bool SDFG::dfgGraph::layout(ogdf::Graph* pg, ogdf::GraphAttributes *pga) {
  // Sugiyama Layout
  ogdf::SugiyamaLayout SL;

  //ogdf::LongestPathRanking *ranking = new ogdf::LongestPathRanking();
  ogdf::OptimalRanking * ranking = new ogdf::OptimalRanking();
  //ogdf::CoffmanGrahamRanking *ranking = new ogdf::CoffmanGrahamRanking();
  //ranking->width(4);
  //ogdf::GreedyCycleRemoval * subGrapher = new ogdf::GreedyCycleRemoval();
  //ranking->setSubgraph(subGrapher);
  SL.setRanking(ranking);

  //ogdf::SplitHeuristic *crossMiner = new ogdf::SplitHeuristic();
  //ogdf::MedianHeuristic *crossMiner = new ogdf::MedianHeuristic();
  //ogdf::GreedyInsertHeuristic *crossMiner = new ogdf::GreedyInsertHeuristic();
  //SL.setCrossMin(crossMiner);

  ogdf::FastHierarchyLayout * layouter = new ogdf::FastHierarchyLayout();
  layouter->layerDistance(G_NODE_H * G_LAYER_DIST);
  layouter->nodeDistance(G_NODE_H * G_NODE_DIST);
  SL.setLayout(layouter);

  SL.runs(10);

  try {
    SL.call(*pga);
  } catch(std::exception e) {
    std::cout << string(__PRETTY_FUNCTION__) 
              << ": layout engine SugiyamaLayout failed."
              << std::endl;
    return false;
  }

  if(!read(pg, pga)) return false;

 
  // special operations for self loops
  for_each(edges.begin(), edges.end(),
           [&](pair<const edge_descriptor, shared_ptr<dfgEdge> >& m) {
               if(boost::source(m.second->id, bg_) == boost::target(m.second->id, bg_)) { // self loop
                 shared_ptr<dfgNode> node = get_source(m.second);
                 double base_len = G_NODE_H*G_NODE_DIST*0.8;
                 switch(m.second->type) {
                 case dfgEdge::SDFG_DDP:
                   m.second->push_bend(-base_len*cos(d2r(30.0)), -base_len*sin(d2r(30.0)), true);
                   m.second->push_bend(-base_len*cos(d2r(85.0)), -base_len*sin(d2r(85.0)), true);
                   break;                   
                 case dfgEdge::SDFG_CAL:
                   m.second->push_bend(-base_len*cos(d2r(20.0)), -base_len*sin(d2r(20.0)), true);
                   m.second->push_bend(-base_len*cos(d2r(80.0)), -base_len*sin(d2r(80.0)), true);
                   break;                   
                 case dfgEdge::SDFG_ASS:
                   m.second->push_bend(-base_len*cos(d2r(10.0)), -base_len*sin(d2r(10.0)), true);
                   m.second->push_bend(-base_len*cos(d2r(70.0)), -base_len*sin(d2r(70.0)), true);
                   break;                   
                 case dfgEdge::SDFG_DAT:
                   m.second->push_bend(-base_len*cos(d2r(00.0)), -base_len*sin(d2r(00.0)), true);
                   m.second->push_bend(-base_len*cos(d2r(60.0)), -base_len*sin(d2r(60.0)), true);
                   break;
                 case dfgEdge::SDFG_CTL:
                   m.second->push_bend(base_len*cos(d2r(00.0)),  -base_len*sin(d2r(00.0)), true);
                   m.second->push_bend(base_len*cos(d2r(60.0)),  -base_len*sin(d2r(60.0)), true);
                   break;
                 case dfgEdge::SDFG_CMP:
                   m.second->push_bend(base_len*cos(d2r(10.0)),  -base_len*sin(d2r(10.0)), true);
                   m.second->push_bend(base_len*cos(d2r(70.0)),  -base_len*sin(d2r(70.0)), true);
                   break;
                 case dfgEdge::SDFG_EQU:
                   m.second->push_bend(base_len*cos(d2r(20.0)),  -base_len*sin(d2r(20.0)), true);
                   m.second->push_bend(base_len*cos(d2r(80.0)),  -base_len*sin(d2r(80.0)), true);
                   break;
                 case dfgEdge::SDFG_LOG:
                   m.second->push_bend(base_len*cos(d2r(30.0)),  -base_len*sin(d2r(30.0)), true);
                   m.second->push_bend(base_len*cos(d2r(90.0)),  -base_len*sin(d2r(90.0)), true);
                   break;
                 case dfgEdge::SDFG_ADR:
                   m.second->push_bend(base_len*cos(d2r(40.0)),  -base_len*sin(d2r(40.0)), true);
                   m.second->push_bend(base_len*cos(d2r(100.0)),  -base_len*sin(d2r(100.0)), true);
                   break;
                 default:
                   m.second->push_bend(base_len*cos(d2r(00.0)),   base_len*sin(d2r(00.0)), true);
                   m.second->push_bend(base_len*cos(d2r(60.0)),   base_len*sin(d2r(60.0)), true);
                 }
               }
             });
  
  return true;
}


///////////////////////////////
// graphic formats
///////////////////////////////
void SDFG::dfgGraph::write(std::ostream& os) const {
  pugi::xml_document sdfg_file;       // using the pugixml library
  pugi::xml_node node_xml = sdfg_file.append_child(pugi::node_declaration);
  node_xml.append_attribute("version") = "1.0";
  node_xml.append_attribute("encoding") = "UTF-8";

  list<shared_ptr<dfgGraph> > sub_graph; // a list to record all sub graphs
  std::set<string> sub_graph_set;        // a set to record all sub_graph written out

  // set current graph as the top module
  pugi::xml_node top = sdfg_file.append_child("top");
  top.append_attribute("name") = name.c_str();

  // write out the current graph
  pugi::xml_node xnode = sdfg_file.append_child("graph");
  write(xnode, sub_graph);
  sub_graph_set.insert(name);

  // write out sub modules
  while(!sub_graph.empty()) {
    if(!sub_graph_set.count(sub_graph.front()->name)) {
      // a new graph has not been written yet
      xnode = sdfg_file.append_child("graph");
      sub_graph.front()->write(xnode, sub_graph); // write it out
      sub_graph_set.insert(sub_graph.front()->name); // record it
    }
    sub_graph.pop_front();      // remove it
  }

  // save the file
  sdfg_file.save(os);

}

void SDFG::dfgGraph::write(pugi::xml_node& xnode, std::list<boost::shared_ptr<dfgGraph> >& GList) const {
  xnode.append_attribute("name") = name.c_str();
  
  // write all nodes to the graph
  for_each(index_map.begin(), index_map.end(), 
           [&](const pair<const unsigned int, const vertex_descriptor>& m) {
               pugi::xml_node node = xnode.append_child("node");
               node.append_attribute("id") = nodes.find(m.second)->second->node_index;
               nodes.find(m.second)->second->write(node, GList);
             });
           
  // write all edges to the graph
  for_each(edge_map.begin(), edge_map.end(), 
           [&](const pair<const unsigned int, edge_descriptor>& m) {
               pugi::xml_node node = xnode.append_child("edge");
               node.append_attribute("source") = get_source(m.second)->node_index;
               node.append_attribute("target") = get_target(m.second)->node_index;
               edges.find(m.second)->second->write(node);
             });
}

void SDFG::dfgGraph::write(ogdf::Graph *pg, ogdf::GraphAttributes *pga){
  pga->init(*pg,
            ogdf::GraphAttributes::nodeGraphics | ogdf::GraphAttributes::edgeGraphics |
            ogdf::GraphAttributes::nodeLabel    | ogdf::GraphAttributes::edgeStyle    | 
            ogdf::GraphAttributes::nodeStyle    | ogdf::GraphAttributes::edgeArrow    |
            ogdf::GraphAttributes::nodeWeight   | ogdf::GraphAttributes::edgeLabel    );
  
  map<vertex_descriptor, void *> i2n;

  // nodes
  for_each(nodes.begin(), nodes.end(),
           [&](const pair<const vertex_descriptor, shared_ptr<dfgNode> >& m) {
               ogdf::node pnode = pg->newNode();
               i2n[m.first] = pnode;
               m.second->write(pnode, pga);
             });

  // edges
  for_each(edges.begin(), edges.end(),
           [&](const pair<const edge_descriptor, shared_ptr<dfgEdge> >& m) {
               // get the source and target node pointers
               ogdf::node ps = static_cast<ogdf::node>(i2n[boost::source(m.first, bg_)]);
               ogdf::node pt = static_cast<ogdf::node>(i2n[boost::target(m.first, bg_)]);
               ogdf::edge pedge = pg->newEdge(ps, pt);
               m.second->write(pedge, pga);
             });
}

bool SDFG::dfgGraph::read(const pugi::xml_node& xnode) {
  // get the graph name
  name = xnode.attribute("name").as_string();

  map<unsigned int, vertex_descriptor> id_map; // need to regenerate vertex_descriptor in BGL

  // get all nodes
  for(pugi::xml_node node = xnode.child("node"); node; node = node.next_sibling("node")) {
    shared_ptr<dfgNode> pnode(new dfgNode());
    if(!pnode->read(node)) return false;
    unsigned int orig = pnode->node_index;
    add_node(pnode);
    id_map[orig] = pnode->id;
  }

  // get all edges
  for(pugi::xml_node edge = xnode.child("edge"); edge; edge = edge.next_sibling("edge")) {
    shared_ptr<dfgEdge> pedge(new dfgEdge());
    if(!pedge->read(edge)) return false;
    unsigned int src, tar;
    src = edge.attribute("source").as_uint();
    tar = edge.attribute("target").as_uint();
    add_edge(pedge,id_map[src], id_map[tar]);
  }
  
  return true;
}

bool SDFG::dfgGraph::read(ogdf::Graph * const pg, ogdf::GraphAttributes * const pga){
  ogdf::node n;
  forall_nodes(n, *pg) {
    unsigned int nid = boost::lexical_cast<unsigned int>(pga->labelNode(n).cstr());
    if(!index_map.count(nid)) return false;
    nodes[index_map[nid]]->read(n, pga);
  }

  ogdf::edge e;
  forall_edges(e, *pg) {
    unsigned int src, tar;
    src = boost::lexical_cast<unsigned int>(pga->labelNode(e->source()).cstr());
    tar = boost::lexical_cast<unsigned int>(pga->labelNode(e->target()).cstr());
    dfgEdge::edge_type_t etype = static_cast<dfgEdge::edge_type_t>
      (boost::lexical_cast<unsigned int>(pga->labelEdge(e).cstr()));
    if(!exist(index_map[src], index_map[tar], etype)) return false;
    get_edge(index_map[src], index_map[tar], etype)->read(e, pga);
  }

  return true;
}

string SDFG::dfgGraph::get_full_name() const {
  if(father) 
    return father->get_full_name();
  else
    return "";
}

bool SDFG::dfgGraph::check_integrity() const {
  // check all nodes
  BOOST_FOREACH(index_map_type index, index_map) {
    assert(nodes.count(index.second));
    assert(nodes.find(index.second)->second->node_index == index.first);
  }
  
  BOOST_FOREACH(node_map_type nm, node_map) {
    assert(nodes.count(nm.second));
    assert(nodes.find(nm.second)->second->get_hier_name() == nm.first);
  }
  
  BOOST_FOREACH(nodes_type n, nodes) {
    shared_ptr<dfgNode> pn = n.second;
    assert(pn);
    assert(pn->id == n.first);
    assert(index_map.count(pn->node_index));
    assert(node_map.count(pn->get_hier_name()));
    assert(pn->pg == this);
    assert(pn->check_integrity());
  }

  // check all edges
  BOOST_FOREACH(edge_map_type em, edge_map) {
    assert(edges.count(em.second));
    assert(edges.find(em.second)->second->edge_index == em.first);   
  }

  BOOST_FOREACH(edges_type e, edges) {
    shared_ptr<dfgEdge> pe = e.second;
    assert(pe);
    assert(pe->id == e.first);
    assert(edge_map.count(pe->edge_index));
    assert(pe->pg == this);
    assert(pe->check_integrity());
  }
  
  return true;
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


/////////////////////////////////////////////////////////////////////////////
/********        global function                                    ********/
/////////////////////////////////////////////////////////////////////////////

shared_ptr<dfgGraph> SDFG::read(std::istream& istr) {
  shared_ptr<dfgGraph> G(new dfgGraph());
  pugi::xml_document doc;       // the sdfg XML file
  pugi::xml_parse_result rv = doc.load(istr, 
                                       pugi::parse_default     |
                                       pugi::parse_declaration |   // declaration
                                       pugi::parse_comments        // commetns
                                       );
  if(!rv) {
    std::cout << string(__PRETTY_FUNCTION__) << " error reading the SDFG XML file!" << std::endl;
    return G;
  } 

  map<string, shared_ptr<dfgGraph> > gmap;
  G->name = doc.child("top").attribute("name").as_string();

  for(pugi::xml_node xgraph = doc.child("graph"); xgraph; xgraph = xgraph.next_sibling("graph")) {
    shared_ptr<dfgGraph> pg(new dfgGraph());
    if(!pg->read(xgraph)) return G;
    gmap[pg->name] = pg;
  }

  // link all module node
  for_each(gmap.begin(), gmap.end(),
           [&](pair<const string, shared_ptr<dfgGraph> >&m) {
             for_each(m.second->nodes.begin(), m.second->nodes.end(),
                      [&](pair<const vertex_descriptor, shared_ptr<dfgNode> > &n) {
                        if(n.second->type == dfgNode::SDFG_MODULE)
                          n.second->child = gmap[n.second->child_name];
                      });
           });

  // set the top graph
  G = gmap[G->name];

  return G;
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
