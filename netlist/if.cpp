/*
 * Copyright (c) 2011-2012 Wei Song <songw@cs.man.ac.uk> 
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

void netlist::IfState::scan_vars(std::set<string>& target,
                                 std::set<string>& dsrc,
                                 std::set<string>& csrc,
                                 bool ctl) const {
  exp->scan_vars(csrc, csrc, csrc, true);
  ifcase->scan_vars(target, dsrc, csrc, ctl);
  if(elsecase)
    elsecase->scan_vars(target, dsrc, csrc, ctl);
}

void netlist::IfState::gen_sdfg(shared_ptr<SDFG::dfgGraph> G, 
                                const std::set<string>& target,
                                const std::set<string>&,
                                const std::set<string>&) {
  std::set<string> t, d, c;     // local version
  scan_vars(t, d, c, false);

  // add control signals
  std::set<string> csig;
  exp->scan_vars(csig, csig, csig, true);
  BOOST_FOREACH(const string& m, target) {
    BOOST_FOREACH(const string& sig, csig) {
      if(!G->exist(sig, m, SDFG::dfgEdge::SDFG_CTL))
        G->add_edge(sig, SDFG::dfgEdge::SDFG_CTL, sig, m);
    }
  }

  // do the rest
  ifcase->gen_sdfg(G, t, d, c);
  if(elsecase)
    elsecase->gen_sdfg(G, t, d, c);
  else {                        // no else, self-loop
    BOOST_FOREACH(const string& m, t) {
      if(!G->exist(m, m, SDFG::dfgEdge::SDFG_DP))
        G->add_edge(m, SDFG::dfgEdge::SDFG_DP, m, m);
    }
  }
}

void netlist::IfState::replace_variable(const VIdentifier& var, const Number& num) {
  exp->replace_variable(var, num);
  ifcase->replace_variable(var, num);
  if(elsecase) elsecase->replace_variable(var, num);
}

