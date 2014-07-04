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
 * Block/non-block assignments
 * 23/02/2012   Wei Song
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
using shell::location;
using std::string;
using std::list;
using std::vector;
using boost::shared_ptr;
using std::map;

netlist::Assign::Assign( const shared_ptr<LConcatenation>& lhs,
                         const shared_ptr<Expression>& rhs,
                         bool b
                         )
  : NetComp(tAssign), lval(lhs), rexp(rhs), blocking(b), continuous(false), named(false)
{}

netlist::Assign::Assign( const location& lloc,
                         const shared_ptr<LConcatenation>& lhs,
                         const shared_ptr<Expression>& rhs,
                         bool b
                         )
  : NetComp(tAssign, lloc), lval(lhs), rexp(rhs), blocking(b), continuous(false), named(false)
{}

ostream& netlist::Assign::streamout(ostream& os, unsigned int indent) const {
  os << string(indent, ' ');
  if(continuous) os << "assign ";
  os << *lval;
  if(blocking) os << " = ";
  else os << " <= ";
  os << *rexp;
  if(continuous) os << ";" << endl;
  return os;
}

void netlist::Assign::set_father(Block *pf) {
  if(father == pf) return;
  father = pf;
  name.set_father(pf);
  lval->set_father(pf);
  rexp->set_father(pf);
}

void netlist::Assign::db_register(int) { 
  lval->db_register(0); 
  rexp->db_register(1); 
}

void netlist::Assign::db_expunge() { 
  lval->db_expunge(); 
  rexp->db_expunge(); 
}

bool netlist::Assign::elaborate(std::set<shared_ptr<NetComp> >&,
                                map<shared_ptr<NetComp>, list<shared_ptr<NetComp> > >&) {
  lval->reduce();
  rexp->reduce();
  return true;
}

void netlist::Assign::gen_sdfg(shared_ptr<SDFG::dfgGraph> G) {
  shared_ptr<SDFG::RTree> ass_tree = get_rtree();
  BOOST_FOREACH(SDFG::RTree::sub_tree_type& t, ass_tree->tree) {
    assert(t.first != SDFG::RTree::DTarget);
    if(!G->exist(SDFG::divide_signal_name(t.first)))
      G->add_node(t.first, SDFG::dfgNode::SDFG_DF);
    BOOST_FOREACH(SDFG::RTree::rtree_edge_type& e, t.second) {
      if(!G->exist(SDFG::divide_signal_name(e.first)))
        G->add_node(e.first, SDFG::dfgNode::SDFG_DF);
      G->add_edge_multi(e.first, e.second, e.first, t.first);
    }
    G->get_node(SDFG::divide_signal_name(t.first))->ptr.insert(get_sp());
  }
}

shared_ptr<SDFG::RTree> netlist::Assign::get_rtree() const {
  shared_ptr<SDFG::RTree> lrf = lval->get_rtree();
  shared_ptr<SDFG::RTree> rrf = rexp->get_rtree();
  lrf->combine(rrf);
  return lrf;
}

shared_ptr<Block> netlist::Assign::unfold() {
  return shared_ptr<Block>();
}

Assign* netlist::Assign::deep_copy(NetComp* bp) const {
  Assign *rv;
  if(!bp) {
    rv =  new Assign( loc,
                      shared_ptr<LConcatenation>(lval->deep_copy(NULL)),
                      shared_ptr<Expression>(rexp->deep_copy(NULL)),
                      blocking
                      );
  } else {
    rv = static_cast<Assign *>(bp); // C++ does not support multiple dispatch
    rv->lval.reset(lval->deep_copy(NULL));
    rv->rexp.reset(rexp->deep_copy(NULL));
    rv->blocking = blocking;
  }
  NetComp::deep_copy(rv);
  rv->name = name;
  rv->named = named;
  rv->continuous = continuous;
  return rv;
}

void netlist::Assign::replace_variable(const VIdentifier& var, const Number& num) {
  lval->replace_variable(var, num);
  rexp->replace_variable(var, num);
}

void netlist::Assign::replace_variable(const VIdentifier& var, const VIdentifier& nvar) {
  lval->replace_variable(var, nvar);
  rexp->replace_variable(var, nvar);
}

shared_ptr<Expression> netlist::Assign::get_combined_expression(const VIdentifier& target, std::set<string> s_set) {
  shared_ptr<SDFG::RTree> rt = get_rtree();
  shared_ptr<Expression> rv;
  if(rt->tree.count(target.get_name())) {
    //std::cout << *this << std::endl;
    rv.reset(rexp->deep_copy(NULL));
    // handle all signals in the expression
    std::set<string> sig_set = rt->get_all(target.get_name());
    BOOST_FOREACH(const string& sname, sig_set) {
      if(sname != target.get_name()) { // other signals
        //std::cout << sname << std::endl;
        shared_ptr<SDFG::dfgNode> pnode = get_module()->DFG->get_node(SDFG::divide_signal_name(sname));
        assert(pnode);
        bool found_source = false;
        while(!found_source) {
          assert(pnode);
          //std::cout << pnode->get_full_name() << std::endl;
          switch(pnode->type) {
          case SDFG::dfgNode::SDFG_DF: {
            if(pnode->ptr.size() > 1) {
              //std::cout << pnode->get_full_name() << std::endl;
              found_source = true;
            } else {
              pnode = (pnode->pg->get_in_nodes_cb(pnode)).front();
            }
            break;
          }
          case SDFG::dfgNode::SDFG_COMB:
          case SDFG::dfgNode::SDFG_FF:
          case SDFG::dfgNode::SDFG_LATCH: {
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
          G_ENV->error("ANA-SSA-1", pnode->get_full_name(), get_module()->DFG->get_node(SDFG::divide_signal_name(sname))->get_full_name());
        }
        if(s_set.count(pnode->get_full_name())) {
          G_ENV->error("ANA-SSA-2", pnode->get_full_name());
        } else {
          std::set<string> m_set = s_set;
          m_set.insert(pnode->get_full_name());
          
          if(!(pnode->type & (SDFG::dfgNode::SDFG_FF | SDFG::dfgNode::SDFG_LATCH | SDFG::dfgNode::SDFG_PORT))) {
            shared_ptr<Expression> sig_exp;
            BOOST_FOREACH(shared_ptr<NetComp> ncomp, pnode->ptr) {
              if(ncomp->ctype != tVariable) {
                //std::cout << *ncomp << std::endl;
                //std::cout << pnode->name << std::endl;
                sig_exp = ncomp->get_combined_expression(VIdentifier(pnode->name), m_set);
                //std::cout << *sig_exp << std::endl;
                break;
              }
            }
            assert(sig_exp);
            rv->replace_variable(VIdentifier(sname), sig_exp);
          }
        }
      }
    }
  }
  return rv;
}
