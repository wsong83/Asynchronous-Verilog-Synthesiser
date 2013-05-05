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
 * An edge in the SDFG library
 * 17/09/2012   Wei Song
 *
 *
 */

#include "dfg_edge.hpp"
#include "dfg_node.hpp"
#include "dfg_graph.hpp"

#include <ogdf/basic/GraphAttributes.h>
#include <boost/format.hpp>

using namespace SDFG;
using boost::shared_ptr;
using std::map;
using std::string;
using std::pair;
using std::list;


void SDFG::dfgEdge::write(pugi::xml_node& xnode) const {
  xnode.append_attribute("name") = name.c_str();
  string stype;
  switch(type) {
  case SDFG_DP:     stype = "data";      break;
  case SDFG_DDP:    stype = "self-data"; break;
  case SDFG_CTL:    stype = "control";   break;
  case SDFG_CLK:    stype = "clk";       break;
  case SDFG_RST:    stype = "rst";       break;
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
    show_hash("self-data");     // 0xdc983be0
    show_hash("control");       // 0xee9cb7ef
    show_hash("clk");           // 0x0018f66b
    show_hash("rst");           // 0x001cb9f4
    show_hash("unknown");       // 0xbddbfb6d
  }

  name = xnode.attribute("name").as_string();
  switch(shash(xnode.attribute("type").as_string())) {
  case 0xdc983be0: type = SDFG_DDP; break;
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

bool SDFG::dfgEdge::check_integrity() const {
  assert(pg->get_target(id));
  assert(pg->get_source(id));
  return true;
}


