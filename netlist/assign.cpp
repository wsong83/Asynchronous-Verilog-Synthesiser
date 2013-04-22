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
  shared_ptr<SDFG::RForest> rf(new SDFG::RForest());
  scan_vars(rf, false);
  //std::cout << "assign ------->" << *this << std::endl;
  //rf->write(std::cout);
  
  BOOST_FOREACH(SDFG::RForest::tree_map_type& t, rf->tree) {
    std::set<string> csig = rf->get_control(t.first);
    std::set<string> dsig = rf->get_data(t.first);
    BOOST_FOREACH(const string& s, csig) {
      G->add_edge(s, SDFG::dfgEdge::SDFG_CTL, s, t.first);
    }
    BOOST_FOREACH(const string& s, dsig) {
      G->add_edge(s, SDFG::dfgEdge::SDFG_DP, s, t.first);
    }
    G->get_node(t.first)->ptr.insert(get_sp());
  }  
}

void netlist::Assign::scan_vars(shared_ptr<SDFG::RForest> rf, bool) const {
  shared_ptr<SDFG::RForest> lrf(new SDFG::RForest());
  shared_ptr<SDFG::RForest> rrf(new SDFG::RForest());
  lval->scan_vars(lrf, false);
  rexp->scan_vars(rrf, false);
  //std::cout << "rhs expression ------->" << *rexp << std::endl;
  //rrf->write(std::cout);
  shared_ptr<SDFG::RForest> crf(new SDFG::RForest());
  crf->build(lrf, rrf);
  rf->add(crf);
  //std::cout << *this << std::endl;
  //rf->write(std::cout);
}

Assign* netlist::Assign::deep_copy() const {
  Assign* rv = new Assign( loc,
                           shared_ptr<LConcatenation>(lval->deep_copy()),
                           shared_ptr<Expression>(rexp->deep_copy()),
                           blocking
                           );
  rv->name = name;
  rv->named = named;
  rv->continuous = continuous;
  return rv;
}

void netlist::Assign::replace_variable(const VIdentifier& var, const Number& num) {
  lval->replace_variable(var, num);
  rexp->replace_variable(var, num);
}

shared_ptr<Expression> netlist::Assign::get_combined_expression(const VIdentifier& target, std::set<string> s_set) {
  shared_ptr<SDFG::RForest> lrf(new SDFG::RForest());
  shared_ptr<SDFG::RForest> rrf(new SDFG::RForest());
  lval->scan_vars(lrf, false);
  rexp->scan_vars(rrf, false);
  shared_ptr<Expression> rv;
  if(lrf->tree.count(target.name)) {
    rv.reset(rexp->deep_copy());
    // handle all signals in the expression
    //if(s_set.size() < MAX_LEVEL_OF_COMBI_EXP && rrf->tree["@DATA"]) {
    if(rrf->tree["@DATA"]) {
      //std::cout << "the size of s_set " << s_set.size() << std::endl;
      BOOST_FOREACH(const string& sname, rrf->tree["@DATA"]->sig) {
        if(sname != target.name) { // other signals
          shared_ptr<SDFG::dfgNode> pnode = get_module()->DFG->get_node(sname);
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
            G_ENV->error("ANA-SSA-1", pnode->get_full_name(), get_module()->DFG->get_node(sname)->get_full_name());
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
  }
  return rv;
}
