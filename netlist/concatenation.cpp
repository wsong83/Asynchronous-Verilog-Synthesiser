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
#include <boost/foreach.hpp>

using namespace netlist;
using std::ostream;
using std::string;
using std::vector;
using boost::shared_ptr;
using std::list;
using std::for_each;

void netlist::ConElem::reduce() {
  exp->reduce();
  for_each(con.begin(), con.end(), [](shared_ptr<ConElem>& m) {
      m->reduce(); 
    });
}

void netlist::ConElem::scan_vars(std::set<string>& t_vars, std::set<string>& d_vars, std::set<string>& c_vars, bool ctl) const {
  if(exp)
    exp->scan_vars(t_vars, d_vars, c_vars, ctl);
  
  BOOST_FOREACH(shared_ptr<ConElem> m, con) {
    m->scan_vars(t_vars, d_vars, c_vars, ctl);
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
  list<shared_ptr<ConElem> >::const_iterator it, end;
  for(it=con.begin(), end=con.end(); it!=end; it++)
    rv->con.push_back(shared_ptr<ConElem>((*it)->deep_copy()));
  return rv;
}

void netlist::ConElem::set_father(Block *pf) {
  if(father == pf) return;
  father = pf;
  exp->set_father(pf);
  list<shared_ptr<ConElem> >::iterator it, end;
  for(it=con.begin(), end=con.end(); it!=end; it++)
    (*it)->set_father(pf);
}

bool netlist::ConElem::check_inparse() {
  bool rv = true;
  rv &= exp->check_inparse();
  list<shared_ptr<ConElem> >::iterator it, end;
  for(it=con.begin(), end=con.end(); it!=end; it++)
    rv &= (*it)->check_inparse();
  return rv;
}

void netlist::ConElem::db_register(int iod) {
  exp->db_register(iod);
  for_each(con.begin(), con.end(), [&](shared_ptr<ConElem>& m) {m->db_register(iod);});
}

void netlist::ConElem::db_expunge() {
  exp->db_expunge();
  for_each(con.begin(), con.end(), [](shared_ptr<ConElem>& m) {m->db_expunge();});
}

bool netlist::ConElem::elaborate(NetComp::elab_result_t &result) {
  bool rv = true;
  result = NetComp::ELAB_Normal;

  rv &= exp->elaborate(result);
  if(!rv) return false;
  
  for_each(con.begin(), con.end(), [&](shared_ptr<ConElem>& m) {
      rv &= m->elaborate(result);
    });
  if(!rv) return false;

  reduce();
  
  // check
  if(con.size() != 0 && !exp->is_valuable()) {
    G_ENV->error(exp->loc, "ELAB-EXPRESSION-0", toString(*exp));
    return false;
  }

  return rv;
}

unsigned int netlist::ConElem::get_width() {
  if(width) return width;
  assert(con.size() == 0);
  width = exp->get_width();
  return width;
}

void netlist::ConElem::set_width(const unsigned int& w) {
  if(width == w) return;
  assert(exp->get_width() > w);
  exp->set_width(w);
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
  for(it=rhs->data.begin(), end=rhs->data.end(); it != end; it++)
    *this + *it;
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

void netlist::Concatenation::scan_vars(std::set<string>& t_vars, std::set<string>& d_vars, std::set<string>& c_vars, bool ctl) const {
  BOOST_FOREACH(shared_ptr<ConElem> m, data) {
    m->scan_vars(t_vars, d_vars, c_vars, ctl);
  }
}

bool netlist::Concatenation::elaborate(elab_result_t &result, const ctype_t, const vector<NetComp *>&) {
  bool rv = true;
  result = ELAB_Normal;
  for_each(data.begin(), data.end(), [&](shared_ptr<ConElem>& m){
      rv &= m->elaborate(result);
    });
  if(!rv) return false;

  reduce();
  
  return rv;
}

void netlist::Concatenation::db_register(int iod) {
  for_each(data.begin(), data.end(), [&](shared_ptr<ConElem>& m) {m->db_register(iod);});
}

void netlist::Concatenation::db_expunge() {
  for_each(data.begin(), data.end(), [](shared_ptr<ConElem>& m) {m->db_expunge();});
}

Concatenation* netlist::Concatenation::deep_copy() const {
  Concatenation* rv = new Concatenation();
  rv->loc = loc;
  rv->width = width;
  BOOST_FOREACH(const shared_ptr<ConElem>& m, data)
    rv->data.push_back(shared_ptr<ConElem>(m->deep_copy()));
  return rv;
}

void netlist::Concatenation::set_father(Block *pf) {
  if(father == pf) return;
  father = pf;
  list<shared_ptr<ConElem> >::iterator it, end;
  for(it=data.begin(), end=data.end(); it!=end; it++)
    (*it)->set_father(pf);
}

bool netlist::Concatenation::check_inparse() {
  bool rv = true;
  list<shared_ptr<ConElem> >::iterator it, end;
  for(it=data.begin(), end=data.end(); it!=end; it++)
    rv &= (*it)->check_inparse();

  return rv;
}

unsigned int netlist::Concatenation::get_width() {
  if(width) return width;
  BOOST_FOREACH(shared_ptr<ConElem>& m, data)
    width += m->get_width();
  return width;
}

void netlist::Concatenation::set_width(const unsigned int& w) {
  if(width == w) return;
  assert(w < get_width());
  unsigned int wm = w;
  list<shared_ptr<ConElem> >::reverse_iterator it, end;
  for(it=data.rbegin(), end=data.rend(); it!=end; it++) {
    if(wm >= (*it)->get_width()) 
      wm -= (*it)->get_width();
    else {
      if(wm == 0) break;
      else {
        (*it)->set_width(wm);
        wm = 0;
      }
    }
  }
  if(it != end) 
    data.erase(data.begin(), it.base()); // ATTN: it is a reverse_iterator
  width = w;
}
