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
#include <boost/format.hpp>
#include <set>
#include <algorithm>

using namespace SDFG;
using boost::shared_ptr;
using std::map;
using std::string;
using std::pair;
using std::list;

namespace SDFG {
  namespace {
    // local functions
    unsigned long shash(const string& str) {
      unsigned int id_size = 32; // assuming all system has a long larger than 4 bytes
      unsigned long rv = 0;
      for(unsigned int i=0; i<str.size(); i++) {
        unsigned long highbit = rv >> (id_size - 2);
        rv <<= 7;
        rv &= 0xffffffff;
        rv |= str[i];
        rv ^= highbit;
      }
      return rv;
    }
    
    // display the hash id of a tring
    void show_hash(const string& str) {
      std::cout << "hash id of \"" << str << "\":" << boost::format("0x%x") % shash(str) << std::endl;
    }
  }
}

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
    xnode.append_child("module").text() = child_name.c_str();
    for_each(port2sig.begin(), port2sig.end(), 
             [&](const pair<const string, const string>& m) {
               pugi::xml_node port = xnode.append_child("portmap");
               port.append_attribute("port") = m.first.c_str();
               port.append_attribute("signal") = m.second.c_str();
             });
  }
}

bool SDFG::dfgNode::read(const pugi::xml_node& xnode) {
  if(1 == 0) {
    show_hash("combi");         // 0x3dfb7169
    show_hash("ff");            // 0x00003366
    show_hash("latch");         // 0xcc3d31e8
    show_hash("module");        // 0xfc9d7666
    show_hash("iport");         // 0x9e1bf974
    show_hash("oport");         // 0xfe1bf974
    show_hash("port");          // 0x0e1bf974
    show_hash("unknown");       // 0xbddbfb6d
    return false;
  }

  id = static_cast<vertex_descriptor>(xnode.attribute("id").as_int());
  name = xnode.attribute("name").as_string();
  switch(shash(xnode.attribute("type").as_string())) {
  case 0x3dfb7169: type = SDFG_COMB;   break;
  case 0x00003366: type = SDFG_FF;     break;
  case 0xcc3d31e8: type = SDFG_LATCH;  break;
  case 0xfc9d7666: type = SDFG_MODULE; break;
  case 0x9e1bf974: type = SDFG_IPORT;  break;
  case 0xfe1bf974: type = SDFG_OPORT;  break;
  case 0x0e1bf974: type = SDFG_PORT;   break;
  case 0xbddbfb6d: type = SDFG_DF;     break;
  default: assert(0 == 1); return false;
  }

  if(type == SDFG_MODULE) {     // port map
    child_name = xnode.child("module").text().get();
    for(pugi::xml_node port = xnode.child("portmap"); port; port = port.next_sibling("portmap")) {
      string port_name = port.attribute("port").as_string();
      string port_signal = port.attribute("signal").as_string();
      port2sig[port_name] = port_signal;
      sig2port.insert(pair<string, string>(port_signal, port_name));
    }
  }

  return true;

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

bool SDFG::dfgEdge::read(const pugi::xml_node& xnode) {
  if(1 == 0) {
    show_hash("data");          // 0x0c987a61
    show_hash("control");       // 0xee9cb7ef
    show_hash("clk");           // 0x0018f66b
    show_hash("rst");           // 0x001cb9f4
    show_hash("unknown");       // 0xbddbfb6d
  }

  name = xnode.attribute("name").as_string();
  switch(shash(xnode.attribute("type").as_string())) {
  case 0x0c987a61: type = SDFG_DP;  break;
  case 0xee9cb7ef: type = SDFG_CTL; break;
  case 0x0018f66b: type = SDFG_CLK; break;
  case 0x001cb9f4: type = SDFG_RST; break;
  case 0xbddbfb6d: type = SDFG_DF;  break;
  default: assert(0 == 1); return false;
  }

  return true;
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
  add_edge(edge, node_map[src], node_map[snk]);
}

void SDFG::dfgGraph::add_edge(shared_ptr<dfgEdge> edge, const vertex_descriptor& src, const vertex_descriptor& snk) {
  assert(edge);
  bool added;
  boost::tie(edge->id, added) = boost::add_edge(src, snk, bg_);
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

bool SDFG::dfgGraph::read(const pugi::xml_node& xnode) {
  // get the graph name
  name = xnode.attribute("name").as_string();

  map<vertex_descriptor, vertex_descriptor> id_map; // need to regenerate vertex_descriptor in BGL

  // get all nodes
  for(pugi::xml_node node = xnode.child("node"); node; node = node.next_sibling("node")) {
    shared_ptr<dfgNode> pnode(new dfgNode());
    if(!pnode->read(node)) {assert(0 == 1); return false;}
    vertex_descriptor orig = pnode->id;
    add_node(pnode);
    id_map[orig] = pnode->id;
  }

  // get all edges
  for(pugi::xml_node edge = xnode.child("edge"); edge; edge = edge.next_sibling("edge")) {
    shared_ptr<dfgEdge> pedge(new dfgEdge());
    if(!pedge->read(edge)) {assert(0 == 1); return false;}
    vertex_descriptor src, tar;
    src = static_cast<vertex_descriptor>(edge.attribute("source").as_int());
    tar = static_cast<vertex_descriptor>(edge.attribute("target").as_int());
    add_edge(pedge,id_map[src], id_map[tar]);
  }
  
  return true;
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
