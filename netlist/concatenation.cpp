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

netlist::ConElem::ConElem() : NetComp(tConElem), father(NULL), width(0) {}

netlist::ConElem::ConElem(const shell::location& lloc) : NetComp(tConElem, lloc), father(NULL), width(0){}

netlist::ConElem::ConElem(const shared_ptr<Expression>& expr, const list<shared_ptr<ConElem> >& elems)
  : NetComp(tConElem), exp(expr), con(elems), father(NULL), width(0) {}

netlist::ConElem::ConElem(const shell::location& lloc, const shared_ptr<Expression>& expr, 
                          const list<shared_ptr<ConElem> >& elems)
  : NetComp(tConElem, lloc), exp(expr), con(elems), father(NULL), width(0) {}

netlist::ConElem::ConElem(const shared_ptr<Expression>& expr)
  : NetComp(tConElem), exp(expr), father(NULL), width(0) {}

netlist::ConElem::ConElem(const shell::location& lloc, const shared_ptr<Expression>& expr)
  : NetComp(tConElem, lloc), exp(expr), father(NULL), width(0) {}

void netlist::ConElem::reduce() {
  exp->reduce();
  BOOST_FOREACH(shared_ptr<ConElem>& m, con) {
    m->reduce(); 
  }
}

void netlist::ConElem::replace_variable(const VIdentifier& var, const Number& num) {
  exp->replace_variable(var, num);
  BOOST_FOREACH(shared_ptr<ConElem> ce, con) {
    ce->replace_variable(var, num);
  }
}

void netlist::ConElem::replace_variable(const VIdentifier& var, const VIdentifier& nvar) {
  exp->replace_variable(var, nvar);
  BOOST_FOREACH(shared_ptr<ConElem> ce, con) {
    ce->replace_variable(var, nvar);
  }
}

void netlist::ConElem::replace_variable(const VIdentifier& var, shared_ptr<Expression> rexp) {
  exp->replace_variable(var, rexp);
  BOOST_FOREACH(shared_ptr<ConElem> ce, con) {
    ce->replace_variable(var, rexp);
  }  
}

SDFG::RTree netlist::ConElem::get_rtree() const {
  SDFG::RTree(rv(""));
  if(exp)
    rv->combine(exp->get_rtree());
  
  BOOST_FOREACH(shared_ptr<ConElem> m, con) {
    rv->combine(m->get_rtree());
  }
  return rv;
}

unsigned int netlist::ConElem::get_width() const {
  if(con.size() == 0) {
    return exp->get_width();
  } else {
    assert(exp->is_valuable());
    unsigned int rv = 0;
    BOOST_FOREACH(shared_ptr<ConElem> m, con) {
      rv += m->get_width();
    }
    return exp->get_value().get_value().get_ui() * rv;
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

ConElem* netlist::ConElem::deep_copy(NetComp* bp) const {
  ConElem* rv;
  if(!bp) rv = new ConElem();
  else    rv = static_cast<ConElem *>(bp); // C++ does not support multiple dispatch
  NetComp::deep_copy(rv);
  rv->exp = shared_ptr<Expression>(exp->deep_copy(NULL));
  BOOST_FOREACH(shared_ptr<ConElem> ce, con) {
    rv->con.push_back(shared_ptr<ConElem>(ce->deep_copy(NULL)));
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
          shared_ptr<ConElem> newConElem((*it)->deep_copy(NULL));
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

void netlist::Concatenation::replace_variable(const VIdentifier& var, const Number& num) {
  BOOST_FOREACH(shared_ptr<ConElem> d, data) {
    d->replace_variable(var, num);
  }
}

void netlist::Concatenation::replace_variable(const VIdentifier& var, const VIdentifier& nvar) {
  BOOST_FOREACH(shared_ptr<ConElem> d, data) {
    d->replace_variable(var, nvar);
  }
}

void netlist::Concatenation::replace_variable(const VIdentifier& var, shared_ptr<Expression> rexp) {
  BOOST_FOREACH(shared_ptr<ConElem> d, data) {
    d->replace_variable(var, rexp);
  }
}

SDFG::RTree netlist::Concatenation::get_rtree() const {
  SDFG::RTree rv("");
  BOOST_FOREACH(shared_ptr<ConElem> d, data) {
    rv->combine(d->get_rtree());
  }
  return rv;
}

unsigned int netlist::Concatenation::get_width() const {
  unsigned int rv = 0;
  BOOST_FOREACH(shared_ptr<ConElem> d, data) {
    rv += d->get_width();
  }
  return rv;
}

void netlist::Concatenation::db_register(int iod) {
  BOOST_FOREACH(shared_ptr<ConElem> d, data) d->db_register(iod);
}

void netlist::Concatenation::db_expunge() {
  BOOST_FOREACH(shared_ptr<ConElem> d, data) d->db_expunge();
}

Concatenation* netlist::Concatenation::deep_copy(NetComp* bp) const {
  Concatenation *rv;
  if(!bp) rv = new Concatenation();
  else    rv = static_cast<Concatenation*>(bp); // C++ does not support multiple dispatch
  NetComp::deep_copy(rv);
  BOOST_FOREACH(const shared_ptr<ConElem>& m, data)
    rv->data.push_back(shared_ptr<ConElem>(m->deep_copy(NULL)));
  return rv;
}

void netlist::Concatenation::set_father(Block *pf) {
  if(father == pf) return;
  father = pf;
  BOOST_FOREACH(shared_ptr<ConElem> d, data) d->set_father(pf);
}
