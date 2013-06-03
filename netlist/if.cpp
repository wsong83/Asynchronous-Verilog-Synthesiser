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
 * If statements
 * 02/04/2012   Wei Song
 *
 *
 */

#include "component.h"
#include "shell/env.h"
#include <boost/foreach.hpp>
#include "sdfg/sdfg.hpp"
#include "sdfg/rtree.hpp"

using namespace netlist;
using std::ostream;
using std::string;
using std::list;
using std::map;
using std::vector;
using boost::shared_ptr;
using boost::static_pointer_cast;
using shell::location;

netlist::IfState::IfState() : NetComp(tIf), named(false) {}

netlist::IfState::IfState(const shell::location& lloc) : NetComp(tIf, lloc), named(false) {}

netlist::IfState::IfState(
                          const shared_ptr<Expression>& exp, 
                          const shared_ptr<Block>& m_ifcase, 
                          const shared_ptr<Block>& m_elsecase
                          )
  : NetComp(NetComp::tIf), exp(exp), ifcase(m_ifcase), elsecase(m_elsecase), named(false)
{
  ifcase->elab_inparse();
  elsecase->elab_inparse();
}

netlist::IfState::IfState(
                          const location& lloc,
                          const shared_ptr<Expression>& exp, 
                          const shared_ptr<Block>& m_ifcase, 
                          const shared_ptr<Block>& m_elsecase
                          )
  : NetComp(NetComp::tIf, lloc), exp(exp), ifcase(m_ifcase), elsecase(m_elsecase), named(false)
{
  ifcase->elab_inparse();
  elsecase->elab_inparse();
}

netlist::IfState::IfState(
                          const shared_ptr<Expression>& exp, 
                          const shared_ptr<Block>& m_ifcase
                          )
  : NetComp(NetComp::tIf), exp(exp),  ifcase(m_ifcase), named(false)
{
  ifcase->elab_inparse();
}

netlist::IfState::IfState(
                          const location& lloc,
                          const shared_ptr<Expression>& exp, 
                          const shared_ptr<Block>& m_ifcase
                          )
  : NetComp(NetComp::tIf, lloc), exp(exp),  ifcase(m_ifcase), named(false)
{
  ifcase->elab_inparse();
}

ostream& netlist::IfState::streamout(ostream& os, unsigned int indent) const {
  return streamout(os, indent, false);
}

void netlist::IfState::set_father(Block *pf) {
  if(father == pf) return;
  father = pf;
  exp->set_father(pf);
  ifcase->set_father(pf);
  if(elsecase) elsecase->set_father(pf);
}

ostream& netlist::IfState::streamout(ostream& os, unsigned int indent, bool fl_prefix) const {
  assert(exp);

  if(!fl_prefix) os << string(indent, ' ');
  
  os << "if (" << *exp << ") ";
  ifcase->streamout(os, indent, true);

  if(!elsecase) return os;
  os << string(indent, ' ') << "else ";
  elsecase->streamout(os, indent, true, true);
  
  return os;

}

IfState* netlist::IfState::deep_copy() const {
  IfState* rv = new IfState(loc);
  rv->name = name;
  rv->named = named;
  
  if(exp) rv->exp.reset(exp->deep_copy());
  if(ifcase) rv->ifcase.reset(ifcase->deep_copy());
  if(elsecase) rv->elsecase.reset(elsecase->deep_copy());

  return rv;
}

void netlist::IfState::db_register(int) {
  if(exp) exp->db_register(1);
  if(ifcase) ifcase->db_register(1);
  if(elsecase) elsecase->db_register(1);
}

void netlist::IfState::db_expunge() {
  if(exp) exp->db_expunge();
  if(ifcase) ifcase->db_expunge();
  if(elsecase) elsecase->db_expunge();
}

bool netlist::IfState::elaborate(std::set<shared_ptr<NetComp> >& to_del,
                                 map<shared_ptr<NetComp>, list<shared_ptr<NetComp> > >& to_add) {
  // elaborate the if condition expression
  exp->reduce();
  ifcase->elaborate(to_del, to_add);
  if(elsecase) elsecase->elaborate(to_del, to_add);

  // check whether it is already constant
  if(exp->is_valuable() && exp->get_value().is_false()) { // false
    if(elsecase) to_add[get_sp()].push_back(elsecase);
    to_del.insert(get_sp());
  } else if(exp->is_valuable() && exp->get_value().is_true()) { // true
    to_add[get_sp()].push_back(ifcase);
    to_del.insert(get_sp());
  }
  return true;
}

shared_ptr<SDFG::RTree> netlist::IfState::get_rtree() const {

  shared_ptr<SDFG::RTree> rv(new SDFG::RTree());
  map<string, unsigned int> target_count;

  shared_ptr<SDFG::RTree> ifcase_rtree = ifcase->get_rtree();
  BOOST_FOREACH(SDFG::RTree::sub_tree_type& t, ifcase_rtree->tree) {
    if(target_count.count(t.first)) target_count[t.first]++;
    else                            target_count[t.first] = 1;
  }
  rv->add_tree(ifcase_rtree);

  shared_ptr<SDFG::RTree> elsecase_rtree(new SDFG::RTree());
  if(elsecase) elsecase_rtree = elsecase->get_rtree();
  BOOST_FOREACH(SDFG::RTree::sub_tree_type& t, elsecase_rtree->tree) {
    if(target_count.count(t.first)) target_count[t.first]++;
    else                            target_count[t.first] = 1;
  }
  rv->add_tree(elsecase_rtree);

  rv->add_tree(exp->get_rtree(), SDFG::dfgEdge::SDFG_CTL);

  // add self paths
  typedef std::pair<const string&, unsigned int> m_type;
  BOOST_FOREACH(m_type t, target_count) {
    if(t.second < 2)
      rv->add_edge(t.first, t.first, SDFG::dfgEdge::SDFG_DDP);
  }

  return rv;
}

void netlist::IfState::replace_variable(const VIdentifier& var, const Number& num) {
  exp->replace_variable(var, num);
  ifcase->replace_variable(var, num);
  if(elsecase) elsecase->replace_variable(var, num);
}

shared_ptr<Expression> netlist::IfState::get_combined_expression(const VIdentifier& target, std::set<std::string> s_set) {
  shared_ptr<Expression> if_exp = ifcase->get_combined_expression(target, s_set);
  shared_ptr<Expression> else_exp;
  if(elsecase)  else_exp = elsecase->get_combined_expression(target, s_set);
  shared_ptr<Expression> rv;
  if(if_exp || else_exp) {
    if(if_exp) {
      if(else_exp) 
        rv.reset(exp->deep_copy()->append(Operation::oQuestion, *if_exp, *else_exp));
      else {
        Expression self_loop(target);
        rv.reset(exp->deep_copy()->append(Operation::oQuestion, *if_exp, self_loop));
      }
    } else {
      Expression self_loop(target);
      rv.reset(exp->deep_copy()->append(Operation::oQuestion, self_loop, *else_exp));      
    }
  }
  //std::cout << "IfState: (target)" << target << " Exp: " << *rv << std::endl;
  return rv;
}
