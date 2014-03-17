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

shared_ptr<dfgNode> SDFG::dfgPath::get_2nd_back() {
  if(path.size() == 0) return shared_ptr<dfgNode>();
  return path.back().first;
}

shared_ptr<dfgNode> SDFG::dfgPath::get_2nd_front() {
  if(path.size() == 0) return shared_ptr<dfgNode>();
  if(path.size() == 1) return tar;
  std::list<path_type>::iterator it = path.begin();
  it++;
  return it->first;
}

std::ostream& SDFG::dfgPath::streamout(std::ostream& os) const {
  if(src && tar) {
    os << src->get_full_name() << "->" 
       << tar->get_full_name() << " (" << get_stype(type) << "): " 
       << std::endl;
    if(!path.empty() && path.size() > 1) {
      BOOST_FOREACH(const path_type& m, path) {
        os << m.first->get_full_name() << "(" << get_stype(m.second) << ")";
      }
      os << tar->get_full_name();
      os << std::endl;
    }
  }

  return os;
}
 
int SDFG::dfgPath::cal_type(int t0, int t1) {
  if(t0 == dfgEdge::SDFG_DF)  return t1;
  if(t1 == dfgEdge::SDFG_DF)  return t0;
  if(t1 & dfgEdge::SDFG_CR_MASK) return t1; // reset and clock
  if(t0 == t1)                return t1;

  // other
  int gt = 0;
  if(t0 & dfgEdge::SDFG_CTL_MASK) gt |= 0x8;
  if(t0 & dfgEdge::SDFG_DAT_MASK) gt |= 0x4;
  if(t1 & dfgEdge::SDFG_CTL_MASK) gt |= 0x2;
  if(t1 & dfgEdge::SDFG_DAT_MASK) gt |= 0x1;
  int dt = cal_type_data(t0&0xf, t1&0xf);
  int ct = cal_type_control(t0>>4&0xff, t1>>4&0xff);
  int d2ct = cal_type_data2control(t0&0xf, t1>>4&0xff);

  switch(gt) {
  case 0xF:   return  (ct|d2ct) << 4 | dt;
  case 0xE:   return  (ct|d2ct) << 4;
  case 0xD:   return  ct << 4 | dt;
  case 0xB:
  case 0xA:   return ct << 4;
  case 0x9:   return t0;
  case 0x7:   return d2ct << 4 | dt;
  case 0x6:   return d2ct << 4;
  case 0x5:   return dt;
  default:
    assert(0 == "impossible data type calculation");
    return 0;
  }
}

int SDFG::dfgPath::cal_type_data(int t0, int t1) {
  int tt = (t0 << 4 )| t1;
  switch(tt) {
  case 0x00:
  case 0x20:
  case 0x40:
  case 0x60:
  case 0x80:
  case 0xA0:
  case 0xC0:
  case 0xE0:
  case 0x02:
  case 0x04:
  case 0x06:
  case 0x08:
  case 0x0A:
  case 0x0C:
  case 0x0E:   return 0x0;      // should not be used in the final value
  case 0x14:   return 0x1;      // default self data path
  case 0x34:   return 0x3;      // default self data path and cal
  case 0x54:   return 0x5;      // default self data path and assign
  case 0x74:   return 0x7;      // default self data path, cal and assign
  case 0x94:   return 0x9;      // default self data path and data
  case 0xB4:   return 0xB;      // default self data path, dat and cal
  case 0xD4:   return 0xD;      // default self data path, dat and assign 
  case 0xF4:   return 0xF;      // default self data path, dat, cal and assign 
  case 0x22:
  case 0x24:
  case 0x26:
  case 0x28:
  case 0x2A:
  case 0x2C:
  case 0x2E:   return 0x2;      // cal -> ??? : cal
  case 0x42:   return 0x2;      // ass -> cal : cal
  case 0x44:   return 0x4;      // ass -> ass : ass
  case 0x46:   return 0x6;      // ass -> cal|ass : cal|ass
  case 0x48:   return 0x8;      // ass -> dat : dat
  case 0x4A:   return 0xA;      // ass -> cal|dat : cal|dat
  case 0x4C:   return 0xC;      // ass -> ass|dat : ass|dat
  case 0x4E:   return 0xE;      // ass -> cal|ass|dat : cal|ass|dat
  case 0x62:   return 0x2;
  case 0x64:   return 0x6;
  case 0x66:   return 0x6;
  case 0x68:   return 0xA;
  case 0x6A:   return 0xA;
  case 0x6C:   return 0xE;
  case 0x6E:   return 0xE;
  case 0x82:
  case 0x84:
  case 0x86:
  case 0x88:
  case 0x8A:
  case 0x8C:
  case 0x8E:   return 0x8;      // dat -> ??? : dat
  case 0xA2:
  case 0xA4:
  case 0xA6:
  case 0xA8:
  case 0xAA:
  case 0xAC:
  case 0xAE:   return 0xA;      // cal|dat -> ??? : ctl|cal
  case 0xC2:   return 0xA;
  case 0xC4:   return 0xC;
  case 0xC6:   return 0xE;
  case 0xC8:   return 0x8;
  case 0xCA:   return 0xA;
  case 0xCC:   return 0xC;
  case 0xCE:   return 0xE;
  case 0xE2:   return 0xA;
  case 0xE4:   return 0xE;
  case 0xE6:   return 0xE;
  case 0xE8:   return 0xA;
  case 0xEA:   return 0xA;
  case 0xEC:   return 0xE;
  case 0xEE:   return 0xE;  
  default:
    std::cout << std::hex << tt << std::endl; 
    assert(0 == "impossible data type calculation");
    return t1;
  }
}

int SDFG::dfgPath::cal_type_control(int t0, int) {
  return t0;
}

int SDFG::dfgPath::cal_type_data2control(int t0, int t1) {
  if(t0 == 0 || t1 == 0) return 0x0;
  return t1;
}

string SDFG::dfgPath::get_stype(int tt) {
  string stype("");
  if(tt == dfgEdge::SDFG_DF) stype += "DF|";
  if(tt & dfgEdge::SDFG_DDP) stype += "DDP|";
  if(tt & dfgEdge::SDFG_CAL) stype += "CAL|";
  if(tt & dfgEdge::SDFG_ASS) stype += "ASS|";
  if(tt & dfgEdge::SDFG_DAT) stype += "DAT|";
  if(tt & dfgEdge::SDFG_CTL) stype += "CTL|";
  if(tt & dfgEdge::SDFG_LOG) stype += "LOG|";
  if(tt & dfgEdge::SDFG_ADR) stype += "ADR|";
  if(tt & dfgEdge::SDFG_CMP) stype += "CMP|";
  if(tt & dfgEdge::SDFG_EQU) stype += "EQU|";
  if(tt & dfgEdge::SDFG_CLK) stype += "CLK|";
  if(tt & dfgEdge::SDFG_RST) stype += "RST|";
  assert(stype.size() > 1);
  stype.erase(stype.size()-1);
  return stype;
}
