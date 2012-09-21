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
#include <set>
#include <algorithm>

using namespace SDFG;
using boost::shared_ptr;
using std::map;
using std::string;
using std::pair;
using std::list;

void SDFG::dfgNode::write(pugi::xml_node& xnode, std::list<boost::shared_ptr<dfgGraph> >& GList) const {
  xnode.append_attribute("name") = name.c_str();
  string stype;
  switch(type) {
  case SDFG_COMB:    stype = "combi";   break;
  case SDFG_FF:      stype = "ff";      break;
  case SDFG_LATCH:   stype = "latch";   break;
  case SDFG_MODULE:  stype = "module";  break;
  case SDFG_IPORT:   stype = "iport";   break;
  case SDFG_OPORT:   stype = "oport";   break;
  case SDFG_PORT:    stype = "port";    break;
  default:           stype = "unknown";
  }    
  xnode.append_attribute("type") = stype.c_str();
  if(type == SDFG_MODULE) {     // module
    if(child)  GList.push_back(child); // push the sub-module to the module list
    for_each(port2sig.begin(), port2sig.end(), 
             [&](const pair<const string, const string>& m) {
               pugi::xml_node port = xnode.append_child("portmap");
               port.append_attribute("port") = m.first.c_str();
               port.append_attribute("signal") = m.second.c_str();
             });
  }
}

void SDFG::dfgEdge::write(pugi::xml_node& xnode) const {
  xnode.append_attribute("name") = name.c_str();
  string stype;
  switch(type) {
  case SDFG_DP:     stype = "data";     break;
  case SDFG_CTL:    stype = "control";  break;
  case SDFG_CLK:    stype = "clk";      break;
  case SDFG_RST:    stype = "rst";      break;
  default:          stype = "unknown";
  }
  xnode.append_attribute("type") = stype.c_str();
}

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

shared_ptr<dfgEdge> SDFG::dfgGraph::get_edge(const edge_descriptor& eid) const{
  if(edges.count(eid))
    return edges.find(eid)->second;
  else
    return shared_ptr<dfgEdge>();
}

shared_ptr<dfgNode> SDFG::dfgGraph::get_node(const vertex_descriptor& nid) const{
  if(nodes.count(nid))
    return nodes.find(nid)->second;
  else
    return shared_ptr<dfgNode>();
}

shared_ptr<dfgNode> SDFG::dfgGraph::get_node(const string& n) const{
  if(node_map.count(n))
    return nodes.find(node_map.find(n)->second)->second;
  else
    return shared_ptr<dfgNode>();
}

shared_ptr<dfgEdge> SDFG::dfgGraph::get_edge(const vertex_descriptor& src, const vertex_descriptor& tar) const{
  if(exist(src, tar))
    return edges.find(boost::edge(get_node(src)->id, get_node(tar)->id, bg_).first)->second;
  else
    return shared_ptr<dfgEdge>();
}

shared_ptr<dfgEdge> SDFG::dfgGraph::get_edge(const string& src, const string& tar) const{
  if(exist(src) && exist(tar))
    return get_edge(node_map.find(src)->second, node_map.find(tar)->second);
  else
    return shared_ptr<dfgEdge>();
}

shared_ptr<dfgEdge> SDFG::dfgGraph::get_edge(const vertex_descriptor& src, const vertex_descriptor& tar, dfgEdge::edge_type_t tt) const{
  GraphTraits::out_edge_iterator eit, eend;
  for(boost::tie(eit, eend) = edge_range(src, tar, bg_);
      eit != eend; ++eit) { 
    if(get_edge(*eit)->type == tt) return edges.find(*eit)->second;
  }
  return shared_ptr<dfgEdge>();
}

shared_ptr<dfgEdge> SDFG::dfgGraph::get_edge(const string& src, const string& tar, dfgEdge::edge_type_t tt) const{
  if(exist(src) && exist(tar))
    return get_edge(node_map.find(src)->second, node_map.find(tar)->second, tt);
  else
    return shared_ptr<dfgEdge>();
}

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
  for_each(nodes.begin(), nodes.end(), 
           [&](const pair<const vertex_descriptor, shared_ptr<dfgNode> >& m) {
               pugi::xml_node node = xnode.append_child("node");
               node.append_attribute("id") = static_cast<int>(m.first);
               m.second->write(node, GList);
             });
           
  // write all edges to the graph
  for_each(edges.begin(), edges.end(), 
           [&](const pair<const edge_descriptor, shared_ptr<dfgEdge> >& m) {
               pugi::xml_node node = xnode.append_child("edge");
               node.append_attribute("source") = static_cast<int>(boost::source(m.first, bg_));
               node.append_attribute("target") = static_cast<int>(boost::target(m.first, bg_));
               m.second->write(node);
             });
}

bool SDFG::dfgGraph::exist(const string& src, const string& tar) const {
  shared_ptr<dfgNode> src_node = get_node(src);
  shared_ptr<dfgNode> tar_node = get_node(tar);
  if(src_node && tar_node) {
    return boost::edge(src_node->id, tar_node->id, bg_).second;
  } else {
    return false;
  }
}

bool SDFG::dfgGraph::exist(const vertex_descriptor& src, const vertex_descriptor& tar) const {
  return boost::edge(src, tar, bg_).second;
}

bool SDFG::dfgGraph::exist(const vertex_descriptor& src, const vertex_descriptor& tar, dfgEdge::edge_type_t tt) const {
  GraphTraits::out_edge_iterator eit, eend;
  for(boost::tie(eit, eend) = edge_range(src, tar, bg_);
      eit != eend; ++eit) { 
    if(get_edge(*eit)->type == tt) return true;
  }
  return false;
}

bool SDFG::dfgGraph::exist(const string& src, const string& tar, dfgEdge::edge_type_t tt) const {
  if(exist(src) && exist(tar))
    return exist(node_map.find(src)->second, node_map.find(tar)->second, tt);
  else
    return false;
}

bool SDFG::dfgGraph::exist(const std::string& name) const {
  return get_node(name);
}    

