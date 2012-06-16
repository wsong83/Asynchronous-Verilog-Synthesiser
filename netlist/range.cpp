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

void netlist::Range::const_copy( const Range& rhs) {
  c = rhs.c;
  cr = rhs.cr;
  rtype = rhs.rtype;
  dim = rhs.dim;
}

Range netlist::Range::op_and ( const Range& rhs) const{
  Range rv;

  if(rtype == TR_Err || rhs.rtype == TR_Err) return rv; // error

  if(rtype == TR_Empty || rhs.rtype == TR_Empty)        // either empty
    { rv.rtype = TR_Empty; return rv; }

  if(!is_valuable() && !rhs.is_valuable())              // both variable, return full
    { rv.rtype = TR_Var; return rv; }

  if(!is_valuable())          { rv.const_copy(rhs);   return rv; }  // variable, return rhs
  else if(!rhs.is_valuable()) { rv.const_copy(*this); return rv; }  // rhs variable, return this

  // both const
  if(rtype == TR_Const && rhs.rtype == TR_Const) {
    if(c == rhs.c) { rv.const_copy(rhs);  return rv; }            // equal 
    else           { rv.rtype = TR_Empty; return rv; }            // single and unequal
  }
  
  if(rtype == TR_Const) {
    if(c <= rhs.cr.first && c >= rhs.cr.second) 
           { rv.const_copy(*this); return rv; }              // left single and belong to right
    else   { rv.rtype = TR_Empty;  return rv; }              // left single but not belong to right
  } else if(rhs.rtype == TR_Const) {
    if(rhs.c <= cr.first && rhs.c >= cr.second) 
           { rv.const_copy(rhs);   return rv; }              // right single and belong to left
    else   { rv.rtype = TR_Empty;  return rv; }              // right single but not belong to left
  } else {
    rv.cr.first  = cr.first  > rhs.cr.first  ? rhs.cr.first  : cr.first ;
    rv.cr.second = cr.second < rhs.cr.second ? rhs.cr.second : cr.second;
    if(rv.cr.first < rv.cr.second) 
           { rv.rtype = TR_Empty;   return rv; }                // no shared area
    else if(rv.cr.first == rv.cr.second) 
           { rv.c = cr.first; rv.rtype = TR_Const; return rv;}  // only one bit
    else   { rv.rtype = TR_CRange;  return rv; }                // const range
  }
}

Range netlist::Range::op_and_tree ( const Range& rhs) const{
  Range rv(op_and(rhs));
  if(rv.is_valid() && !rv.is_empty())
    rv.child = RangeArrayCommon::op_and(rhs.child);
  return rv;
}

Range netlist::Range::op_or ( const Range& rhs) const {
  Range rv;

  if(rtype == TR_Err || rhs.rtype == TR_Err) return rv; // error

  if(rtype == TR_Empty && rhs.rtype == TR_Empty)        // both empty, return empty
    { rv.rtype = TR_Empty; return rv; }

  if(rtype == TR_Empty)          { rv.const_copy(rhs);   return rv; }  // empty, return rhs
  else if(rhs.rtype == TR_Empty) { rv.const_copy(*this); return rv; }  // rhs empty, return this

  if(!is_valuable() || !rhs.is_valuable())                // either variable
    { rv.rtype = TR_Var; return rv; }

  // both const
  if(rtype == TR_Const && rhs.rtype == TR_Const) {
    if(c == rhs.c) { rv.const_copy(rhs);  return rv; }            // equal 
    else           { rv.rtype = TR_Err;   return rv; }            // single and unequal
  }
  
  if(rtype == TR_Const) {
    if(c <= rhs.cr.first+1 && c+1 >= rhs.cr.second) {        // left single and belong to right
      rv.const_copy(rhs);
      rv.cr.first = c > rv.cr.first ? c : rv.cr.first;
      rv.cr.second = c < rv.cr.second ? c : rv.cr.second;
      return rv;
    } else   { rv.rtype = TR_Err;  return rv; }              // left single but not belong to right
  } else if(rhs.rtype == TR_Const) {
    if(rhs.c <= cr.first+1 && rhs.c+1 >= cr.second) {        // right single and belong to left
      rv.const_copy(*this);
      rv.cr.first = rhs.c > rv.cr.first ? rhs.c : rv.cr.first;
      rv.cr.second = rhs.c < rv.cr.second ? rhs.c : rv.cr.second;
      return rv;
    } else   { rv.rtype = TR_Err;  return rv; }              // right single but not belong to left
  } else {
    if(cr.second <= rhs.cr.first+1 || cr.first+1 >= rhs.cr.second) { // adjacent
      rv.cr.first  = cr.first  > rhs.cr.first  ? cr.first  : rhs.cr.first ;
      rv.cr.second = cr.second < rhs.cr.second ? cr.second : rhs.cr.second;
      rv.rtype = TR_CRange; return rv;
    } else   { rv.rtype = TR_Err;  return rv; }              // not adjacent
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

bool netlist::Range::op_equ_tree(const Range& rhs) const {
  bool rv = true;
  rv &= op_equ(rhs);
  if(rv) rv &= RangeArrayCommon::op_equ(rhs.child);
  return rv;
}

bool netlist::Range::op_belong_to(const Range& rhs) const {
  if(rtype == TR_Err || rhs.rtype == TR_Err) return false;      // error
  if(rtype == TR_Empty) return true;                            // empty
  else if(rhs.rtype == TR_Empty) return false;                  // rhs empty

  if(!rhs.is_valuable()) return true;                           // rhs variable
  else if(!is_valuable())  return false;                        // variable

  // both const
  if(rtype == TR_Const && rhs.rtype == TR_Const) {
    if(c == rhs.c) return true;                                 // equal 
    else return false;                                          // single and unequal
  }
  
  if(rtype == TR_Const) {
    if(c <= rhs.cr.first && c >= rhs.cr.second) return true;    // left single and belong to right
    else  return false;                                         // left single but not belong to right
  } else if(rhs.rtype == TR_Const) {
    if(rhs.c == cr.first+1 && rhs.c == cr.second) return true;  // equal
    else return false;                                          // not equal
  } else {
    if(cr.first <= rhs.cr.first && cr.second >= rhs.cr.second)  // belong to 
      return true;
    else return false;                                          // not belong to 
  }
}

bool netlist::Range::op_adjacent_to(const Range& rhs) const {
  if(rtype == TR_Err || rhs.rtype == TR_Err) return false;      // error
  if(rtype == TR_Empty || rhs.rtype == TR_Empty) return false;  // either empty
  if(!is_valuable() || !rhs.is_valuable()) return true;         // either variable

  // both const
  if(rtype == TR_Const && rhs.rtype == TR_Const) {              // both const single 
    if(c <= rhs.c+1 && c+1 >= rhs.c) return true;               // adjacent
    else return false;                                          // not adjacent
  }
  if(rtype == TR_Const) {                                       // left const single
    if(c <= rhs.cr.first+1 && c+1 >= rhs.cr.second) return true;
    else return false;
  } else if(rhs.rtype == TR_Const) {                            // right const single
    if(rhs.c <= cr.first+1 && rhs.c+1 >= cr.second) return true;
    else return false;
  } else {                                                      // both const range 
    if(cr.second <= rhs.cr.first+1 || rhs.cr.second <= cr.first + 1) 
      return true;
    else return false;
  }
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
