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

CaseItem* netlist::CaseItem::deep_copy(NetComp* bp) const {
  CaseItem *rv;
  if(!bp) rv = new CaseItem();
  else    rv = static_cast<CaseItem *>(bp); // C++ does not support multiple dispatch
  NetComp::deep_copy(rv);

  if(body) rv->body.reset(body->deep_copy(NULL));
  BOOST_FOREACH(const  shared_ptr<Expression>& m, exps)
    rv->exps.push_back(shared_ptr<Expression>(m->deep_copy(NULL)));
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

SDFG::RForest netlist::CaseItem::get_rforest() const {
  return body->get_rforest();
}

void netlist::CaseItem::replace_variable(const VIdentifier& var, const Number& num) {
  BOOST_FOREACH(shared_ptr<Expression> e, exps) {
    e->replace_variable(var, num);
  }
  if(body) body->replace_variable(var, num);
}

void netlist::CaseItem::replace_variable(const VIdentifier& var, const VIdentifier& nvar) {
  BOOST_FOREACH(shared_ptr<Expression> e, exps) {
    e->replace_variable(var, nvar);
  }
  if(body) body->replace_variable(var, nvar);
}

shared_ptr<Block> netlist::CaseItem::unfold() {
  body = body->unfold();
  return shared_ptr<Block>();
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

netlist::CaseState::CaseState()
  : NetComp(tCase), named(false), case_type(CASE_DEFAULT) 
{}

netlist::CaseState::CaseState(const shell::location& lloc)
  : NetComp(tCase, lloc), named(false), case_type(CASE_DEFAULT) 
{}

netlist::CaseState::CaseState(const shared_ptr<Expression>& exp, const list<shared_ptr<CaseItem> >& citems, 
                              const shared_ptr<CaseItem>& ditem, case_type_t mcase)
  : NetComp(tCase), exp(exp), cases(citems), named(false), case_type(mcase) 
{
  cases.push_back(ditem);
}

netlist::CaseState::CaseState(const shell::location& lloc, const shared_ptr<Expression>& exp, 
                              const list<shared_ptr<CaseItem> >& citems, const shared_ptr<CaseItem>& ditem, 
                              case_type_t mcase)
  : NetComp(tCase, lloc), exp(exp), cases(citems), named(false), case_type(mcase) 
{
  cases.push_back(ditem);
}

netlist::CaseState::CaseState(const shared_ptr<Expression>& exp, const list<shared_ptr<CaseItem> >& citems, 
                              case_type_t mcase)
  : NetComp(tCase), exp(exp), cases(citems), named(false), case_type(mcase) {} 

netlist::CaseState::CaseState(const shell::location& lloc, const shared_ptr<Expression>& exp, 
                              const list<shared_ptr<CaseItem> >& citems, case_type_t mcase)
  : NetComp(tCase, lloc), exp(exp), cases(citems), named(false), case_type(mcase) {} 

netlist::CaseState::CaseState(const shared_ptr<Expression>& exp, const shared_ptr<CaseItem>& ditem, 
                              case_type_t mcase)
  : NetComp(tCase), exp(exp), named(false), case_type(mcase) 
{
  cases.push_back(ditem);
}

netlist::CaseState::CaseState(const shell::location& lloc, const shared_ptr<Expression>& exp, 
                              const shared_ptr<CaseItem>& ditem, case_type_t mcase)
  : NetComp(tCase, lloc), exp(exp), named(false), case_type(mcase) 
{
  cases.push_back(ditem);
}

ostream& netlist::CaseState::streamout (ostream& os, unsigned int indent) const {
  os << string(indent, ' ');
  switch(case_type) {
  case CASE_X: os << "casex(" << *exp << ")" << endl; break;
  case CASE_Z: os << "casez(" << *exp << ")" << endl; break;
  default:     os << "case("  << *exp << ")" << endl; break;
  }
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

CaseState* netlist::CaseState::deep_copy(NetComp* bp) const {
  CaseState *rv;
  if(!bp) rv = new CaseState();
  else    rv = static_cast<CaseState *>(bp); // C++ does not support multiple dispatch
  NetComp::deep_copy(rv);
  
  rv->name = name;
  rv->named = named;
  rv->case_type = case_type;
  if(exp) rv->exp.reset(exp->deep_copy(NULL));
  BOOST_FOREACH(const shared_ptr<CaseItem>& m, cases)
    rv->cases.push_back(shared_ptr<CaseItem>(m->deep_copy(NULL)));
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

SDFG::RForest netlist::CaseState::get_rforest() const {
  SDFG::RTree exp_rtree = exp->get_rtree();
  SDFG::RForest rv;
  int num_of_case = 0;
  map<string, int> target_count;
  bool has_default = false;

  list<SDFG::sig_map> case_smap;
  // get all cases and count the target counts
  BOOST_FOREACH(shared_ptr<CaseItem> m, cases) {
    // add the case
    SDFG::RForest case_rtree = m->get_rforest();
    rv.combine(case_rtree);
    case_smap.push_back(case_rtree.get_target_signals());    

    num_of_case++;
    if(m->is_default()) {
      has_default = true;
      break;
    }
  }
  
  // no default, or case not full, add self loops
  if(!has_default && num_of_case < (1 << exp->get_width())) {
    SDFG::sig_map smap_all = rv.get_target_signals();
    for(SDFG::sig_map::iterator it=smap_all.begin(); it!=smap_all.end(); ++it) {
      SDFG::dfgRangeMap r = it->second;
      SDFG::RTree mt(it->first, r);
      mt.add(SDFG::RRelation(it->first, r, SDFG::dfgEdge::SDFG_DDP));
      rv.add(mt);
    }
  } else {
    // chase for latch
    SDFG::sig_map smap_all = rv.get_target_signals();
    SDFG::RForest sloops;
    BOOST_FOREACH(SDFG::sig_map smap_item, case_smap) {
      for(SDFG::sig_map::iterator it=smap_all.begin(); it!=smap_all.end(); ++it) {
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
  }
  
  // avoid translating ROM into case
  if(rv.get_data_signals().size())
    exp_rtree.assign_type(SDFG::dfgEdge::SDFG_EQU);
  else
    exp_rtree.assign_type(SDFG::dfgEdge::SDFG_DAT);

  for(SDFG::RForest::iterator it = rv.begin(); it != rv.end(); ++it) {
    it->second.combine(exp_rtree);
  }

  //std::cout << *this << *rv << std::endl;
  return rv;
}

void netlist::CaseState::replace_variable(const VIdentifier& var, const Number& num) {
  exp->replace_variable(var, num);
  BOOST_FOREACH(shared_ptr<CaseItem> ci, cases) {
    ci->replace_variable(var, num);
  }
}

void netlist::CaseState::replace_variable(const VIdentifier& var, const VIdentifier& nvar) {
  exp->replace_variable(var, nvar);
  BOOST_FOREACH(shared_ptr<CaseItem> ci, cases) {
    ci->replace_variable(var, nvar);
  }
}

shared_ptr<Block> netlist::CaseState::unfold() {
  // unfold the child cases
  BOOST_FOREACH(shared_ptr<CaseItem> c, cases) {
    c->unfold();
  }

  // reduce
  exp->reduce();

  if(exp->is_valuable()) {
    Number exp_val = exp->get_value();
    // find the case item
    BOOST_FOREACH(shared_ptr<CaseItem>& m, cases) {
      if (m->is_match(exp_val))
        return m->body;
    }
  }

  return shared_ptr<Block>();
}

shared_ptr<Expression> netlist::CaseState::get_combined_expression(const VIdentifier& target, std::set<string> s_set) {
  //std::cout << *this << std::endl;
  bool has_default = false;
  bool target_found = false;
  typedef std::pair<list<shared_ptr<Expression> >, shared_ptr<Expression> > cexp_type;
  list<cexp_type> m_case_exps;
  BOOST_FOREACH(boost::shared_ptr<CaseItem> ct, cases) {
    m_case_exps.push_back(cexp_type(ct->exps, ct->body->get_combined_expression(target, s_set)));
    
    if((!target_found) && m_case_exps.back().second) 
      target_found = true;
    
    if(ct->is_default()) {
      has_default = true;
      break;
    }
  }

  if(!target_found) return shared_ptr<Expression>(); // no target found at all
  
  if(!has_default) 
    m_case_exps.push_back(cexp_type(list<shared_ptr<Expression> >(), 
                                    shared_ptr<Expression>(new Expression(target))
                                    ));
  
  shared_ptr<Expression> rv = m_case_exps.back().second;
  if(!rv) rv = shared_ptr<Expression>(new Expression(target));
  m_case_exps.pop_back();
  
  while(!m_case_exps.empty()) {
    shared_ptr<Expression> cond;
    BOOST_FOREACH(shared_ptr<Expression> e, m_case_exps.back().first) {
      shared_ptr<Expression> case_exp(e->deep_copy(NULL));
      shared_ptr<Expression> case_var(exp->deep_copy(NULL));
      case_exp->append(Operation::oCEq, *case_var);
      if(cond) {
        cond->append(Operation::oLOr, *case_exp);
      } else {
        cond = case_exp;
      }
    }
    if(m_case_exps.back().second) {
      cond->append(Operation::oQuestion, *(m_case_exps.back().second), *rv);
      //rv.reset(cond->append(Operation::oQuestion, *(m_case_exps.back().second), *rv));
      rv = cond;
    } else {
      Expression self_loop(target);
      cond->append(Operation::oQuestion, self_loop, *rv);
      rv = cond;
    }
    m_case_exps.pop_back();
  }
  
  return rv;
}
