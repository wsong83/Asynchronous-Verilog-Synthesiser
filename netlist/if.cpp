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

using namespace netlist;
using std::ostream;
using std::string;
using std::vector;
using boost::shared_ptr;
using boost::static_pointer_cast;
using shell::location;

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
  if(elsecase.use_count() != 0) elsecase->set_father(pf);
}

bool netlist::IfState::check_inparse() {
  bool rv = true;
  rv &= exp->check_inparse();
  rv &= ifcase->check_inparse();
  if(elsecase.use_count() != 0) rv &= elsecase->check_inparse();
  return rv;
}

ostream& netlist::IfState::streamout(ostream& os, unsigned int indent, bool fl_prefix) const {
  assert(exp.use_count() != 0);

  if(!fl_prefix) os << string(indent, ' ');
  
  os << "if (" << *exp << ") ";
  ifcase->streamout(os, indent, true);

  if(elsecase.use_count() == 0) return os;

  os << string(indent, ' ') << "else ";
  if(!elsecase->is_blocked() && (elsecase->front()->get_type() == NetComp::tIf)) {
    static_pointer_cast<IfState>(elsecase->front())->streamout(os, indent, true);
  } else
    elsecase->streamout(os, indent, true);
  
  return os;

}

IfState* netlist::IfState::deep_copy() const {
  IfState* rv = new IfState(loc);
  rv->name = name;
  rv->named = named;
  
  if(exp.use_count() != 0) rv->exp.reset(exp->deep_copy());
  if(ifcase.use_count() != 0) rv->ifcase.reset(ifcase->deep_copy());
  if(elsecase.use_count() != 0) rv->elsecase.reset(elsecase->deep_copy());

  return rv;
}

void netlist::IfState::db_register(int) {
  if(exp.use_count() != 0) exp->db_register(1);
  if(ifcase.use_count() != 0) ifcase->db_register(1);
  if(elsecase.use_count() != 0) elsecase->db_register(1);
}

void netlist::IfState::db_expunge() {
  if(exp.use_count() != 0) exp->db_expunge();
  if(ifcase.use_count() != 0) ifcase->db_expunge();
  if(elsecase.use_count() != 0) elsecase->db_expunge();
}

bool netlist::IfState::elaborate(elab_result_t &result, const ctype_t mctype, const vector<NetComp *>& fp) {
  bool rv = true;
  result = ELAB_Normal;
  
  // check the father component
  if(!(
       mctype == tGenBlock ||      // an if statement can be defined in a generate block
       mctype == tSeqBlock         // an if statement can be defined in a sequential block
       )) {
    G_ENV->error(loc, "ELAB-IF-0");
    return false;
  }

  // elaborate the if condition expression
  assert(exp.use_count() != 0);
  rv &= exp->elaborate(result, mctype, fp);
  if(!rv) return rv;

  // check whether it is already constant
  if(exp->is_valuable() && exp->get_value() == 0) { // false
    ifcase.reset();
    if(elsecase.use_count() != 0) rv &= elsecase->elaborate(result, mctype, fp);
    if(elsecase.use_count() == 0 || result == ELAB_Empty) {
      elsecase.reset();
      result = ELAB_Empty;      // tell the upper-level to remove this
    } else {
      ifcase = elsecase;
      elsecase.reset();
      exp.reset(new Expression(Number(1))); // move else case to if case and reset condition
      result = ELAB_Const_If;               // tell the upper-level to reduce it
    }
  } else if(exp->is_valuable() && exp->get_value() != 0) { // true
    elsecase.reset();
    if(ifcase.use_count() != 0) rv &= ifcase->elaborate(result, mctype, fp);
    if(ifcase.use_count() == 0 || result == ELAB_Empty) {
      ifcase.reset();
      result = ELAB_Empty;      // tell the upper-level to remove this
    } else {
      exp.reset(new Expression(Number(1))); // directly set the condition to the easist form
      result = ELAB_Const_If;               // tell the upper-level to reduce it
    }
  } else if(exp->is_valuable()) { // x or z
    assert(0 == "x or z in the if condition expression!");
  } else {
    elab_result_t result_if    = ELAB_Normal;
    elab_result_t result_else  = ELAB_Normal;
    if(ifcase.use_count() != 0)    rv &= ifcase->elaborate(result_if, mctype, fp);
    if(elsecase.use_count() != 0)  rv &= elsecase->elaborate(result_else, mctype, fp);
    if(rv) {                    // pose elab process
      if(result_if == ELAB_Empty) ifcase.reset();
      if(result_else == ELAB_Empty) elsecase.reset();
      if(ifcase.use_count() == 0 && elsecase.use_count() == 0)
        result = ELAB_Empty;
      else
        result = ELAB_Normal;
    }
  }

  return rv;
}

void netlist::IfState::set_always_pointer(SeqBlock *p) {
  if(ifcase.use_count() != 0) ifcase->set_always_pointer(p);
  if(elsecase.use_count() != 0) elsecase->set_always_pointer(p);
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
                                std::set<string>& target,
                                std::set<string>& dsrc,
                                std::set<string>& csrc) {
  std::set<string> t, d, c;     // local version
  scan_vars(t, d, c, false);

  if(t.size() < target.size()) { // self loop
    BOOST_FOREACH(const string& m, target) {
      if(!t.count(m)) {         // the signal to have self-loop
        if(!G->exist(m, m, SDFG::dfgEdge::SDFG_DATA)) 
          G->add_edge(m, SDFG::dfgEdge::SDFG_DATA, m, m);
      }
    }
  }

}
