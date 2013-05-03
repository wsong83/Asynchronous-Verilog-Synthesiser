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
  if(type == SDFG_MODULE && sig2port.count(sname)) {
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
  if(type == SDFG_MODULE && child->exist(pname)) {
    port2sig[pname] = sname;
    sig2port[sname].push_back(pname);
  }
}
