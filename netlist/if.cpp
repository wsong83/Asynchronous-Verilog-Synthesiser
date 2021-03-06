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

IfState* netlist::IfState::deep_copy(NetComp* bp) const {
  IfState *rv;
  if(!bp) rv = new IfState(loc);
  else    rv = static_cast<IfState *>(bp); // C++ does not support multiple dispatch
  NetComp::deep_copy(rv);

  rv->name = name;
  rv->named = named;
  
  if(exp) rv->exp.reset(exp->deep_copy(NULL));
  if(ifcase) rv->ifcase.reset(ifcase->deep_copy(NULL));
  if(elsecase) rv->elsecase.reset(elsecase->deep_copy(NULL));

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

SDFG::RForest netlist::IfState::get_rforest() const {

  // build the forest
  SDFG::RForest rv, if_rt, else_rt;
  if_rt = ifcase->get_rforest();
  if(elsecase)
    else_rt = elsecase->get_rforest();

  rv.combine(if_rt);
  rv.combine(else_rt);

  // self-loop
  SDFG::sig_map smap = rv.get_target_signals();
  if(elsecase) {
    list<SDFG::sig_map> if_smap;
    if_smap.push_back(if_rt.get_target_signals());
    if_smap.push_back(else_rt.get_target_signals());
    BOOST_FOREACH(SDFG::sig_map smap_item, if_smap) {
      for(SDFG::sig_map::iterator it=smap.begin(); it!=smap.end(); ++it) {
        if(smap_item.count(it->first)) {
          if(smap_item[it->first].proper_subset(it->second)) {
            SDFG::dfgRangeMap r = it->second.complement(smap_item[it->first]);
            SDFG::RTree mt(it->first, r);
            mt.add(SDFG::RRelation(it->first, r, SDFG::dfgEdge::SDFG_DDP));
            rv.add(mt);
          }
        } else {
          SDFG::dfgRangeMap r = it->second;
          SDFG::RTree mt(it->first, r);
          mt.add(SDFG::RRelation(it->first, r, SDFG::dfgEdge::SDFG_DDP));
          rv.add(mt);
        }
      }
    }
  } else {
    for(SDFG::sig_map::iterator it=smap.begin(); it!=smap.end(); ++it) {
      SDFG::dfgRangeMap r = it->second;
      SDFG::RTree mt(it->first, r);
      mt.add(SDFG::RRelation(it->first, r, SDFG::dfgEdge::SDFG_DDP));
      rv.add(mt);
    } 
  }

  SDFG::RTree exp_rt = exp->get_rtree();
  exp_rt.assign_type(SDFG::dfgEdge::SDFG_LOG);
  for(SDFG::RForest::iterator it = rv.begin(); it != rv.end(); ++it) {
    it->second.combine(exp_rt);
  }

  return rv;
}

void netlist::IfState::replace_variable(const VIdentifier& var, const Number& num) {
  exp->replace_variable(var, num);
  ifcase->replace_variable(var, num);
  if(elsecase) elsecase->replace_variable(var, num);
}

void netlist::IfState::replace_variable(const VIdentifier& var, const VIdentifier& nvar) {
  exp->replace_variable(var, nvar);
  ifcase->replace_variable(var, nvar);
  if(elsecase) elsecase->replace_variable(var, nvar);
}

shared_ptr<Expression> netlist::IfState::get_combined_expression(const VIdentifier& target, std::set<std::string> s_set) {
  shared_ptr<Expression> if_exp = ifcase->get_combined_expression(target, s_set);
  shared_ptr<Expression> else_exp;
  if(elsecase)  else_exp = elsecase->get_combined_expression(target, s_set);
  shared_ptr<Expression> rv;
  if(if_exp || else_exp) {
    if(if_exp) {
      if(else_exp) 
        rv.reset(exp->deep_copy(NULL)->append(Operation::oQuestion, *if_exp, *else_exp));
      else {
        Expression self_loop(target);
        rv.reset(exp->deep_copy(NULL)->append(Operation::oQuestion, *if_exp, self_loop));
      }
    } else {
      Expression self_loop(target);
      rv.reset(exp->deep_copy(NULL)->append(Operation::oQuestion, self_loop, *else_exp));      
    }
  }
  return rv;
}

shared_ptr<Block> netlist::IfState::unfold() {
  // unfold the childs
  ifcase = ifcase->unfold();
  if(elsecase) elsecase = elsecase->unfold();

  // reduce the statement
  exp->reduce();

  if(exp->is_valuable()) {
    if(exp->get_value().is_false()) // false
      return elsecase;
    else
      return ifcase;
  } 

  return shared_ptr<Block>();
}
