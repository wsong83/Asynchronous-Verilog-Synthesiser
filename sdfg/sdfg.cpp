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
 * A help library of SDFG
 * 17/09/2012   Wei Song
 *
 *
 */

#include "sdfg.hpp"
#include <boost/tuple/tuple.hpp>

using namespace SDFG;
using boost::shared_ptr;
using std::map;
using std::string;
using std::pair;

void SDFG::dfgGraph::add_node(shared_ptr<dfgNode> node) {
  assert(node);
  node->id = boost::add_vertex(bg_);
  nodes[node->id] = node;
  node_map[node->name] = node->id;
  if(node->type & dfgNode::SDFG_PORT)
    port_map[node->name] = node->id;
}

shared_ptr<dfgNode> SDFG::dfgGraph::add_node(const string& n, dfgNode::node_type_t t) {
  shared_ptr<dfgNode> node(new dfgNode(n, t));
  add_node(node);
  return node;
}

void SDFG::dfgGraph::add_edge(shared_ptr<dfgEdge> edge, const string& src, const string& snk) {
  assert(edge);
  assert(node_map.count(src) && node_map.count(snk));
  bool added;
  boost::tie(edge->id, added) = boost::add_edge(node_map[src], node_map[snk], bg_);
  edges[edge->id] = edge;
}

shared_ptr<dfgEdge> SDFG::dfgGraph::add_edge(const string& n, dfgEdge::edge_type_t t, const string& src, const string& snk) {
  shared_ptr<dfgEdge> edge(new dfgEdge(n, t));
  add_edge(edge, src, snk);
  return edge;
}

shared_ptr<dfgEdge> SDFG::dfgGraph::get_edge(edge_descriptor eid) {
  assert(edges.count(eid));
  return edges[eid];
}

shared_ptr<dfgEdge> SDFG::dfgGraph::get_edge(edge_descriptor eid) const{
  assert(edges.count(eid));
  return edges.find(eid)->second;
}

shared_ptr<dfgNode> SDFG::dfgGraph::get_node(vertex_descriptor nid) {
  assert(nodes.count(nid));
  return nodes[nid];
}

shared_ptr<dfgNode> SDFG::dfgGraph::get_node(vertex_descriptor nid) const{
  assert(nodes.count(nid));
  return nodes.find(nid)->second;
}

shared_ptr<dfgNode> SDFG::dfgGraph::get_node(const string& n) {
  assert(node_map.count(n));
  return nodes[node_map[n]];
}

shared_ptr<dfgNode> SDFG::dfgGraph::get_node(const string& n) const{
  assert(node_map.count(n));
  return nodes.find(node_map.find(n)->second)->second;
}


