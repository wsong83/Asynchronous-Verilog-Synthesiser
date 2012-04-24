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
 * Range declaration and definition
 * 08/02/2012   Wei Song
 *
 *
 */

#include "component.h"

using namespace netlist;

netlist::Range::Range(const mpz_class& sel)
  : NetComp(tRange), c(sel), dim(false), rtype(TR_Const) {  }

netlist::Range::Range(const location& lloc, const mpz_class& sel)
  : NetComp(tRange, lloc), c(sel), dim(false), rtype(TR_Const) {  }

netlist::Range::Range(const mpz_class& rl, const mpz_class& rr)
  : NetComp(tRange), cr(rl,rr), dim(false), rtype(TR_CRange) {  }

netlist::Range::Range(const location& lloc, const mpz_class& rl, const mpz_class& rr)
  : NetComp(tRange, lloc), cr(rl,rr), dim(false), rtype(TR_CRange) {  }

netlist::Range::Range(const shared_ptr<Expression>& sel)
  : NetComp(tRange), dim(false), rtype(TR_Var) 
{ 
  shared_ptr<Expression> m(sel->deep_copy());
  m->reduce();
  if(m->is_valuable()) {	// constant expression, value it now
    c = m->get_value().get_value();
    rtype = TR_Const;
  } else {
    v = m;
  }
 };

netlist::Range::Range(const location& lloc, const shared_ptr<Expression>& sel)
  : NetComp(tRange, lloc), dim(false), rtype(TR_Var) 
{ 
  shared_ptr<Expression> m(sel->deep_copy());
  m->reduce();
  if(m->is_valuable()) {	// constant expression, value it now
    c = m->get_value().get_value();
    rtype = TR_Const;
  } else {
    v = m;
  }
 };

netlist::Range::Range(const Range_Exp& sel, bool dimm)
  : NetComp(tRange), dim(dimm), rtype(TR_Range) 
{ 
  Range_Exp m(shared_ptr<Expression>(sel.first->deep_copy()), shared_ptr<Expression>(sel.second->deep_copy()));
  m.first->reduce();
  m.second->reduce();
  if(*(m.first) == *(m.second)) {	// only one bit is selected
    if(m.first->is_valuable()) { // constant expression, value it now
      c = m.first->get_value().get_value();
      rtype = TR_Const;
    } else {			// variable expression
      v = m.first;
      rtype = TR_Var;
    }
  } else {
    if(m.first->is_valuable() && m.second->is_valuable()) {
      rtype = TR_CRange;
      cr.first = m.first->get_value().get_value();
      cr.second = m.second->get_value().get_value();
    } else {
      r = m;
    } 
  }
};

netlist::Range::Range(const location& lloc, const Range_Exp& sel, bool dimm)
  : NetComp(tRange, lloc), dim(dimm), rtype(TR_Range) 
{ 
  Range_Exp m(shared_ptr<Expression>(sel.first->deep_copy()), shared_ptr<Expression>(sel.second->deep_copy()));
  m.first->reduce();
  m.second->reduce();
  if(*(m.first) == *(m.second)) {	// only one bit is selected
    if(m.first->is_valuable()) { // constant expression, value it now
      c = m.first->get_value().get_value();
      rtype = TR_Const;
    } else {			// variable expression
      v = m.first;
      rtype = TR_Var;
    }
  } else {
    if(m.first->is_valuable() && m.second->is_valuable()) {
      rtype = TR_CRange;
      cr.first = m.first->get_value().get_value();
      cr.second = m.second->get_value().get_value();
    } else {
      r = m;
    } 
  }
};

netlist::Range::Range(const Range_Exp& sel, int updown)
  : NetComp(tRange), dim(false), rtype(TR_Range)
{
  shared_ptr<Expression> first(sel.first->deep_copy());
  Range_Exp m_sel;
  if(updown == 1) { // positive colon
    *first+*(sel.second);
    m_sel.first = first;
    m_sel.second.reset(sel.first->deep_copy());
  } else if(updown == -1){ // negtive colon
    m_sel.first.reset(sel.first->deep_copy());
    *first - *(sel.second);
    m_sel.second = first;
  } else {
    // error
    assert(1 == 0);
  }

  if(*(m_sel.first) == *(m_sel.second)) {	// only one bit is selected
    if(m_sel.first->is_valuable()) { // constant expression, value it now
      c = m_sel.first->get_value().get_value();
      rtype = TR_Const;
    } else {			// variable expression
      v = m_sel.first;
      rtype = TR_Var;
    } 
  } else {
    if(m_sel.first->is_valuable() && m_sel.second->is_valuable()) {
      rtype = TR_CRange;
      cr.first = m_sel.first->get_value().get_value();
      cr.second = m_sel.second->get_value().get_value();
    } else {
      r = m_sel;
    } 
  }
}

netlist::Range::Range(const location& lloc, const Range_Exp& sel, int updown)
  : NetComp(tRange, lloc), dim(false), rtype(TR_Range)
{
  shared_ptr<Expression> first(sel.first->deep_copy());
  Range_Exp m_sel;
  if(updown == 1) { // positive colon
    *first+*(sel.second);
    m_sel.first = first;
    m_sel.second.reset(sel.first->deep_copy());
  } else if(updown == -1){ // negtive colon
    m_sel.first.reset(sel.first->deep_copy());
    *first - *(sel.second);
    m_sel.second = first;
  } else {
    // error
    assert(1 == 0);
  }

  if(*(m_sel.first) == *(m_sel.second)) {	// only one bit is selected
    if(m_sel.first->is_valuable()) { // constant expression, value it now
      c = m_sel.first->get_value().get_value();
      rtype = TR_Const;
    } else {			// variable expression
      v = m_sel.first;
      rtype = TR_Var;
    } 
  } else {
    if(m_sel.first->is_valuable() && m_sel.second->is_valuable()) {
      rtype = TR_CRange;
      cr.first = m_sel.first->get_value().get_value();
      cr.second = m_sel.second->get_value().get_value();
    } else {
      r = m_sel;
    } 
  }
}

void netlist::Range::db_register() {
  v->db_register(1);
  r.first->db_register(1);
  r.second->db_register(1);
}
  
void netlist::Range::set_father(Block *pf) {
  if(father == pf) return;
  father = pf;
  switch(rtype) {
  case TR_Var: v->set_father(pf); return;
  case TR_Range: r.first->set_father(pf); r.second->set_father(pf); return;
  default: return;
  }
}

ostream& netlist::Range::streamout(ostream& os, unsigned int indent) const {
  os << string(indent, ' ');
  switch(rtype) {
  case TR_Const: os << c; break;
  case TR_Var: os << *v; break;
  case TR_Range: os << *(r.first) << ":" << *(r.second); break;
  case TR_CRange: os << cr.first << ":" << cr.second; break;
  default: // should not go here
    assert(0 == "Wrong range type");
  }
  return os;
}

bool netlist::Range::check_inparse() {
  bool rv = true;
  
  switch(rtype) {
  case TR_Var: rv &= v->check_inparse(); return rv;
  case TR_Range: 
    rv &= r.first->check_inparse(); 
    rv &= r.second->check_inparse(); 
    return rv;
  default: 
    return rv;
  }
}

Range* netlist::Range::deep_copy() const {
  Range* rv = new Range();
  switch(rtype) {
  case TR_Const: rv->c = c; break;
  case TR_Var: rv->v = shared_ptr<Expression>(v->deep_copy()); break;
  case TR_Range: rv->r = Range_Exp(shared_ptr<Expression>(r.first->deep_copy()), shared_ptr<Expression>(r.second->deep_copy())); break;
  case TR_CRange: rv->cr = cr; break;
  default: // should not go here
    assert(0 == "Wrong range type");
  }
  rv->dim = dim;
  rv->rtype = rtype;
  rv->set_father(father);
  return rv;
}
