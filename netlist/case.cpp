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
#include "sdfg/rtree.hpp"
#include "sdfg/sdfg.hpp"

using namespace netlist;
using std::ostream;
using std::endl;
using std::string;
using std::vector;
using boost::shared_ptr;
using std::list;
using std::map;
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

  if(body)
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

bool netlist::CaseItem::elaborate(std::set<shared_ptr<NetComp> >& to_del,
                                  map<shared_ptr<NetComp>, list<shared_ptr<NetComp> > >& to_add) {

  // check all expressions are const expressions
  BOOST_FOREACH(shared_ptr<Expression>& m, exps) 
    m->reduce();

  // check the case body
  if(body) 
    if(!body->elaborate(to_del, to_add))
      return false;

  return true;
}

void netlist::CaseItem::scan_vars(shared_ptr<SDFG::RForest> rf, bool ctl) const {
  body->scan_vars(rf, ctl);
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

bool netlist::CaseState::elaborate(std::set<shared_ptr<NetComp> >& to_del,
                                   map<shared_ptr<NetComp>, list<shared_ptr<NetComp> > >& to_add) {
  exp->reduce();

  // elaborate all case items
  BOOST_FOREACH(shared_ptr<CaseItem>& m, cases)
    if(!m->elaborate(to_del, to_add)) return false;


  // post-elaborate process
  if(cases.size() == 0) {       // empty case
    to_del.insert(get_sp());
    return true;
  }

  if(exp->is_valuable()) {      // const case condition
    Number exp_val = exp->get_value();
    // find the case item
    BOOST_FOREACH(shared_ptr<CaseItem>& m, cases) {
      if (m->is_match(exp_val)) {
        to_add[get_sp()].push_back(m->body);
        break;
      }
    }
    to_del.insert(get_sp());
  }

  return true;

} 

void netlist::CaseState::scan_vars(shared_ptr<SDFG::RForest> rf, bool ctl) const {
  shared_ptr<SDFG::RForest> exprf(new SDFG::RForest());
  exp->scan_vars(exprf, true);
  list<shared_ptr<SDFG::RForest> > branches;
  BOOST_FOREACH(const shared_ptr<CaseItem>& m, cases) {
    shared_ptr<SDFG::RForest> mrf(new SDFG::RForest());
    m->scan_vars(mrf, ctl);
    branches.push_back(mrf);
  }
  rf->add(exprf, branches);
}

void netlist::CaseState::replace_variable(const VIdentifier& var, const Number& num) {
  exp->replace_variable(var, num);
  BOOST_FOREACH(shared_ptr<CaseItem> ci, cases) {
    ci->replace_variable(var, num);
  }
}

