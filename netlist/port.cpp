/*
 * Copyright (c) 2012-2014 Wei Song <songw@cs.man.ac.uk> 
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
 * Ports
 * 16/02/2012   Wei Song
 *
 *
 */

#include "component.h"
#include "shell/env.h"
#include "sdfg/sdfg.hpp"
#include "sdfg/rtree.hpp"
#include <boost/foreach.hpp>

using namespace netlist;
using std::ostream;
using std::endl;
using std::string;
using boost::shared_ptr;
using shell::location;
using std::vector;
using std::map;
using std::list;

netlist::Port::Port(const VIdentifier& pid)
  : NetComp(tPort), name(*(pid.deep_copy(NULL))), ptype(0), dir(-2), signed_flag(false) {}

netlist::Port::Port(const location& lloc, const VIdentifier& pid)
  : NetComp(tPort, lloc), name(*(pid.deep_copy(NULL))), ptype(0), dir(-2), signed_flag(false) {}

netlist::Port::Port(const location& lloc)
  : NetComp(tPort, lloc), ptype(0), dir(-2), signed_flag(false) {}

void netlist::Port::set_father(Block *pf) {
  if(father == pf) return;
  father = pf;
  name.set_father(pf);
}

void netlist::Port::db_register(int) {
  if(dir <= 0) name.db_register(0);
  if(dir >= 0) name.db_register(1);
}

void netlist::Port::db_expunge() {
  name.db_expunge();
}

bool netlist::Port::elaborate(std::set<shared_ptr<NetComp> >&,
                              map<shared_ptr<NetComp>, list<shared_ptr<NetComp> > >&) {
  name.reduce();
  return true;
}

shared_ptr<Expression> netlist::Port::get_combined_expression(const VIdentifier& target, std::set<string> s_set) {
  shared_ptr<SDFG::dfgNode> pnode = get_module()->DFG->get_node(target.get_name());
  shared_ptr<Expression> rv(new Expression(target));
  assert(pnode);
  bool found_source = false;
  while(!found_source) {
    assert(pnode);
    //std::cout << pnode->get_full_name() << std::endl;
    switch(pnode->type) {
    case SDFG::dfgNode::SDFG_DF: {
      if(pnode->ptr.size() > 1) {
        found_source = true;
      } else {
        pnode = (pnode->pg->get_in_nodes_cb(pnode)).front();
      }
      break;
    }
    case SDFG::dfgNode::SDFG_COMB:
    case SDFG::dfgNode::SDFG_FF: {
      found_source = true;
      break;
    }
    case SDFG::dfgNode::SDFG_IPORT: 
    case SDFG::dfgNode::SDFG_OPORT:   {
      if((pnode->pg->get_in_nodes_cb(pnode)).size()) {
        pnode = (pnode->pg->get_in_nodes_cb(pnode)).front(); // get the source from the higher hierarchy
      } else {
        found_source = true; // top level
      }
      break;
    }
    default:
      assert(0 == "wrong type");
    }
  }
  assert(pnode);
  if(pnode->type & SDFG::dfgNode::SDFG_LATCH) {
    G_ENV->error("ANA-SSA-1", pnode->get_full_name(), get_module()->DFG->get_node(target.get_name())->get_full_name());
  }
  if(s_set.count(pnode->get_full_name())) {
    G_ENV->error("ANA-SSA-2", pnode->get_full_name());
  } else {
    s_set.insert(pnode->get_full_name());
    //std::cout << pnode->get_full_name() << std::endl;
    if(!(pnode->type & (SDFG::dfgNode::SDFG_FF | SDFG::dfgNode::SDFG_LATCH | SDFG::dfgNode::SDFG_PORT))) {
      shared_ptr<Expression> sig_exp;
      BOOST_FOREACH(shared_ptr<NetComp> ncomp, pnode->ptr) {
        if(ncomp->ctype != tVariable) {
          //std::cout << *ncomp << std::endl;
          //std::cout << pnode->name << std::endl;
          sig_exp = ncomp->get_combined_expression(VIdentifier(pnode->name), s_set);
          //std::cout << *sig_exp << std::endl;
          break;
        }
      }
      assert(sig_exp);
      rv->replace_variable(target, sig_exp);
    }
  }
  return rv;
}

Port* netlist::Port::deep_copy(Port *rv) const {
  if(!rv) rv = new Port(loc);
  NetComp::deep_copy(rv);
  VIdentifier *mname = name.deep_copy(NULL);
  rv->name = *mname;
  delete mname;
  rv->dir = dir;
  rv->ptype = ptype;
  rv->signed_flag = signed_flag;
  return rv;
}      

ostream& netlist::Port::streamout(ostream& os, unsigned int indent) const {
  os << string(indent, ' ');
  if(is_in())
    os << "input ";
  else if(is_out())
    os << "output ";
  else if(is_inout())
    os << "inout ";
  else
    os << "UNKOWN_port ";

  if(signed_flag) os << "signed ";
  name.get_range().RangeArrayCommon::streamout(os, 0, "", true, false); // show range of declaration 
  name.get_range().RangeArrayCommon::streamout(os, 1, name.get_name(), true, true); // show dimension of declaration
  os << ";" << endl;

  return os;
}
