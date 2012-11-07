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
 * Concatenation
 * 22/02/2012   Wei Song
 *
 *
 */

#include <algorithm>
#include "component.h"
#include "shell/env.h"
#include "sdfg/rtree.hpp"
#include <boost/foreach.hpp>

using namespace netlist;
using std::ostream;
using std::string;
using std::vector;
using boost::shared_ptr;
using std::list;
using std::for_each;

netlist::ConElem::ConElem() : father(NULL), width(0) {}

netlist::ConElem::ConElem(const shell::location& lloc) : loc(lloc), father(NULL), width(0){}

netlist::ConElem::ConElem(const shared_ptr<Expression>& expr, const list<shared_ptr<ConElem> >& elems)
  : exp(expr), con(elems), father(NULL), width(0) {}

netlist::ConElem::ConElem(const shell::location& lloc, const shared_ptr<Expression>& expr, 
                          const list<shared_ptr<ConElem> >& elems)
  : exp(expr), con(elems), loc(lloc), father(NULL), width(0) {}

netlist::ConElem::ConElem(const shared_ptr<Expression>& expr)
  : exp(expr), father(NULL), width(0) {}

netlist::ConElem::ConElem(const shell::location& lloc, const shared_ptr<Expression>& expr)
  : exp(expr), loc(lloc), father(NULL), width(0) {}

void netlist::ConElem::reduce() {
  exp->reduce();
  BOOST_FOREACH(shared_ptr<ConElem>& m, con) {
    m->reduce(); 
  }
}

void netlist::ConElem::scan_vars(shared_ptr<SDFG::RForest> rf, bool ctl) const {
  if(exp)
    exp->scan_vars(rf, ctl);
  
  BOOST_FOREACH(shared_ptr<ConElem> m, con) {
    m->scan_vars(rf, ctl);
  }
}

void netlist::ConElem::replace_variable(const VIdentifier& var, const Number& num) {
  exp->replace_variable(var, num);
  BOOST_FOREACH(shared_ptr<ConElem> ce, con) {
    ce->replace_variable(var, num);
  }
}

ostream& netlist::ConElem::streamout(ostream& os, unsigned int indent) const {
  os << string(indent, ' ');
  if(0 == con.size()) {
    os << *exp;
  } else {
    os << "{" << exp << "{";
    list<shared_ptr<ConElem> >::const_iterator it, end;
    it=con.begin();
    end=con.end(); 
    while(true) {
      os << **it;
      it++;
      if(it != end)
        os << ",";
      else
        break;
    }
    os << "}}";
  }
  return os;
}

ConElem* netlist::ConElem::deep_copy() const {
  ConElem* rv = new ConElem();
  rv->loc = loc;
  rv->exp = shared_ptr<Expression>(exp->deep_copy());
  BOOST_FOREACH(shared_ptr<ConElem> ce, con) {
    rv->con.push_back(shared_ptr<ConElem>(ce->deep_copy()));
  }
  return rv;
}

void netlist::ConElem::set_father(Block *pf) {
  if(father == pf) return;
  father = pf;
  exp->set_father(pf);
  BOOST_FOREACH(shared_ptr<ConElem> ce, con) {
    ce->set_father(pf);
  }
}

void netlist::ConElem::db_register(int iod) {
  exp->db_register(iod);
  BOOST_FOREACH(shared_ptr<ConElem> ce, con) {
    ce->db_register(iod);
  }
}

void netlist::ConElem::db_expunge() {
  exp->db_expunge();
  BOOST_FOREACH(shared_ptr<ConElem> ce, con) {
    ce->db_expunge();
  }
}

ostream& netlist::Concatenation::streamout(ostream& os, unsigned int indent) const {
  os << string(indent, ' ');
  if(data.size() > 1) {
    list<shared_ptr<ConElem> >::const_iterator it, end;
    os << "{";
    it=data.begin();
    end=data.end(); 
    while(true) {
      os << **it;
      it++;
      if(it != end)
	os << ",";
      else
	break;
    }
    os << "}";
  } else if (data.size() == 1) { // only one element
    os << *(data.front());
  }
  return os;
}
    
Concatenation& netlist::Concatenation::operator+ (const shared_ptr<Concatenation>& rhs) {
  list<shared_ptr<ConElem> >::iterator it, end;
  BOOST_FOREACH(shared_ptr<ConElem> ce, rhs->data) {
    operator+(ce);
  }
  return *this;
}

Concatenation& netlist::Concatenation::operator+ (const shared_ptr<ConElem>& rhs) {
  // check whether it is an embedded concatenation
  if( 0 == rhs->con.size() &&
      rhs->exp->is_singular() &&
      rhs->exp->get_op().get_type() == Operation::oCon
      ) {
    const Concatenation& m_con = rhs->exp->get_op().get_con();
    BOOST_FOREACH(const shared_ptr<ConElem>& it, m_con.data)
      *this + it;
  } else {
    data.push_back(rhs);
  }
  return *this;
}

void netlist::Concatenation::reduce() {
  list<shared_ptr<ConElem> >::iterator it, pre, end, begin;
  begin = data.begin();
  it = data.begin();
  pre = data.begin();
  end = data.end();

  // first iteration, remove embedded concatenations
  while(it != end) {
    (*it)->reduce();
    if((*it)->con.size() == 0) { // an expression
      if((*it)->exp->is_singular() &&
         (*it)->exp->get_op().get_type() == Operation::oCon) { // embedded concatenation
        const Concatenation cm = (*it)->exp->get_op().get_con(); // fetch the concatenation
        data.insert(it,cm.data.begin(), cm.data.end());	// copy the elements to the current level
        data.erase(it); // delete current one as its content is copied
        
        // re-run all element inserted
        if(pre == begin) {	// the first element is removed
          begin = data.begin();
          it = data.begin();
          pre = data.begin();
        } else {		// find the start of the inserted elements
          it = pre;
          it++;
        }	  
      } else it++;
    } else {			// a {x{con}}
      if((*it)->exp->is_valuable()) { // x is a const number, repeat con for x times
        for(mpz_class i = (*it)->exp->get_value().get_value(); i!=0; i--) {
          shared_ptr<ConElem> newConElem((*it)->deep_copy());
          data.insert(it, newConElem->con.begin(), newConElem->con.end());
        }
        data.erase(it);
 
        // re-run all element inserted
        if(pre == begin) {	// the first element is removed
          begin = data.begin();
          it = data.begin();
          pre = data.begin();
        } else {		// find the start of the inserted elements
          it = pre;
          it++;
        }
      } else it++;	
    }
  }

  // second iteration, remove continueous numbers
  begin = data.begin();
  it = data.begin();
  pre = data.begin();
  end = data.end();

  while(it != end) {
    if(it == pre) it++;		// bypass the first element
    else if((*pre)->con.size() == 0   &&
            (*pre)->exp->is_valuable() &&
            (*it)->con.size() == 0    &&
            (*it)->exp->is_valuable()) { // both pre and it are numbers
      (*pre)->exp->concatenate(*((*it)->exp));
      it = data.erase(it);
    } else { 
      pre = it; 
      it++; 
    }
  }
}

void netlist::Concatenation::scan_vars(shared_ptr<SDFG::RForest> rf, bool ctl) const {
  BOOST_FOREACH(shared_ptr<ConElem> m, data) {
    m->scan_vars(rf, ctl);
  }
}

void netlist::Concatenation::replace_variable(const VIdentifier& var, const Number& num) {
  BOOST_FOREACH(shared_ptr<ConElem> d, data) {
    d->replace_variable(var, num);
  }
}

void netlist::Concatenation::db_register(int iod) {
  BOOST_FOREACH(shared_ptr<ConElem> d, data) d->db_register(iod);
}

void netlist::Concatenation::db_expunge() {
  BOOST_FOREACH(shared_ptr<ConElem> d, data) d->db_expunge();
}

Concatenation* netlist::Concatenation::deep_copy() const {
  Concatenation* rv = new Concatenation();
  rv->loc = loc;
  BOOST_FOREACH(const shared_ptr<ConElem>& m, data)
    rv->data.push_back(shared_ptr<ConElem>(m->deep_copy()));
  return rv;
}

void netlist::Concatenation::set_father(Block *pf) {
  if(father == pf) return;
  father = pf;
  BOOST_FOREACH(shared_ptr<ConElem> d, data) d->set_father(pf);
}
