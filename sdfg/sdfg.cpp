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
#include <boost/lexical_cast.hpp>
#include <set>
#include <algorithm>

#include <ogdf/layered/SugiyamaLayout.h>
#include <ogdf/layered/FastHierarchyLayout.h>
#include <ogdf/layered/LongestPathRanking.h>
//#include <ogdf/layered/OptimalRanking.h>
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

void SDFG::dfgNode::graphic_init() {
  if(bbox.first == 0.0)
    switch(type) {
    case SDFG_COMB:    bbox = pair<double, double>(40.0, 25.0); break;
    case SDFG_FF:      bbox = pair<double, double>(20.0, 20.0); break;
    case SDFG_MODULE:  bbox = pair<double, double>(60.0, 35.0); break;
    case SDFG_IPORT:   bbox = pair<double, double>(20.0, 20.0); break;
    case SDFG_OPORT:   bbox = pair<double, double>(20.0, 20.0); break;
    case SDFG_PORT:    bbox = pair<double, double>(30.0, 30.0); break;
    default:           bbox = pair<double, double>(20.0, 20.0); break;
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
    pugi::xml_node xmodule = xnode.append_child("module");
    xmodule.append_attribute("name") = child_name.c_str();
    for_each(port2sig.begin(), port2sig.end(), 
             [&](const pair<const string, const string>& m) {
               pugi::xml_node port = xmodule.append_child("portmap");
               port.append_attribute("port") = m.first.c_str();
               port.append_attribute("signal") = m.second.c_str();
             });
    if(bbox.first != 0.0) {     // graphic information
      pugi::xml_node xgraphic = xnode.append_child("graphic");
      pugi::xml_node xsize = xgraphic.append_child("size");
      xsize.append_attribute("width") = bbox.first;
      xsize.append_attribute("height") = bbox.second;
      pugi::xml_node xpos = xgraphic.append_child("position");
      xpos.append_attribute("x") = position.first;
      xpos.append_attribute("y") = position.second;
    }
  }
}

void SDFG::dfgNode::write(void *pnode, ogdf::GraphAttributes *pga) {
  ogdf::node pn = static_cast<ogdf::node>(pnode);
  graphic_init();
  pga->labelNode(pn) = boost::str(boost::format("%u") % static_cast<unsigned int>(id)).c_str();
  pga->width(pn) = bbox.first;
  pga->height(pn) = bbox.second;
  pga->x(pn) = position.first;
  pga->y(pn) = position.second;
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

  id = static_cast<vertex_descriptor>(xnode.attribute("id").as_uint());
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
    pugi::xml_node xmodule = xnode.child("module");
    child_name = xmodule.attribute("name").as_string();
    for(pugi::xml_node port = xmodule.child("portmap"); port; port = port.next_sibling("portmap")) {
      string port_name = port.attribute("port").as_string();
      string port_signal = port.attribute("signal").as_string();
      port2sig[port_name] = port_signal;
      sig2port.insert(pair<string, string>(port_signal, port_name));
    }
  }

  return true;

}

bool SDFG::dfgNode::read(void * const pnode, ogdf::GraphAttributes * const pga) {
  ogdf::node const pn = static_cast<ogdf::node const>(pnode);
  
  position.first = pga->x(pn);
  position.second = pga->y(pn);

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
  
  if(!bend.empty()) {     // graphic information
    pugi::xml_node xgraphic = xnode.append_child("graphic");
    for_each(bend.begin(), bend.end(), 
             [&](const pair<double, double>& m) {
               pugi::xml_node xbend = xgraphic.append_child("point");
               xbend.append_attribute("x") = m.first;
               xbend.append_attribute("x") = m.second;
             });
  }
}

void SDFG::dfgEdge::write(void *pedge, ogdf::GraphAttributes *pga) {
  ogdf::edge pe = static_cast<ogdf::edge>(pedge);
  // as multiple edges can exist between two nodes, edge type is stored to identify the single edge
  pga->labelEdge(pe) = boost::str(boost::format("%u") % type).c_str();
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

bool SDFG::dfgEdge::read(void * const pedge, ogdf::GraphAttributes * const pga) {
  ogdf::edge const pe = static_cast<ogdf::edge const>(pedge);
  
  bend.clear();
  for(ogdf::ListConstIterator<ogdf::DPoint> b = pga->bends(pe).begin(); b.valid(); ++b) {
    bend.push_back(pair<double,double>((*b).m_x, (*b).m_y));
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
  node->pg = this;
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
  edge->pg = this;
}

shared_ptr<dfgEdge> SDFG::dfgGraph::add_edge(const string& n, dfgEdge::edge_type_t t, const string& src, const string& snk) {
  shared_ptr<dfgEdge> edge(new dfgEdge(n, t));
  add_edge(edge, src, snk);
  return edge;
}

bool SDFG::dfgGraph::remove_edge(boost::shared_ptr<dfgNode> src, boost::shared_ptr<dfgNode> tar, dfgEdge::edge_type_t etype) {
  if(src && tar) 
    remove_edge(src->id, tar->id, etype);
  else
    return false;
}

bool SDFG::dfgGraph::remove_edge(const std::string& src, const std::string& tar, dfgEdge::edge_type_t etype) {
  shared_ptr<dfgEdge> pe = get_edge(src, tar, etype);
  if(pe)
    return remove_edge(pe->id);
  else
    return false;
}

bool SDFG::dfgGraph::remove_edge(const vertex_descriptor& src, const vertex_descriptor& tar, dfgEdge::edge_type_t etype) {
  shared_ptr<dfgEdge> pe = get_edge(src, tar, etype);
  if(pe)
    return remove_edge(pe->id);
  else
    return false;
}

bool SDFG::dfgGraph::remove_edge(const edge_descriptor& eid) {
  if(edges.erase(eid)) {
    boost::remove_edge(eid, bg_);
    return true;
  } else
    return false;
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

shared_ptr<dfgNode> SDFG::dfgGraph::get_source(const edge_descriptor& eid) const {
  if(exist(eid))
    return nodes.find(boost::source(eid, bg_))->second;
  else
    return shared_ptr<dfgNode>();
}

shared_ptr<dfgNode> SDFG::dfgGraph::get_source(shared_ptr<dfgEdge> pe) const {
  if(pe)
    return get_source(pe->id);
  else
    return shared_ptr<dfgNode>();
}

shared_ptr<dfgNode> SDFG::dfgGraph::get_target(const edge_descriptor& eid) const {
  if(exist(eid))
    return nodes.find(boost::target(eid, bg_))->second;
  else
    return shared_ptr<dfgNode>();
}

shared_ptr<dfgNode> SDFG::dfgGraph::get_target(shared_ptr<dfgEdge> pe) const {
  if(pe)
    return get_target(pe->id);
  else
    return shared_ptr<dfgNode>();
}

bool SDFG::dfgGraph::layout() {
  ogdf::Graph g;
  ogdf::GraphAttributes ga;
  write(&g, &ga);
  return layout(&g, &ga);
}

bool SDFG::dfgGraph::layout(ogdf::Graph* pg, ogdf::GraphAttributes *pga) {
  // Sugiyama Layout
  ogdf::SugiyamaLayout SL;

  ogdf::LongestPathRanking *ranking = new ogdf::LongestPathRanking();
  //ogdf::OptimalRanking * ranking = new ogdf::OptimalRanking();
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

  SL.runs(30);

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
                 if(m.second->type == dfgEdge::SDFG_CTL) { // control
                   m.second->bend.push_back(pair<double, double>
                                            (node->position.first+G_NODE_H * G_NODE_DIST,
                                             node->position.second));
                   m.second->bend.push_back(pair<double, double>
                                            (node->position.first+G_NODE_H * G_NODE_DIST * 0.5,
                                             node->position.second - G_NODE_H * G_NODE_DIST*0.866));
                 } else if(m.second->type == dfgEdge::SDFG_DP) { // data
                   m.second->bend.push_back(pair<double, double>
                                            (node->position.first-G_NODE_H * G_NODE_DIST,
                                             node->position.second));
                   m.second->bend.push_back(pair<double, double>
                                            (node->position.first-G_NODE_H * G_NODE_DIST * 0.5,
                                             node->position.second - G_NODE_H * G_NODE_DIST*0.866));
                 } else {       // other, should not be this case
                   m.second->bend.push_back(pair<double, double>
                                            (node->position.first+G_NODE_H * G_NODE_DIST,
                                             node->position.second));
                   m.second->bend.push_back(pair<double, double>
                                            (node->position.first+G_NODE_H * G_NODE_DIST * 0.5,
                                             node->position.second + G_NODE_H * G_NODE_DIST*0.866));
                 }
               }
             });

  return true;
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
    src = static_cast<vertex_descriptor>(edge.attribute("source").as_uint());
    tar = static_cast<vertex_descriptor>(edge.attribute("target").as_uint());
    add_edge(pedge,id_map[src], id_map[tar]);
  }
  
  return true;
}

bool SDFG::dfgGraph::read(ogdf::Graph * const pg, ogdf::GraphAttributes * const pga){
  ogdf::node n;
  forall_nodes(n, *pg) {
    vertex_descriptor nid = static_cast<vertex_descriptor>
      (boost::lexical_cast<unsigned long>(pga->labelNode(n).cstr()));
    if(!nodes.count(nid)) return false;
    nodes[nid]->read(n, pga);
  }

  ogdf::edge e;
  forall_edges(e, *pg) {
    vertex_descriptor src, tar;
    src = static_cast<vertex_descriptor>
      (boost::lexical_cast<unsigned long>(pga->labelNode(e->source()).cstr()));
    tar = static_cast<vertex_descriptor>
      (boost::lexical_cast<unsigned long>(pga->labelNode(e->target()).cstr()));
    dfgEdge::edge_type_t etype = static_cast<dfgEdge::edge_type_t>
      (boost::lexical_cast<unsigned long>(pga->labelEdge(e).cstr()));
    if(!exist(src, tar, etype)) return false;
    get_edge(src, tar, etype)->read(e, pga);
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

bool SDFG::dfgGraph::exist(const edge_descriptor& eid) const {
  return get_edge(eid);
}

bool SDFG::dfgGraph::exist(const std::string& name) const {
  return get_node(name);
}    

void SDFG::dfgGraph::simplify(list<shared_ptr<dfgNode> >& proc_list) {
  // first I need to implement remove nodes

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
