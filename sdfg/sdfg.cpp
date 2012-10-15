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
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <set>
#include <algorithm>
#include <iostream>

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



/////////////////////////////////////////////////////////////////////////////
/********        global definitions                                 ********/
/////////////////////////////////////////////////////////////////////////////

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




/////////////////////////////////////////////////////////////////////////////
/********        Node                                               ********/
/////////////////////////////////////////////////////////////////////////////

dfgNode* SDFG::dfgNode::copy() const {
  dfgNode* rv = new dfgNode();
  rv->ptr = ptr;
  rv->child = child;
  rv->child_name = child_name;
  rv->sig2port = sig2port;
  rv->port2sig = port2sig;
  rv->pg = NULL;
  rv->name = name;
  rv->hier = hier;
  rv->id = NULL;
  rv->node_index = 0;
  rv->type = type;
  return rv;
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
  xnode.append_attribute("name") = get_hier_name().c_str();
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
  pga->labelNode(pn) = boost::str(boost::format("%u") % node_index).c_str();
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

  node_index = xnode.attribute("id").as_uint();
  set_hier_name(xnode.attribute("name").as_string());
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
      sig2port[port_signal].push_back(port_name);
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

shared_ptr<dfgNode> SDFG::dfgNode::flatten() const {
  if(!pg->father)
    return shared_ptr<dfgNode>();
  else {
    shared_ptr<dfgNode> rv(copy());
    rv->hier.push_front(pg->father->name);
    // change node type if it is a port
    if(rv->type & SDFG_PORT)
      rv->type = SDFG_DF;
    pg->father->pg->add_node(rv);
    return rv;
  }
}

string SDFG::dfgNode::get_hier_name() const {
  string rv;
  BOOST_FOREACH(const string& m, hier) {
    rv += m + "/";
  }

  return rv+name;
}

string SDFG::dfgNode::get_full_name() const {
  string rv = pg->get_full_name();
  if(rv.empty())
    rv = get_hier_name();
  else
    rv += "/" + get_hier_name();
  return rv;
}

void SDFG::dfgNode::set_hier_name(const string& hname) {
  boost::char_separator<char> sep("/");
  boost::tokenizer<boost::char_separator<char> > tokens(hname, sep);
  hier.clear();
  BOOST_FOREACH(const string& m, tokens) {
    hier.push_back(m);
  }
  name = hier.back();
  hier.pop_back();
}

void SDFG::dfgNode::remove_port_sig(const string& sname, int dir) {
  if(sig2port.count(sname)) {
    // remove the port map connection
    list<string>::iterator it = sig2port[sname].begin();
    list<string>::iterator end = sig2port[sname].end();
    while(it!=end) {
      if((child->get_node(*it)->type & SDFG_PORT) 
         && (child->get_node(*it)->type != SDFG_OPORT)
         && dir <= 0) {
        port2sig.erase(*it);
        sig2port[sname].erase(it);
        it = sig2port[sname].begin();
        end = sig2port[sname].end();
      } else if((child->get_node(*it)->type & SDFG_PORT) 
                && (child->get_node(*it)->type != SDFG_IPORT)
                && dir >= 0) {
        port2sig.erase(*it);
        sig2port[sname].erase(it);
        it = sig2port[sname].begin();
        end = sig2port[sname].end();
      } else 
        ++it;
    }
  }
}

void SDFG::dfgNode::add_port_sig(const string& pname, const string& sname) {
  if(child->exist(pname)) {
    port2sig[pname] = sname;
    sig2port[sname].push_back(pname);
  }
}


/////////////////////////////////////////////////////////////////////////////
/********        Edge                                               ********/
/////////////////////////////////////////////////////////////////////////////

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



/////////////////////////////////////////////////////////////////////////////
/********        Path                                               ********/
/////////////////////////////////////////////////////////////////////////////
void SDFG::dfgPath::push_back(boost::shared_ptr<dfgNode> n, int et) {
  if(path.empty())
    src = n;
  path.push_back(n);
  type |= et;
  node_set.insert(n);
}

void SDFG::dfgPath::push_front(boost::shared_ptr<dfgNode> n, int et) {
  path.push_front(n);
  type |= et;
  node_set.insert(n);
  src = n;
}

void SDFG::dfgPath::combine(boost::shared_ptr<dfgPath> p) {
  tar = p->tar;
  path.insert(path.end(), p->path.begin(), p->path.end());
  type |= p->type;
}

std::ostream& SDFG::dfgPath::streamout(std::ostream& os) const {
  if(src && tar) {
    // get the type
    string stype;
    if(type == dfgEdge::SDFG_DF) stype = "DF";
    else {
      if(type & dfgEdge::SDFG_DP) stype = "DP";
      if(type & dfgEdge::SDFG_CTL) {
        if((type & dfgEdge::SDFG_CLK) == dfgEdge::SDFG_CLK) stype = stype.empty() ? "CLK" : stype + "|CLK";
        if((type & dfgEdge::SDFG_RST) == dfgEdge::SDFG_RST) stype = stype.empty() ? "RST" : stype + "|RST";
        if((type & dfgEdge::SDFG_CTL) == dfgEdge::SDFG_CTL) stype = stype.empty() ? "CTL" : stype + "|CTL";
      }
    }

    os << src->get_full_name() << "->" << tar->get_full_name() << " (" << stype << "): " << std::endl;
    if(path.empty())
      os << src->get_full_name() << " > ... > " << tar->get_full_name();
    else {
      BOOST_FOREACH(shared_ptr<dfgNode> m, path) {
        os << m->get_full_name() << " > ";
      }
      os << tar->get_full_name();
    }
    os << std::endl;
  }

  return os;
}

/////////////////////////////////////////////////////////////////////////////
/********        Graph                                              ********/
/////////////////////////////////////////////////////////////////////////////


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
  node->node_index = ++node_index;
  index_map[node_index] = node->id;
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
  if(exist(src, snk, edge->type)) return; // do not insert a edge between the same nodes with the same type
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

shared_ptr<dfgEdge> SDFG::dfgGraph::add_edge(const string& n, dfgEdge::edge_type_t t, const vertex_descriptor& src, const vertex_descriptor& snk) {
  shared_ptr<dfgEdge> edge(new dfgEdge(n, t));
  add_edge(edge, src, snk);
  return edge;
}


///////////////////////////////
// remove nodes and edges
///////////////////////////////
void SDFG::dfgGraph::remove_node(const string& node) {
  if(node_map.count(node))
    remove_node(node_map[node]);
}

void SDFG::dfgGraph::remove_node(shared_ptr<dfgNode> pn) {
  if(pn)
    return remove_node(pn->id);
}

void SDFG::dfgGraph::remove_node(const vertex_descriptor& nid) {
  if(!nodes.count(nid)) return;
  shared_ptr<dfgNode> pn = nodes[nid];
  
  // remove all output edges
  {
    list<shared_ptr<dfgEdge> > elist = get_out_edges(nid);
    BOOST_FOREACH(shared_ptr<dfgEdge> m, elist)
      remove_edge(m->id);
  }

  // remove all input edges
  {
    list<shared_ptr<dfgEdge> > elist = get_in_edges(nid);
    BOOST_FOREACH(shared_ptr<dfgEdge> m, elist)
      remove_edge(m->id);
  }

  // remove the node
  node_map.erase(pn->get_hier_name());
  nodes.erase(nid);
  boost::remove_vertex(nid, bg_);
  pn->pg = NULL;                    // make sure it cannot access graph

  if(pn->type & dfgNode::SDFG_PORT) { // port
    remove_port(pn->get_hier_name());
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

void SDFG::dfgGraph::remove_edge(boost::shared_ptr<dfgNode> src, boost::shared_ptr<dfgNode> tar) {
  if(src && tar) 
    remove_edge(src->id, tar->id);
}

void SDFG::dfgGraph::remove_edge(const std::string& src, const std::string& tar) {
  if(node_map.count(src) && node_map.count(tar))
    remove_edge(node_map[src], node_map[tar]);
}

void SDFG::dfgGraph::remove_edge(const vertex_descriptor& src, const vertex_descriptor& tar) {
  edge_descriptor eid;
  bool found;
  boost::tie(eid, found) = boost::edge(src, tar, bg_);
  while(found) {
    remove_edge(eid);
    boost::tie(eid, found) = boost::edge(src, tar, bg_);
  }
}

void SDFG::dfgGraph::remove_edge(boost::shared_ptr<dfgNode> src, boost::shared_ptr<dfgNode> tar, dfgEdge::edge_type_t etype) {
  if(src && tar) remove_edge(src->id, tar->id, etype);
}

void SDFG::dfgGraph::remove_edge(const std::string& src, const std::string& tar, dfgEdge::edge_type_t etype) {
  shared_ptr<dfgEdge> pe = get_edge(src, tar, etype);
  if(pe) remove_edge(pe->id);
}

void SDFG::dfgGraph::remove_edge(const vertex_descriptor& src, const vertex_descriptor& tar, dfgEdge::edge_type_t etype) {
  shared_ptr<dfgEdge> pe = get_edge(src, tar, etype);
  if(pe) remove_edge(pe->id);
}

void SDFG::dfgGraph::remove_edge(boost::shared_ptr<dfgEdge> edge) {
  if(edge) remove_edge(edge->id);
}

void SDFG::dfgGraph::remove_edge(const edge_descriptor& eid) {
  if(edges.count(eid)) {
    shared_ptr<dfgEdge> pe = edges[eid];
    shared_ptr<dfgNode> src = get_source(eid);
    shared_ptr<dfgNode> tar = get_target(eid);
    
    if(src->type == dfgNode::SDFG_MODULE) // remove a port signal
      src->remove_port_sig(tar->get_hier_name(), 1); // output

    if(tar->type == dfgNode::SDFG_MODULE)
      tar->remove_port_sig(src->get_hier_name(), -1); // input

    edges.erase(eid);
    boost::remove_edge(eid, bg_);
    pe->pg = NULL;
  }
}

void SDFG::dfgGraph::remove_port(const std::string& pname) {
  if(father && father->port2sig.count(pname)) {
    string sname = father->port2sig[pname];
    // remove the port map connection
    list<string>::iterator it, end;
    for(it=father->sig2port[sname].begin(), end=father->sig2port[sname].end(); it!=end; ++it) {
      if(*it == pname) {
        father->sig2port[sname].erase(it);
        break;
      }
    }
    // scan it again to get input/output count
    bool inpc = false;
    bool outpc = false;
    for(it=father->sig2port[sname].begin(), end=father->sig2port[sname].end(); it!=end; ++it) {
      switch(get_node(*it)->type) {
      case dfgNode::SDFG_IPORT: inpc = true; break;
      case dfgNode::SDFG_OPORT: outpc = true; break;
      case dfgNode::SDFG_PORT:  inpc = true; outpc = true; break;
      default: assert(0 == "node type wrong!");
      }
    }
    
    if(!inpc) {               // remove input
      father->pg->remove_edge(sname, father->get_hier_name());
    }
    
    if(!outpc) {               // remove output
      father->pg->remove_edge(father->get_hier_name(), sname);
    }
    
    father->port2sig.erase(pname);
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
                                  fnodes[get_source_id(m.first)]->id,
                                  fnodes[get_target_id(m.first)]->id
                                  );
           });

  // connect all ports
  for_each(nodes.begin(), nodes.end(),
           [&](const pair<const vertex_descriptor, shared_ptr<dfgNode> >& m) {
               if(m.second->type == dfgNode::SDFG_IPORT) {
                 shared_ptr<dfgNode> src = get_in_nodes_cb(m.second->id).front();
                 father->pg->add_edge(src->get_hier_name(), 
                                      get_in_edges_cb(m.second->id).front()->type,
                                      src->id,
                                      fnodes[m.first]->id
                                      );
               }
               if(m.second->type == dfgNode::SDFG_OPORT) {
                 shared_ptr<dfgNode> tar = get_out_nodes_cb(m.second->id).front();
                 father->pg->add_edge(fnodes[m.first]->get_hier_name(), 
                                      get_out_edges_cb(m.second->id).front()->type,
                                      fnodes[m.first]->id,
                                      tar->id
                                      );
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
                                          fnodes[m.first]->id,
                                          snode->id);
                   }
                   // input
                   for(boost::tie(oit, oend) = 
                         boost::edge_range(
                                           snode->id,
                                           father->id,
                                           father->pg->bg_);
                       oit != oend; ++oit) { 
                     father->pg->add_edge(snode->get_hier_name(), 
                                          father->pg->edges.find(*oit)->second->type,
                                          snode->id,
                                          fnodes[m.first]->id);
                   }
                 }
               }             
             });

  return rv;
}


///////////////////////////////
// get nodes and edges
///////////////////////////////
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
  for(boost::tie(eit, eend) = boost::edge_range(src, tar, bg_);
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

shared_ptr<dfgNode> SDFG::dfgGraph::get_source_cb(const edge_descriptor& eid) const {
  shared_ptr<dfgNode> inode = get_source(eid);
  if(inode->type == dfgNode::SDFG_MODULE && inode->child) {
    list<string> plist = inode->sig2port.find(get_target(eid)->get_hier_name())->second;
    if(!plist.empty()) {
      BOOST_FOREACH(string& m, plist) {
        shared_ptr<dfgNode> n = inode->child->get_node(m);
        if(n->type & dfgNode::SDFG_OPORT && n->type != dfgNode::SDFG_IPORT)
          return n;
      }
      return shared_ptr<dfgNode>();
    }
    return shared_ptr<dfgNode>();
  } else
    return inode;
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

list<shared_ptr<dfgNode> > SDFG::dfgGraph::get_target_cb(const edge_descriptor& eid) const {
  list<shared_ptr<dfgNode> > rv;
  shared_ptr<dfgNode> onode = get_target(eid);
  if(onode->type == dfgNode::SDFG_MODULE && onode->child) {
    list<string> plist = onode->sig2port.find(get_source(eid)->get_hier_name())->second;
    if(!plist.empty()) {
      BOOST_FOREACH(string& m, plist) {
        shared_ptr<dfgNode> n = onode->child->get_node(m);
        if(n->type & dfgNode::SDFG_IPORT && n->type != dfgNode::SDFG_OPORT)
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
// existance check
///////////////////////////////
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

bool SDFG::dfgGraph::exist(shared_ptr<dfgNode> src, shared_ptr<dfgNode> tar, dfgEdge::edge_type_t tt) const {
  if(src && tar) return exist(src->id, tar->id, tt);
  else return false;
}

bool SDFG::dfgGraph::exist(const edge_descriptor& eid) const {
  return edges.count(eid);
}

bool SDFG::dfgGraph::exist(shared_ptr<dfgEdge> e) const {
  if(e) return exist(e->id);
  else return false;
}    

bool SDFG::dfgGraph::exist(const vertex_descriptor& nid) const {
  return nodes.count(nid);
}    

bool SDFG::dfgGraph::exist(shared_ptr<dfgNode> n) const {
  if(n) return exist(n->id);
  else return false;
}    

bool SDFG::dfgGraph::exist(const std::string& name) const {
  return node_map.count(name);
}


///////////////////////////////
// traverse
///////////////////////////////
unsigned int SDFG::dfgGraph::size_out_edges(const vertex_descriptor& nid) const {
  if(nodes.count(nid))
    return boost::out_degree(nid, bg_);
  else
    return 0;
}

unsigned int SDFG::dfgGraph::size_out_edges_cb(const vertex_descriptor& nid) const {
  if(nodes.count(nid)) {
    shared_ptr<dfgNode> pn = nodes.find(nid)->second;
    if(pn->type == dfgNode::SDFG_OPORT) { // output port
      if(father && father->port2sig.count(pn->get_hier_name()))
        return 1;
      else
        return 0;
    } else if(pn->type == dfgNode::SDFG_PORT) { // I/O port
      if(father && father->port2sig.count(pn->get_hier_name()))
        return 1 + boost::out_degree(nid, bg_);
      else
        return boost::out_degree(nid, bg_);
    } else
      return boost::out_degree(nid, bg_);
  } else
    return 0;
}

unsigned int SDFG::dfgGraph::size_out_edges(const string& node) const {
  if(node_map.count(node))
    return size_out_edges(node_map.find(node)->second);
  else
    return 0;
}

unsigned int SDFG::dfgGraph::size_out_edges(shared_ptr<dfgNode> pn) const {
  if(pn)
    return size_out_edges(pn->id);
  else
    return 0;
}

unsigned int SDFG::dfgGraph::size_in_edges(const vertex_descriptor& nid) const {
  if(nodes.count(nid))
    return boost::in_degree(nid, bg_);
  else
    return 0;
}

unsigned int SDFG::dfgGraph::size_in_edges_cb(const vertex_descriptor& nid) const {
  if(nodes.count(nid)) {
    shared_ptr<dfgNode> pn = nodes.find(nid)->second;
    if(pn->type == dfgNode::SDFG_IPORT) { // input port
      if(father && father->port2sig.count(pn->get_hier_name()))
        return 1;
      else
        return 0;
    } else if(pn->type == dfgNode::SDFG_PORT) { // I/O port
      if(father && father->port2sig.count(pn->get_hier_name()))
        return 1 + boost::in_degree(nid, bg_);
      else
        return boost::in_degree(nid, bg_);
    } else
      return boost::in_degree(nid, bg_);
  } else
    return 0;
}

unsigned int SDFG::dfgGraph::size_in_edges(const string& node) const {
  if(node_map.count(node))
    return size_in_edges(node_map.find(node)->second);
  else
    return 0;
}

unsigned int SDFG::dfgGraph::size_in_edges(shared_ptr<dfgNode> pn) const {
  if(pn)
    return size_in_edges(pn->id);
  else
    return 0;
}

list<shared_ptr<dfgNode> > SDFG::dfgGraph::get_out_nodes(const vertex_descriptor& nid) const {
  list<shared_ptr<dfgNode> > rv;
  GraphTraits::adjacency_iterator nit, nend;
  for(boost::tie(nit, nend) = boost::adjacent_vertices(nid, bg_);
      nit != nend; ++nit) {
    rv.push_back(nodes.find(*nit)->second);
  }
  return rv;
}

list<shared_ptr<dfgNode> > SDFG::dfgGraph::get_out_nodes_cb(const vertex_descriptor& nid) const {
  list<shared_ptr<dfgNode> > rv;
  if(!nodes.count(nid)) return rv;
  shared_ptr<dfgNode> pn = nodes.find(nid)->second;
  if(pn->type == dfgNode::SDFG_OPORT || pn->type == dfgNode::SDFG_PORT) { // output or I/O port
    if(father && father->port2sig.count(pn->get_hier_name())) {
        rv.push_back(father->pg->get_node(father->port2sig.find(pn->get_hier_name())->second));
    }
  } 

  if(pn->type != dfgNode::SDFG_OPORT) { // have internal outputs
    GraphTraits::adjacency_iterator nit, nend;
    for(boost::tie(nit, nend) = boost::adjacent_vertices(nid, bg_);
        nit != nend; ++nit) {
      rv.push_back(nodes.find(*nit)->second);
    }
  }
  return rv;
}

list<shared_ptr<dfgNode> > SDFG::dfgGraph::get_out_nodes(const std::string& node) const {
  if(node_map.count(node))
    return get_out_nodes(node_map.find(node)->second);
  return list<shared_ptr<dfgNode> >();
}

list<shared_ptr<dfgNode> > SDFG::dfgGraph::get_out_nodes(shared_ptr<dfgNode> pn) const {
  if(pn) 
    return get_out_nodes(pn->id);
  return list<shared_ptr<dfgNode> >();
}

list<shared_ptr<dfgNode> > SDFG::dfgGraph::get_in_nodes(const vertex_descriptor& nid) const {
  list<shared_ptr<dfgNode> > rv;
  GType::inv_adjacency_iterator nit, nend; //!! why inv_adjacency_iterator is in Graph instead of Trait?
  for(boost::tie(nit, nend) = boost::inv_adjacent_vertices(nid, bg_);
      nit != nend; ++nit) {
    rv.push_back(nodes.find(*nit)->second);
  }
  return rv;
}

list<shared_ptr<dfgNode> > SDFG::dfgGraph::get_in_nodes_cb(const vertex_descriptor& nid) const {
  list<shared_ptr<dfgNode> > rv;
  if(!nodes.count(nid)) return rv;
  shared_ptr<dfgNode> pn = nodes.find(nid)->second;
  if(pn->type == dfgNode::SDFG_IPORT || pn->type == dfgNode::SDFG_PORT) { // input or I/O port
    if(father && father->port2sig.count(pn->get_hier_name())) {
      rv.push_back(father->pg->get_node(father->port2sig.find(pn->get_hier_name())->second));
    }
  } 
  
  if(pn->type != dfgNode::SDFG_IPORT) { // have internal outputs
    GType::inv_adjacency_iterator nit, nend; //!! why inv_adjacency_iterator is in Graph instead of Trait?
    for(boost::tie(nit, nend) = boost::inv_adjacent_vertices(nid, bg_);
        nit != nend; ++nit) {
      rv.push_back(nodes.find(*nit)->second);
    }
  }
  return rv;
}

list<shared_ptr<dfgNode> > SDFG::dfgGraph::get_in_nodes(const std::string& node) const {
  if(node_map.count(node))
    return get_in_nodes(node_map.find(node)->second);
  return list<shared_ptr<dfgNode> >();
}

list<shared_ptr<dfgNode> > SDFG::dfgGraph::get_in_nodes(shared_ptr<dfgNode> pn) const {
  if(pn) 
    return get_in_nodes(pn->id);
  return list<shared_ptr<dfgNode> >();
}

list<shared_ptr<dfgEdge> > SDFG::dfgGraph::get_out_edges(const vertex_descriptor& nid) const {
  list<shared_ptr<dfgEdge> > rv;
  if(nodes.count(nid)) {
    GraphTraits::out_edge_iterator eit, eend;
    for(boost::tie(eit, eend) = boost::out_edges(nid, bg_);
        eit != eend; ++eit) {
      rv.push_back(edges.find(*eit)->second);
    }
  }
  return rv;
}

list<shared_ptr<dfgEdge> > SDFG::dfgGraph::get_out_edges_cb(const vertex_descriptor& nid) const {
  list<shared_ptr<dfgEdge> > rv;
  if(nodes.count(nid)) {
    shared_ptr<dfgNode> pn = nodes.find(nid)->second;
    if(pn->type == dfgNode::SDFG_OPORT || pn->type == dfgNode::SDFG_PORT) { // output or I/O port
      if(father && father->port2sig.count(pn->get_hier_name())) {
        string tar_name = father->port2sig.find(pn->get_hier_name())->second;
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

    if(pn->type != dfgNode::SDFG_OPORT) { // have internal outputs
      GraphTraits::out_edge_iterator eit, eend;
      for(boost::tie(eit, eend) = boost::out_edges(nid, bg_);
          eit != eend; ++eit) {
        rv.push_back(edges.find(*eit)->second);
      }
    }
  }
  return rv;
}

list<shared_ptr<dfgEdge> > SDFG::dfgGraph::get_out_edges(const std::string& node) const {
  if(node_map.count(node))
    return get_out_edges(node_map.find(node)->second);
  return list<shared_ptr<dfgEdge> >();
}

list<shared_ptr<dfgEdge> > SDFG::dfgGraph::get_out_edges(boost::shared_ptr<dfgNode> pn) const {
  if(pn) 
    return get_out_edges(pn->id);
  return list<shared_ptr<dfgEdge> >();
}

list<shared_ptr<dfgEdge> > SDFG::dfgGraph::get_in_edges(const vertex_descriptor& nid) const {
  list<shared_ptr<dfgEdge> > rv;
  if(nodes.count(nid)) {
    GraphTraits::in_edge_iterator eit, eend;
    for(boost::tie(eit, eend) = boost::in_edges(nid, bg_);
        eit != eend; ++eit) {
      assert(nid == boost::target(*eit, bg_));
      rv.push_back(edges.find(*eit)->second);
    }
  }
  return rv;
}

list<shared_ptr<dfgEdge> > SDFG::dfgGraph::get_in_edges_cb(const vertex_descriptor& nid) const {
  list<shared_ptr<dfgEdge> > rv;
  if(nodes.count(nid)) {
    shared_ptr<dfgNode> pn = nodes.find(nid)->second;
    if(pn->type == dfgNode::SDFG_IPORT || pn->type == dfgNode::SDFG_PORT) { // input or I/O port
      if(pn && father && father->port2sig.count(pn->get_hier_name())) {
        string tar_name = father->port2sig.find(pn->get_hier_name())->second;
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

    if(pn->type != dfgNode::SDFG_IPORT) { // have internal outputs
      GraphTraits::in_edge_iterator eit, eend;
      for(boost::tie(eit, eend) = boost::in_edges(nid, bg_);
          eit != eend; ++eit) {
        assert(nid == boost::target(*eit, bg_));
        rv.push_back(edges.find(*eit)->second);
      }
    }
  }
  return rv;
}

list<shared_ptr<dfgEdge> > SDFG::dfgGraph::get_in_edges(const std::string& node) const {
  if(node_map.count(node))
    return get_in_edges(node_map.find(node)->second);
  return list<shared_ptr<dfgEdge> >();
}

list<shared_ptr<dfgEdge> > SDFG::dfgGraph::get_in_edges(boost::shared_ptr<dfgNode> pn) const {
  if(pn) 
    return get_in_edges(pn->id);
  return list<shared_ptr<dfgEdge> >();
}

///////////////////////////////
// graphic property
///////////////////////////////
unsigned int SDFG::dfgGraph::size_of_nodes() const {
  return nodes.size();
}

unsigned int SDFG::dfgGraph::size_of_regs() const {
  unsigned int rv = 0;
  for_each(nodes.begin(), nodes.end(),
           [&](const pair<const vertex_descriptor, shared_ptr<dfgNode> >& m) {
               if(m.second->type & (dfgNode::SDFG_FF|dfgNode::SDFG_LATCH))
                 rv++;
             });
  return rv;
}

unsigned int SDFG::dfgGraph::size_of_combs() const {
  unsigned int rv = 0;
  for_each(nodes.begin(), nodes.end(),
           [&](const pair<const vertex_descriptor, shared_ptr<dfgNode> >& m) {
               if(! m.second->type & (dfgNode::SDFG_FF|dfgNode::SDFG_LATCH|dfgNode::SDFG_MODULE))
                 rv++;
             });
  return rv;
}

unsigned int SDFG::dfgGraph::size_of_modules() const {
  unsigned int rv = 0;
  for_each(nodes.begin(), nodes.end(),
           [&](const pair<const vertex_descriptor, shared_ptr<dfgNode> >& m) {
               if(m.second->type & dfgNode::SDFG_MODULE)
                 rv++;
             });
  return rv;
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
  for_each(nodes.begin(), nodes.end(), 
           [&](const pair<const vertex_descriptor, shared_ptr<dfgNode> >& m) {
               pugi::xml_node node = xnode.append_child("node");
               node.append_attribute("id") = m.second->node_index;
               m.second->write(node, GList);
             });
           
  // write all edges to the graph
  for_each(edges.begin(), edges.end(), 
           [&](const pair<const edge_descriptor, shared_ptr<dfgEdge> >& m) {
               pugi::xml_node node = xnode.append_child("edge");
               node.append_attribute("source") = get_source(m.first)->node_index;
               node.append_attribute("target") = get_target(m.first)->node_index;
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
