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
using std::ostream;
using std::string;
using std::vector;
using boost::shared_ptr;
using shell::location;

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

Range& netlist::Range::op_and ( const Range& rhs) {
  assert(is_valuable() && rhs.is_valuable());
  if(is_empty()) return *this;                             // left empty
  if(rhs.is_empty()) { rtype = TR_Empty; return *this; }   // right empty
  if(rtype == TR_Const && rhs.rtype == TR_Const) {
    if(c == rhs.c) return *this;                           // equal 
    else {rtype = TR_Empty; return *this; }                // single and unequal
  }
  if(rtype == TR_Const) {
    if(c <= rhs.cr.first && c >= rhs.cr.second) return *this; // left single and belong to right
    else { rtype = TR_Empty; return *this; }                  // left single but not belong to right
  } else if(rhs.rtype == TR_Const) {
    if(rhs.c <= cr.first && rhs.c >= cr.second) {             // right single and belong to left
      c = rhs.c;
      rtype = TR_Const;
      return *this;
    }
    else { rtype = TR_Empty; return *this; }                  // right single but not belong to left
  } else {
    cr.first  = cr.first  > rhs.cr.first  ? rhs.cr.first  : cr.first ;
    cr.second = cr.second < rhs.cr.second ? rhs.cr.second : cr.second;
    if(cr.first < cr.second) { rtype = TR_Empty; return *this; } // no shared area
    if(cr.first == cr.second) { c = cr.first; rtype = TR_Const; return *this;} // only one bit
    return *this;
  }
}

Range& netlist::Range::op_or ( const Range& rhs) {
  assert(is_valuable() && rhs.is_valuable());
  if(rhs.is_empty()) return *this;                         // right empty
  if(is_empty()) { rtype = rhs.rtype; c = rhs.c; cr=rhs.cr; return *this; }   // left empty
  if(rtype == TR_Const && rhs.rtype == TR_Const) {
    if(c == rhs.c) return *this;                           // equal 
    else if(c == rhs.c + 1 || c + 1 == rhs.c) {            // adjacent
      cr.first = c > rhs.c ? c : rhs.c;
      cr.second = c > rhs.c ? rhs.c : c;
      rtype = TR_CRange;
      return *this;
    } else {rtype = TR_Err; return *this; }                  // single and unadjacent
  }
  if(rtype == TR_Const) {
    if(c <= rhs.cr.first+1 && c+1 >= rhs.cr.second) {      // left single and belong to right
      cr.first = c > rhs.cr.first ? c : rhs.cr.first;
      cr.second = rhs.cr.second > c ? c : rhs.cr.second;
      rtype = TR_CRange;
      return *this;
    } else { rtype = TR_Err; return *this; }               // not adjacent
  } else if(rhs.rtype == TR_Const) {
    if(rhs.c <= cr.first+1 && rhs.c+1 >= cr.second) {      // right single and belong to left
      cr.first = rhs.c > cr.first ? rhs.c : cr.first;
      cr.second = cr.second > rhs.c ? rhs.c : cr.second;
      return *this;
    } else { rtype = TR_Err; return *this; }             // not adjacent
  } else {
    if(cr.second <= rhs.cr.first + 1) {
      cr.first  = cr.first  > rhs.cr.first  ? cr.first  : rhs.cr.first ;
      cr.second = cr.second < rhs.cr.second ? cr.second : rhs.cr.second;
      return *this;
    } else {rtype = TR_Err; return *this; }             // not adjacent
  }
}

bool netlist::Range::op_equ(const Range& rhs) const {
  switch(rtype) {
  case TR_Empty: return rhs.rtype == TR_Empty;
  case TR_Const: return rhs.rtype == TR_Const && c == rhs.c;
  case TR_CRange: return rhs.rtype == TR_CRange && cr.first == rhs.cr.first && cr.second == rhs.cr.second;
  case TR_Var: return rhs.rtype == TR_Var;
  default: return false;
  }
}

bool netlist::Range::op_belong_to(const Range& rhs) const {
  assert(is_valuable() && rhs.is_valuable());
  if(rtype == TR_Empty) return true;
  if(rhs.rtype == TR_Empty) return false;
  if(rtype == TR_Const && rhs.rtype == TR_Const)
    if(c == rhs.c) return true;
    else return false;
  else if(rtype == TR_Const) {
    if(c <= rhs.cr.first && c >= rhs.cr.second) return true;
    else return false;
  } else if(rhs.rtype == TR_Const) {
    if(rhs.c == cr.first && rhs.c == cr.second) return true;
    else return false;
  } else {
    if(cr.first <= rhs.cr.first && cr.second >= rhs.cr.second) return true;
    else return false;
  }
}

bool netlist::Range::op_adjacent_to(const Range& rhs) const {
  assert(is_valuable() && rhs.is_valuable());
  if(rtype == TR_Empty || rhs.rtype == TR_Empty) return false;
  if(rtype == TR_Const && rhs.rtype == TR_Const) {
    if(c <= rhs.c+1 && c+1 >= rhs.c) return true;
    else return false;
  }
  if(rtype == TR_Const) {
    if(c <= rhs.cr.first+1 && c+1 >= rhs.cr.second) return true;
    else return false;
  }
  if(rhs.rtype == TR_Const) {
    if(rhs.c <= cr.first+1 && rhs.c+1 >= cr.second) return true;
    else return false;
  }
  else if(cr.second <= rhs.cr.first+1 || rhs.cr.second <= cr.first + 1) return true;
  else return false;
} 

void netlist::Range::db_register(int iod) {
  switch(rtype) {
  case TR_Var: v->db_register(1); break;
  case TR_Range: r.first->db_register(1); r.second->db_register(1); break;
  default: ;
  }
}

void netlist::Range::db_expunge() {
  switch(rtype) {
  case TR_Var: v->db_expunge(); break;
  case TR_Range: r.first->db_expunge(); r.second->db_expunge(); break;
  default: ;
  }
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
  case TR_Const: os << c.get_value(); break;
  case TR_Var: os << *v; break;
  case TR_Range: {
    if(r.first->is_valuable())  os << r.first->get_value().get_value();
    else                        os << *(r.first); 
    os << ":"; 
    if(r.second->is_valuable()) os << r.second->get_value().get_value();
    else                        os << *(r.second); 
    break;
  }
  case TR_CRange: os << cr.first.get_value() << ":" << cr.second.get_value(); break;
  case TR_Empty: os << "empty"; break;
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
  case TR_Empty: break;
  default: // should not go here
    assert(0 == "Wrong range type");
  }
  rv->dim = dim;
  rv->rtype = rtype;
  return rv;
}

bool netlist::Range::elaborate(elab_result_t &result, const ctype_t mctype, const vector<NetComp *>& fp) {
  bool rv = true;
  result = ELAB_Normal;

  switch(rtype) {
  case TR_Var: {
    rv = v->elaborate(result, tExp); 
    if(v->is_valuable()) {
      c = v->get_value();
      rtype = TR_Const;
      v.reset();
    }
    break;
  }
  case TR_Range: {
    rv = r.first->elaborate(result) && r.second->elaborate(result); 
    if(r.first->is_valuable() && r.second->is_valuable()) {
      if(r.first->get_value() == r.second->get_value()) {
        c = v->get_value();
        rtype = TR_Const;
      } else {
        cr.first = r.first->get_value();
        cr.second = r.second->get_value();
        rtype = TR_CRange;
      }
      r.first.reset();
      r.second.reset();
    }
    break;
  }
  default:;
  }

  return rv;
}

Range netlist::operator& ( const Range& lhs, const Range& rhs) {
  shared_ptr<Range> rv;
  if(!rhs.is_valuable()) {
    rv.reset(lhs.deep_copy());
  } else if(!lhs.is_valuable()) {
    rv.reset(rhs.deep_copy());
  } else {
    rv.reset(lhs.deep_copy());
    rv->op_and(rhs);
  }
  return *rv;
}

Range netlist::operator| ( const Range& lhs, const Range& rhs) {
  shared_ptr<Range> rv;
  if(!lhs.is_valuable()) {
    rv.reset(lhs.deep_copy());
  } else if(!rhs.is_valuable()) {
    rv.reset(rhs.deep_copy());
  } else {
    rv.reset(lhs.deep_copy());
    rv->op_or(rhs);
  }
  return *rv;
}
 
bool netlist::operator>= (const Range& lhs, const Range& rhs) {
  if(!lhs.is_valuable()) return true;
  if(!rhs.is_valuable()) return false;
  return rhs.op_belong_to(lhs);
}
