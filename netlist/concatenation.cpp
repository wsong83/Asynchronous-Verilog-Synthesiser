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

#include "component.h"
#include <algorithm>
using std::for_each;

using namespace netlist;

ConElem& netlist::ConElem::operator= (const ConElem& rhs) {
  con = rhs.con;
  exp = rhs.exp;
  return *this;
}

ConElem& netlist::ConElem::operator= (const pair<shared_ptr<Expression>, list<ConElem> >& rhs) {
  con = rhs;
  exp.reset();
  return *this;
}

ConElem& netlist::ConElem::operator= (const shared_ptr<Expression>& rhs) {
  con.first.reset();
  con.second.clear();
  exp = rhs;
  return *this;
}

void netlist::ConElem::reduce() {
  if(0 != con.first.use_count()) {
    con.first->reduce();
    list<ConElem>::iterator it, end;
    for(it=con.second.begin(), end=con.second.end(); it != end; it++)
      it->reduce();
  } else {
      exp->reduce();
  }
}

ostream& netlist::ConElem::streamout(ostream& os) const {
  if(0 != exp.use_count()) {
    os << *exp;
  } else if(0 != con.first.use_count()) {
    os << "{" << *(con.first) << "{";
    if(con.second.size() > 0) {
      list<ConElem>::const_iterator it, end;
      it=con.second.begin();
      end=con.second.end(); 
      while(true) {
	os << *it;
	it++;
	if(it != end)
	  os << ",";
	else
	  break;
      }
    } else {
      // should not come here
      assert(0 == "an empty sub concatenation");
    }
    os << "}}";
  }
  return os;
}

ostream& netlist::Concatenation::streamout(ostream& os) const {
  if(data.size() > 1) {
    list<ConElem>::const_iterator it, end;
    os << "{";
    it=data.begin();
    end=data.end(); 
    while(true) {
      os << *it;
      it++;
      if(it != end)
	os << ",";
      else
	break;
    }
    os << "}";
  } else if (data.size() == 1) { // only one element
    os << data.front();
  }
  return os;
}
    
Concatenation& netlist::Concatenation::operator+ (const Concatenation& rhs) {
  list<ConElem>::const_iterator it, end;
  for(it=rhs.data.begin(), end=rhs.data.end(); it != end; it++)
    *this + *it;
  return *this;
}

Concatenation& netlist::Concatenation::operator+ (const ConElem& rhs) {
  // check whether it is an embedded concatenation
  if( 0 != rhs.exp.use_count() &&
      rhs.exp->size() == 1     &&
      rhs.exp->eqn.front().get_type() == Operation::oCon
      ) {
    shared_ptr<Concatenation> conp = rhs.exp->eqn.front().get_con();
    list<ConElem>::const_iterator it, end;
    for(it=conp->data.begin(), end=conp->data.end(); it != end; it++)
      *this + *it;
  } else {
    data.push_back(rhs);
  }
  return *this;
}

void netlist::Concatenation::reduce() {
  list<ConElem>::iterator it, pre, end, begin;
  begin = data.begin();
  it = data.begin();
  pre = data.begin();
  end = data.end();

  // first iteration, remove embedded concatenations
  while(it != end) {
    it->reduce();
    if(it->exp.use_count() != 0) { // an expression
      if(it->exp->eqn.size() == 1 &&
         it->exp->eqn.front().get_type() == Operation::oCon) { // embedded concatenation
        shared_ptr<Concatenation> cmp = it->exp->eqn.front().get_con(); // fetch the concatenation
        data.insert(it,cmp->data.begin(), cmp->data.end());	// copy the elements to the current level
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
      if(it->con.first->is_valuable()) { // x is a const number, repeat con for x times
        for(mpz_class i = it->con.first->get_value().get_value(); i!=0; i--)
          data.insert(it, it->con.second.begin(), it->con.second.end());
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
    else {
      if(pre->exp.use_count() != 0 &&
         pre->exp->is_valuable() &&
         it->exp.use_count() != 0 &&
         it->exp->is_valuable()) { // both pre and it are numbers
        pre->exp->concatenate(it->exp);
        it = data.erase(it);
      } else it++;
    }
  }
}


