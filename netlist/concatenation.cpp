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

using namespace netlist;

void netlist::ConElem::reduce() {
  exp->reduce();
  if(0 != con.size()) {
    list<shared_ptr<ConElem> >::iterator it, end;
    for(it=con.begin(), end=con.end(); it != end; it++)
      (*it)->reduce();
  }
}

void netlist::ConElem::db_register(int iod) {
  exp->db_register(iod);

  list<shared_ptr<ConElem> >::iterator it, end;
  for(it = con.begin(), end = con.end(); it != end; it++) 
    (*it)->db_register(iod);
}

void netlist::ConElem::db_expunge() {
  exp->db_expunge();

  list<shared_ptr<ConElem> >::iterator it, end;
  for(it = con.begin(), end = con.end(); it != end; it++) 
    (*it)->db_expunge();
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
  list<shared_ptr<ConElem> >::const_iterator it, end;
  for(it=con.begin(), end=con.end(); it!=end; it++)
    rv->con.push_back(shared_ptr<ConElem>((*it)->deep_copy()));
  return rv;
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
    
Concatenation& netlist::Concatenation::operator+ (shared_ptr<Concatenation>& rhs) {
  list<shared_ptr<ConElem> >::iterator it, end;
  for(it=rhs->data.begin(), end=rhs->data.end(); it != end; it++)
    *this + *it;
  return *this;
}

Concatenation& netlist::Concatenation::operator+ (shared_ptr<ConElem>& rhs) {
  // check whether it is an embedded concatenation
  if( 0 == rhs->con.size() &&
      rhs->exp->size() == 1 &&
      rhs->exp->eqn.front()->get_type() == Operation::oCon
      ) {
    Concatenation& m_con = rhs->exp->eqn.front()->get_con();
    list<shared_ptr<ConElem> >::iterator it, end;
    for(it=m_con.data.begin(), end=m_con.data.end(); it != end; it++)
      *this + *it;
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
      if((*it)->exp->eqn.size() == 1 &&
         (*it)->exp->eqn.front()->get_type() == Operation::oCon) { // embedded concatenation
        Concatenation cm = (*it)->exp->eqn.front()->get_con(); // fetch the concatenation
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
          data.insert(it,(*it)->con.begin(), (*it)->con.end());
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

void netlist::Concatenation::db_register(int iod) {
  list<shared_ptr<ConElem> >::iterator it, end;
  for(it = data.begin(), end = data.end(); it != end; it++) 
    (*it)->db_register(iod);
}

void netlist::Concatenation::db_expunge() {
  list<shared_ptr<ConElem> >::iterator it, end;
  for(it = data.begin(), end = data.end(); it != end; it++) 
    (*it)->db_expunge();
}

Concatenation* netlist::Concatenation::deep_copy() const {
  Concatenation* rv = new Concatenation();
  list<shared_ptr<ConElem> >::const_iterator it, end;
  for(it=data.begin(), end=data.end(); it!=end; it++)
    rv->data.push_back(shared_ptr<ConElem>((*it)->deep_copy()));
  return rv;
}
