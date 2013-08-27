/*
 * Copyright (c) 2011-2013 Wei Song <songw@cs.man.ac.uk> 
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
 * A node in the SDFG library
 * 17/09/2012   Wei Song
 *
 *
 */

#include "dfg_node.hpp"
#include "dfg_edge.hpp"
#include "dfg_graph.hpp"

#include <ogdf/basic/GraphAttributes.h>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>

using namespace SDFG;
using boost::shared_ptr;
using std::map;
using std::string;
using std::pair;
using std::list;


unsigned int SDFG::dfgNode::size_out_edges(bool bself) const { 
  return pg->size_out_edges(id, bself); 
}

unsigned int SDFG::dfgNode::size_out_edges_cb(bool bself) const { // cb: cross-boundar
  return pg->size_out_edges_cb(id, bself); 
}

unsigned int SDFG::dfgNode::size_in_edges(bool bself) const {
  return pg->size_in_edges(id, bself); 
}

unsigned int SDFG::dfgNode::size_in_edges_cb(bool bself) const { // cb: cross-boundar
  return pg->size_in_edges_cb(id, bself); 
}

list<shared_ptr<dfgNode> > SDFG::dfgNode::get_out_nodes(bool bself) const {
  return pg->get_out_nodes(id, bself); 
}

list<shared_ptr<dfgNode> > SDFG::dfgNode::get_out_nodes_cb(bool bself) const {
  return pg->get_out_nodes_cb(id, bself); 
}

list<shared_ptr<dfgNode> > SDFG::dfgNode::get_in_nodes(bool bself) const {
  return pg->get_in_nodes(id, bself); 
}

list<shared_ptr<dfgNode> > SDFG::dfgNode::get_in_nodes_cb(bool bself) const {
  return pg->get_in_nodes_cb(id, bself); 
}

list<shared_ptr<dfgEdge> > SDFG::dfgNode::get_out_edges(bool bself) const {
  return pg->get_out_edges(id, bself);
}

list<shared_ptr<dfgEdge> > SDFG::dfgNode::get_out_edges_cb(bool bself) const {
  return pg->get_out_edges_cb(id, bself); 
}

list<shared_ptr<dfgEdge> > SDFG::dfgNode::get_in_edges(bool bself) const {
  return pg->get_in_edges(id, bself);
}

list<shared_ptr<dfgEdge> > SDFG::dfgNode::get_in_edges_cb(bool bself) const {
  return pg->get_in_edges_cb(id, bself); 
}

int SDFG::dfgNode::get_in_edges_type(bool bself) const {
  return pg->get_in_edges_type(id, bself);
}

int SDFG::dfgNode::get_out_edges_type(bool bself) const {
  return pg->get_out_edges_type(id, bself);
}

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
  rv->dp_type = dp_type;
  return rv;
}

void SDFG::dfgNode::graphic_init() {
  if(bbox.first == 0.0)
    switch(type) {
    case SDFG_COMB:    bbox = pair<double, double>(40.0, 25.0); break;
    case SDFG_FF:      bbox = pair<double, double>(20.0, 20.0); break;
    case SDFG_LATCH:   bbox = pair<double, double>(35.0, 20.0); break;
    case SDFG_MODULE:  bbox = pair<double, double>(60.0, 35.0); break;
    case SDFG_GATE:    bbox = pair<double, double>(35.0, 35.0); break;
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
  case SDFG_GATE:    stype = "gate";    break;
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
    show_hash("gate");          // 0x0cf87a65
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
  case 0x0cf87a65: type = SDFG_GATE;   break;
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
      sig2port[port_signal].insert(port_name);
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
  if(type == SDFG_MODULE && sig2port.count(sname)) {
    // remove the port map connection
    std::set<string> m_slist = sig2port[sname]; // local copy
    BOOST_FOREACH(const string& sig, m_slist) {
      shared_ptr<dfgNode> child_node = child->get_node(sig);
      if(child_node) {
        if((child_node->type & SDFG_PORT) 
           && (child_node->type != SDFG_OPORT)
           && dir <= 0) {
          port2sig[sig] = string();
          sig2port[sname].erase(sig);
        } else if((child_node->type & SDFG_PORT) 
                  && (child_node->type != SDFG_IPORT)
                  && dir >= 0) {
          port2sig[sig] = string();
          sig2port[sname].erase(sig);
        } 
      }
    }
  }
  if(sig2port[sname].size() == 0) sig2port.erase(sname);
}

void SDFG::dfgNode::add_port_sig(const string& pname, const string& sname) {
  if(type == SDFG_MODULE && child->exist(pname)) {
    port2sig[pname] = sname;
    sig2port[sname].insert(pname);
  }
}

void SDFG::dfgNode::remap_ports() {
  map<string, std::set<string> > m_s2p;
  map<string, string> m_p2s;

  BOOST_FOREACH(shared_ptr<dfgNode> n, child->get_list_of_nodes(SDFG_PORT)) {
    string pname = n->get_hier_name();
    if(port2sig.count(pname)) {
      m_p2s[pname] = port2sig[pname];
      string sname = port2sig[pname];
      if(sname.size())
        m_s2p[sname].insert(pname);
    }
  }

  port2sig = m_p2s;
  sig2port = m_s2p;
}

void SDFG::dfgNode::set_new_child(shared_ptr<dfgGraph> c) {
  child = c;
  child->father = this;
  remap_ports();
}

std::ostream& SDFG::dfgNode::streamout(std::ostream& os) const {
  os << "dfgNode: " << get_full_name() << " (";
  switch(type) {
  case SDFG_DF:     os << "default";  break;
  case SDFG_COMB:   os << "combi";    break;
  case SDFG_FF:     os << "FF";       break;
  case SDFG_LATCH:  os << "latch";    break;
  case SDFG_MODULE: os << "module";   break;
  case SDFG_GATE:   os << "gate";     break;
  case SDFG_IPORT:  os << "in";       break;
  case SDFG_OPORT:  os << "out";      break;
  case SDFG_PORT:   os << "inout";    break;
  default:          os << "unknown";
  }

  os << ")";

  return os;
}

bool SDFG::dfgNode::remove_useless_ports() {
  bool rv = false;
  std::map<std::string, std::string> m_port2sig = port2sig;
  BOOST_FOREACH(port2sig_type p2s, m_port2sig) {
    if(p2s.second.size() == 0) { // open or const input
      port2sig.erase(p2s.first);
      rv = true;
    }
  }
  return rv;
}

bool SDFG::dfgNode::check_integrity() const {
  if(type == SDFG_MODULE) {
    if(child) {
      assert(child->father == this);
      assert(child->check_integrity());
      BOOST_FOREACH(port2sig_type p2s, port2sig) {
        assert(child->exist(p2s.first));
        shared_ptr<dfgNode> p = child->get_node(p2s.first);
        assert(p);
        if(p2s.second.size() > 0) {
          assert(pg->exist(p2s.second));
          assert(p->type & SDFG_PORT);
          if(p->type != SDFG_IPORT) {
            assert(pg->exist(id, p2s.second));
          }
          if(p->type != SDFG_OPORT) {
            assert(pg->exist(p2s.second, id));
          }
          assert(sig2port.count(p2s.second));
          assert(sig2port.find(p2s.second)->second.count(p2s.first));
        }
      }
    }
  } else if(type & SDFG_PORT) {
    if(pg->father != NULL) {
      assert(pg->father->port2sig.count(get_hier_name()));
    }
  }
  return true;
}
