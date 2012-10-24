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
 * Case statements
 * 26/03/2012   Wei Song
 *
 *
 */

#include "component.h"
#include "shell/env.h"
#include <algorithm>
#include <set>
#include <boost/foreach.hpp>
#include "sdfg/sdfg.hpp"

using namespace netlist;
using std::ostream;
using std::endl;
using std::string;
using std::vector;
using boost::shared_ptr;
using std::list;
using std::for_each;

netlist::CaseItem::CaseItem(const shared_ptr<Expression>& exp, const shared_ptr<Block>& body)
  : NetComp(tCaseItem), body(body)
{
  exps.push_back(exp);
}

netlist::CaseItem::CaseItem(const shell::location& lloc, const shared_ptr<Expression>& exp, const shared_ptr<Block>& body)
  : NetComp(tCaseItem, lloc), body(body)
{
  exps.push_back(exp);
}

// normal default case
netlist::CaseItem::CaseItem(const shared_ptr<Block>& body)
  : NetComp(tCaseItem), body(body) {}

netlist::CaseItem::CaseItem(const shell::location& lloc, const shared_ptr<Block>& body)
  : NetComp(tCaseItem, lloc), body(body)
{
  body->elab_inparse();
}

// multiple expressions
netlist::CaseItem::CaseItem(const list<shared_ptr<Expression> >& expm, const shared_ptr<Block>& body)
  : NetComp(tCaseItem), exps(expm), body(body) {}

netlist::CaseItem::CaseItem(const shell::location& lloc, const list<shared_ptr<Expression> >& expm, const shared_ptr<Block>& body)
  : NetComp(tCaseItem, lloc), exps(expm), body(body) {}

ostream& netlist::CaseItem::streamout (ostream& os, unsigned int indent) const {
  // show the expressions
  if(is_default()) {
    os << string(indent, ' ');    // show the indent for each line
    os << "default: ";
  } else if(exps.size() == 1) {
    os << string(indent, ' ');    // show the indent for each line
    os << *(exps.front()) << ": ";
  } else {
    unsigned int ncase = exps.size(); // number of cases in the item
    list<shared_ptr<Expression> >::const_iterator it = exps.begin();
    for(unsigned int i=0; i< ncase-1; i++) {
      os << string(indent, ' ');    // show the indent for each line
      os << *(*it) << "," << endl;
      it++;
    }
    os << string(indent, ' ') << *(*it) << ": ";
  }
  
  // the body part
  body->streamout(os, indent, true);
  return os;
}

void netlist::CaseItem::set_father(Block *pf) {
  if(father == pf) return;
  father = pf;
  BOOST_FOREACH(shared_ptr<Expression>& it, exps)
    it->set_father(pf);

  if(body.use_count())
    body->set_father(pf);
}

CaseItem* netlist::CaseItem::deep_copy() const {
  CaseItem* rv = new CaseItem(loc);
  if(body) rv->body.reset(body->deep_copy());
  BOOST_FOREACH(const  shared_ptr<Expression>& m, exps)
    rv->exps.push_back(shared_ptr<Expression>(m->deep_copy()));
  return rv;
}

void netlist::CaseItem::db_register(int) {
  BOOST_FOREACH(shared_ptr<Expression>& m, exps) m->db_register(1);
  if(body) body->db_register(1);
}

void netlist::CaseItem::db_expunge() {
  BOOST_FOREACH(shared_ptr<Expression>& m, exps) m->db_expunge();
  if(body) body->db_expunge();
}

bool netlist::CaseItem::elaborate(set<shared_ptr<Variable> >& to_del,
                                  map<shared_ptr<NetComp>, list<shared_ptr<Variable> > >& to_add) {

  // check all expressions are const expressions
  BOOST_FOREACH(shared_ptr<Expression>& m, exps) 
    rv &= m->elaborate(to_del, to_add);
  if(!rv) return false;

  // check the case body
  if(body) rv &= body->elaborate(to_del, to_add);

  return rv;
}

void netlist::CaseItem::scan_vars(std::set<string>& target,
                                  std::set<string>& dsrc,
                                  std::set<string>& csrc,
                                  bool ctl) const {
  body->scan_vars(target, dsrc, csrc, ctl);
}

void netlist::CaseItem::gen_sdfg(shared_ptr<SDFG::dfgGraph> G, 
                                 const std::set<string>& target,
                                 const std::set<string>&,
                                 const std::set<string>&) {
  std::set<string> t, d, c;     // local version
  scan_vars(t, d, c, false);
  
  // for all targets not in t, there is a self-loop
  if(t.size() < target.size()) { // self loop
    BOOST_FOREACH(const string& m, target) {
      if(!t.count(m)) {         // the signal to have self-loop
        if(!G->exist(m, m, SDFG::dfgEdge::SDFG_DP)) 
          G->add_edge(m, SDFG::dfgEdge::SDFG_DP, m, m);
      }
    }
  }
  
  body->gen_sdfg(G, t, d, c);
}

void netlist::CaseItem::replace_variable(const VIdentifier& var, const Number& num) {
  BOOST_FOREACH(shared_ptr<Expression> e, exps) {
    e->replace_variable(var, num);
  }
  if(body) body->replace_variable(var, num);
}

bool netlist::CaseItem::is_match(const Number& val) const {
  bool rv = false;
  if(exps.size() == 0) return true;                // default

  BOOST_FOREACH(const shared_ptr<Expression>& m, exps) {
    assert(m->is_valuable());
    rv |= (m->get_value() == val);
  }
  return rv;
}

netlist::CaseState::CaseState(const shared_ptr<Expression>& exp, const list<shared_ptr<CaseItem> >& citems, 
                              const shared_ptr<CaseItem>& ditem, bool mcasex)
  : NetComp(tCase), exp(exp), cases(citems), named(false), casex(mcasex) 
{
  cases.push_back(ditem);
}

netlist::CaseState::CaseState(const shell::location& lloc, const shared_ptr<Expression>& exp, 
                              const list<shared_ptr<CaseItem> >& citems, const shared_ptr<CaseItem>& ditem, 
                              bool mcasex)
  : NetComp(tCase, lloc), exp(exp), cases(citems), named(false), casex(mcasex) 
{
  cases.push_back(ditem);
}

netlist::CaseState::CaseState(const shared_ptr<Expression>& exp, const list<shared_ptr<CaseItem> >& citems, 
                              bool mcasex)
  : NetComp(tCase), exp(exp), cases(citems), named(false), casex(mcasex) {} 

netlist::CaseState::CaseState(const shell::location& lloc, const shared_ptr<Expression>& exp, 
                              const list<shared_ptr<CaseItem> >& citems, bool mcasex)
  : NetComp(tCase, lloc), exp(exp), cases(citems), named(false), casex(mcasex) {} 

netlist::CaseState::CaseState(const shared_ptr<Expression>& exp, const shared_ptr<CaseItem>& ditem, 
                              bool mcasex)
  : NetComp(tCase), exp(exp), named(false), casex(mcasex) 
{
  cases.push_back(ditem);
}

netlist::CaseState::CaseState(const shell::location& lloc, const shared_ptr<Expression>& exp, 
                              const shared_ptr<CaseItem>& ditem, bool mcasex)
  : NetComp(tCase, lloc), exp(exp), named(false), casex(mcasex) 
{
  cases.push_back(ditem);
}

ostream& netlist::CaseState::streamout (ostream& os, unsigned int indent) const {
  os << string(indent, ' ');
  if(casex) os << "casex(" << *exp << ")" << endl;
  else      os << "case("  << *exp << ")" << endl;
  BOOST_FOREACH(const shared_ptr<CaseItem>& it, cases)
    it->streamout(os, indent+2);
  os << string(indent, ' ') << "endcase" << endl;
  return os;
}

void netlist::CaseState::set_father(Block *pf) {
  if(father == pf) return;
  father = pf;
  name.set_father(pf);
  exp->set_father(pf);
  BOOST_FOREACH(shared_ptr<CaseItem>& it, cases)
    it->set_father(pf);
}

CaseState* netlist::CaseState::deep_copy() const {
  CaseState* rv = new CaseState(loc);
  rv->name = name;
  rv->named = named;
  rv->casex = casex;
  if(exp) rv->exp.reset(exp->deep_copy());
  BOOST_FOREACH(const shared_ptr<CaseItem>& m, cases)
    rv->cases.push_back(shared_ptr<CaseItem>(m->deep_copy()));
  return rv;
}

void netlist::CaseState::db_register(int) {
  BOOST_FOREACH(shared_ptr<CaseItem>& m, cases) m->db_register(1);
  if(exp) exp->db_register(1);
}

void netlist::CaseState::db_expunge() {
  BOOST_FOREACH(shared_ptr<CaseItem>& m, cases) m->db_expunge();
  if(exp) exp->db_expunge();
}

bool netlist::CaseState::elaborate(set<shared_ptr<Variable> >& to_del,
                                   map<shared_ptr<NetComp>, list<shared_ptr<Variable> > >& to_add) {
  bool rv = true;
  
  //elaborate the case expression
  assert(exp.use_count() != 0);
  rv &= exp->elaborate(result, mctype, fp);
  if(!rv) return false;

  // elaborate all case items
  BOOST_FOREACH(shared_ptr<CaseItem>& m, cases)
    rv &= m->elaborate(result, mctype);
  if(!rv) return false;

  // post-elaborate process
  
  // check the number of default and make sure it is at the end
  list<shared_ptr<CaseItem> >::iterator it, end;
  for(it=cases.begin(), end=cases.end(); it!=end; it++) {
    if((*it)->is_default()) {
      it++;
      if(it != end) {           // still have case items after a default case
        G_ENV->error((*it)->loc, "ELAB-CASE-1");
        cases.erase(it, end);
      }
      break;
    }
  }

  // check all cases have different values and remove duplicated cases
  // #define CASE_DEBUG_CASE
#ifdef CASE_DEBUG_CASE
  std::cout << "beginning check case statements: " << std::endl;
#endif

  bool already_has_default = false;
  std::set<string> case_exps;
  it=cases.begin();
  end=cases.end();
  while(it!=end) {
    if(!already_has_default && (*it)->is_default()) {
      it++;
      already_has_default = true;
      continue;
    }

    // check all case expressions
    list<shared_ptr<Expression> >::iterator eit, eend;
    eit=(*it)->exps.begin(); 
    eend=(*it)->exps.end(); 
    while(eit!=eend) {
      if(case_exps.count(Number::trim_zeros((*eit)->get_value().get_txt_value()))) { // duplicated
#ifdef CASE_DEBUG_CASE
        std::cout << "duplicated " << **eit << " text: " 
                  << Number::trim_zeros((*eit)->get_value().get_txt_value())  
                  << std::endl;
#endif
        G_ENV->error((*eit)->loc, "ELAB-CASE-2");
        eit = (*it)->exps.erase(eit);
      } else {
#ifdef CASE_DEBUG_CASE
        std::cout << "insert " << **eit << " text: " 
                  << Number::trim_zeros((*eit)->get_value().get_txt_value())  
                  << std::endl;
#endif
        case_exps.insert(Number::trim_zeros((*eit)->get_value().get_txt_value()));
        eit++;
      }
    }

    if((*it)->is_default()) {
      it = cases.erase(it);
    } else {
      it++;
    }
  } 

  if(cases.size() == 0) {       // empty case
    result = ELAB_Empty;
    return rv;
  }

  if(exp->is_valuable()) {      // const case condition
    Number exp_val = exp->get_value();
    // find the case item
    for(it=cases.begin(), end=cases.end(); it!=end; it++) {
      if((*it)->is_match(exp_val)) {
        cases.push_front(shared_ptr<CaseItem>(new CaseItem((*it)->loc, (*it)->body)));
        break;
      }
    }
    if(it == end) {             // no match at all
      result = ELAB_Empty;
      return rv;
    } else {
      result = ELAB_Const_Case;
      return rv;
    }
  }

  if(cases.size() == 1 || (cases.size() == 2 && cases.back()->is_default())) {       // only one case item
    // convert it into an if
    result = ELAB_To_If_Case;
    return rv;
  }

  return rv;

} 

void netlist::CaseState::scan_vars(std::set<string>& target,
                                   std::set<string>& dsrc,
                                   std::set<string>& csrc,
                                   bool ctl) const {
  exp->scan_vars(csrc, csrc, csrc, true);
  BOOST_FOREACH(const shared_ptr<CaseItem>& m, cases) {
    m->scan_vars(target, dsrc, csrc, ctl);
  }
  
}

void netlist::CaseState::gen_sdfg(shared_ptr<SDFG::dfgGraph> G, 
                              const std::set<string>& target,
                              const std::set<string>&,
                              const std::set<string>&) {
  
  std::set<string> t, d, c;     // local version
  scan_vars(t, d, c, false);
  
  // add control signals
  BOOST_FOREACH(const string& m, target) {
    BOOST_FOREACH(const string& csig, c) {
      if(!G->exist(csig, m, SDFG::dfgEdge::SDFG_CTL))
        G->add_edge(csig, SDFG::dfgEdge::SDFG_CTL, csig, m);
    }
  }

  // the case items
  BOOST_FOREACH(shared_ptr<CaseItem>& m, cases) {
    m->gen_sdfg(G, t, d, c);
  }

  // check whether there is a default
  // do not do it now
}

void netlist::CaseState::replace_variable(const VIdentifier& var, const Number& num) {
  exp->replace_variable(var, num);
  BOOST_FOREACH(shared_ptr<CaseItem> ci, cases) {
    ci->replace_variable(var, num);
  }
}

