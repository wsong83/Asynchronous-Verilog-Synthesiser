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
 * A path in the SDFG library
 * 17/09/2012   Wei Song
 *
 *
 */

#include "dfg_path.hpp"
#include "dfg_node.hpp"
#include "dfg_edge.hpp"

#include <boost/foreach.hpp>

using namespace SDFG;
using boost::shared_ptr;
using std::map;
using std::string;
using std::pair;
using std::list;

void SDFG::dfgPath::push_back(boost::shared_ptr<dfgNode> n, int et) {
  if(path.empty())
    src = n;
  path.push_back(path_type(n, et));
  type = cal_type(type, et);
  node_set.insert(n);
}
  
void SDFG::dfgPath::push_front(boost::shared_ptr<dfgNode> n, int et) {
  path.push_front(path_type(n, et));
  type = cal_type(et, type);
  node_set.insert(n);
  src = n;
}

void SDFG::dfgPath::combine(boost::shared_ptr<dfgPath> p) {
  tar = p->tar;
  path.insert(path.end(), p->path.begin(), p->path.end());
  type = cal_type(type, p->type);
}

std::ostream& SDFG::dfgPath::streamout(std::ostream& os) const {
  if(src && tar) {
    // get the type
    string stype;
    if(type == dfgEdge::SDFG_DF) stype = "DF";
    else {
      if(type & dfgEdge::SDFG_DP) stype = "DP";
      if(type & dfgEdge::SDFG_DDP) stype = stype = stype = stype.empty() ? "DDP" : stype + "|DDP";
      if(type & dfgEdge::SDFG_CTL) {
        if((type & dfgEdge::SDFG_CLK) == dfgEdge::SDFG_CLK) 
          stype = stype.empty() ? "CLK" : stype + "|CLK";
        else if((type & dfgEdge::SDFG_RST) == dfgEdge::SDFG_RST) 
          stype = stype.empty() ? "RST" : stype + "|RST";
        else 
          stype = stype.empty() ? "CTL" : stype + "|CTL";
      }
    }

    os << src->get_full_name() << "->" << tar->get_full_name() << " (" << stype << "): " << std::endl;
    if(!path.empty() && path.size() > 1) {
      BOOST_FOREACH(const path_type& m, path) {
        string stype;
        if(m.second == dfgEdge::SDFG_DF) stype = "DF";
        else {
          if(m.second & dfgEdge::SDFG_DP) stype = "DP";
          if(m.second & dfgEdge::SDFG_DDP) stype = stype = stype.empty() ? "DDP" : stype + "|DDP";
          if(m.second & dfgEdge::SDFG_CTL) {
            if((m.second & dfgEdge::SDFG_CLK) == dfgEdge::SDFG_CLK) 
              stype = stype.empty() ? "CLK" : stype + "|CLK";
            else if((m.second & dfgEdge::SDFG_RST) == dfgEdge::SDFG_RST) 
              stype = stype.empty() ? "RST" : stype + "|RST";
            else 
              stype = stype.empty() ? "CTL" : stype + "|CTL";
          }
        }
        os << m.first->get_full_name() << "(" << stype << ")";
      }
      os << tar->get_full_name();
      os << std::endl;
    }
  }

  return os;
}
 
int SDFG::dfgPath::cal_type(const int& t0, const int& t1) {
  int tt = (t0 << 12 )| t1;
  switch(tt) {
  case 0x000000:   return 0x000;
  case 0x000001:   return 0x001; // self-data
  case 0x000010:   return 0x010; // data
  case 0x000011:   return 0x011; // data/self-data
  case 0x000080:   return 0x080; // control
  case 0x000081:   return 0x081; // control/self-data
  case 0x000090:   return 0x090; // control/data
  case 0x000091:   return 0x091; // control/data/self-data
  case 0x0000a0:   return 0x0a0; // clk
  case 0x0000c0:   return 0x0c0; // rst
  case 0x001000:   return 0x001; // self-data
  case 0x001001:   return 0x001; // self-data
  case 0x010000:   return 0x010; // data
  case 0x010010:   return 0x010; // data
  case 0x010080:   return 0x080; // control
  case 0x010090:   return 0x090; // control/data
  case 0x0100a0:   return 0x0a0; // clk
  case 0x0100c0:   return 0x0c0; // rst
  case 0x011000:   return 0x011; // data/self-data
  case 0x080000:   return 0x080; // control
  case 0x080010:   return 0x080; // control
  case 0x080080:   return 0x080; // control
  case 0x080090:   return 0x080; // control
  case 0x0800a0:   return 0x0a0; // clk
  case 0x0800c0:   return 0x0c0; // rst
  case 0x081000:   return 0x081; // control/self-data
  case 0x090000:   return 0x090; // control/data
  case 0x090010:   return 0x090; // control/data
  case 0x090080:   return 0x080; // control
  case 0x090090:   return 0x090; // control/data
  case 0x0900a0:   return 0x0a0; // clk
  case 0x0900c0:   return 0x0c0; // rst
  case 0x091000:   return 0x091; // control/data/self-data
  case 0x0a0000:   return 0x0a0; // clk
  case 0x0c0000:   return 0x0c0; // rst
  default:
    std::cout << std::hex << tt << std::endl; 
    assert(0 == "impossible type calculation");
    return t1;
  }
}
