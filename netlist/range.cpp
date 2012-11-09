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
#include "sdfg/rtree.hpp"

using namespace netlist;
using std::ostream;
using std::string;
using std::vector;
using std::pair;
using boost::shared_ptr;
using shell::location;

netlist::Range::Range() 
  : NetComp(tRange), dim(false), rtype(TR_Err) { }

netlist::Range::Range(const mpz_class& sel)
  : NetComp(tRange), c(sel), dim(false), rtype(TR_Const) {  }

netlist::Range::Range(const location& lloc, const mpz_class& sel)
  : NetComp(tRange, lloc), c(sel), dim(false), rtype(TR_Const) {  }

netlist::Range::Range(const Number& rl, const Number& rr)
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

pair<long, long> netlist::Range::get_plain_range() const {
  switch(rtype) {
  case TR_Const:
    return pair<long, long>(c.get_value().get_si(),c.get_value().get_si());
  case TR_CRange:
    return pair<long, long>(cr.first.get_value().get_si(), cr.second.get_value().get_si());
  default:
    assert(0 == "Wrong range type!");
    return pair<long, long>();
  }
}

bool netlist::Range::is_valuable_tree() const {
  bool rv = rtype == TR_Const || rtype == TR_CRange|| rtype == TR_Empty;
  if(rv)
    return RangeArrayCommon::is_valuable();
  else return false;
}

bool netlist::Range::is_selection(bool& m_leaf) const {
  switch(rtype) {
  case TR_Const:
  case TR_Var:
    if(m_leaf) return false;    // no further selection after a leaf node
    else return true;
  case TR_CRange:
    if(m_leaf) return false;    // no further selection after a lead node
    else { m_leaf = true; return true; } // set leaf node
  default:
    return false;
  }
}
    

Range netlist::Range::const_copy(bool tree, const Range& maxRange) const {
  Range rv;
  rv.loc = loc;
  rv.rtype = rtype;
  switch(rtype) {
  case TR_Err: 
  case TR_Empty: break;
  case TR_Const: rv.c = c; break;
  case TR_Var: {
      // maxRange is needed
      if(v->is_valuable()) {
        rv.c = v->get_value();
        rv.rtype = TR_Const;
      } else {
        assert(maxRange.is_valid());
        rv.cr = maxRange.cr;
        rv.c = maxRange.c;
        rv.rtype = maxRange.rtype;
      }
      break;
    }
  case TR_Range: {
    // maxRange is needed
    assert(maxRange.is_valid());
    rv.cr.first = r.first->is_valuable() ? r.first->get_value() : 
      maxRange.rtype == TR_Const ? maxRange.c : maxRange.cr.first;
    rv.cr.second = r.second->is_valuable() ? r.second->get_value() : 
      maxRange.rtype == TR_Const ? maxRange.c : maxRange.cr.second;
    assert(rv.cr.first >= rv.cr.second);
    if(rv.cr.first == rv.cr.second) { // reducible to single
      rv.c = rv.cr.first;
      rv.rtype = TR_Const;
    } else rv.rtype = TR_CRange;
    break;
  }
  case TR_CRange: rv.cr = cr;
  }

  rv.dim = dim;
  if(tree) rv.child = RangeArrayCommon::const_copy(maxRange.RangeArrayCommon::is_empty() ? Range() : *(maxRange.child.front()));
  rv.width = width;
  return rv;
}

Range netlist::Range::op_and ( const Range& rhs) const{
  Range rv;
  if(rtype == TR_Err || rhs.rtype == TR_Err) return rv; // error
  assert(is_valuable() && rhs.is_valuable());           // no variable range

  rv.rtype = TR_Empty;
  if(rtype == TR_Empty || rhs.rtype == TR_Empty)        // either empty
    return rv;

  if(rtype == TR_Const && rhs.rtype == TR_Const) {
    if(c == rhs.c) return const_copy();                 // equal 
    else           return rv;                           // single and unequal
  }
  
  if(rtype == TR_Const) {
    if(c <= rhs.cr.first && c >= rhs.cr.second) 
           return const_copy();                         // left single and belong to right
    else   return rv;                                   // left single but not belong to right
  } else if(rhs.rtype == TR_Const) {
    if(rhs.c <= cr.first && rhs.c >= cr.second) 
           return rhs.const_copy();                     // right single and belong to left
    else   return rv;                                   // right single but not belong to left
  } else {
    rv.cr.first  = cr.first  > rhs.cr.first  ? rhs.cr.first  : cr.first ;
    rv.cr.second = cr.second < rhs.cr.second ? rhs.cr.second : cr.second;
    if(rv.cr.first < rv.cr.second) 
           return rv;                                   // no shared area
    else if(rv.cr.first == rv.cr.second) 
           { rv.c = rv.cr.first; rv.rtype = TR_Const; return rv;}  // only one bit
    else   { rv.rtype = TR_CRange;  return rv; }                // const range
  }
}

Range netlist::Range::op_and_tree ( const Range& rhs) const{
  Range rv(op_and(rhs));
  if(!rv.is_empty()) {
    // they must both have childs or both leaves
    assert((!child.empty() && !rhs.child.empty()) ||(child.empty() && rhs.child.empty()));
    rv.child = RangeArrayCommon::op_and(rhs.child);
    if((rv.child.size() == 1) && (rv.child.front()->is_empty())) {
      rv.child.clear();
      rv.rtype = TR_Empty;
    }
  }
  //std::cout << "Range &-tree: " << *this << "; " << rhs << "; " << rv << std::endl;
  return rv;
}


Range netlist::Range::op_or ( const Range& rhs) const {
  Range rv;
  if(rtype == TR_Err || rhs.rtype == TR_Err) return rv; // error
  assert(is_valuable() && rhs.is_valuable());           // no variable range

  rv.rtype = TR_Empty;
  if(rtype == TR_Empty && rhs.rtype == TR_Empty)        // both empty, return empty
    return rv;

  if(rtype == TR_Empty)          return rhs.const_copy();  // empty, return rhs
  else if(rhs.rtype == TR_Empty) return const_copy();      // rhs empty, return this

  rv.rtype = TR_Err;
  if(rtype == TR_Const && rhs.rtype == TR_Const) {
    if(c == rhs.c) return rhs.const_copy();             // equal
    else if(c == rhs.c + 1)                             // adjacent
      { rv.cr.first = c; rv.cr.second = rhs.c; rv.rtype = TR_CRange; return rv; }
    else if(c + 1 == rhs.c)                             // adjacent
      { rv.cr.first = rhs.c; rv.cr.second = c; rv.rtype = TR_CRange; return rv; }
    else           return rv;                           // single and unequal
  }
  
  if(rtype == TR_Const) {
    if(c <= rhs.cr.first+1 && c+1 >= rhs.cr.second) {   // left single and belong to right
      rv = rhs.const_copy();
      rv.cr.first = c > rv.cr.first ? c : rv.cr.first;
      rv.cr.second = c < rv.cr.second ? c : rv.cr.second;
      return rv;
    } else return rv;                                   // left single but not belong to right
  } else if(rhs.rtype == TR_Const) {
    if(rhs.c <= cr.first+1 && rhs.c+1 >= cr.second) {   // right single and belong to left
      rv = const_copy();
      rv.cr.first = rhs.c > rv.cr.first ? rhs.c : rv.cr.first;
      rv.cr.second = rhs.c < rv.cr.second ? rhs.c : rv.cr.second;
      return rv;
    } else return rv;                                   // right single but not belong to left
  } else {
    if(cr.second <= rhs.cr.first+1 || cr.first+1 >= rhs.cr.second) { // adjacent
      rv.cr.first  = cr.first  > rhs.cr.first  ? cr.first  : rhs.cr.first ;
      rv.cr.second = cr.second < rhs.cr.second ? cr.second : rhs.cr.second;
      rv.rtype = TR_CRange; return rv;
    } else return rv;                                   // not adjacent
  }
}

vector<Range> netlist::Range::op_deduct (const Range& rhs) const {
  vector<Range> rv(2);
  assert(is_valid() && is_valuable() && rhs.is_valid() && rhs.is_valuable());
  if(rtype == TR_Empty) { rv[0].set_empty(); rv[1].set_empty(); }
  else if(rhs.rtype == TR_Empty) { rv[0] = rhs.const_copy(); rv[1].set_empty(); }
  else if(rtype == TR_Const && rhs.rtype == TR_Const) {         // both const
    if(c > rhs.c)       { rv[0] = const_copy(); rv[1].set_empty();    }
    else if(c == rhs.c) { rv[0].set_empty();    rv[1].set_empty();    }
    else                { rv[0].set_empty();    rv[1] = const_copy(); }
  } else if(rtype == TR_Const && rhs.rtype == TR_CRange) { // rhs range
    if(c > rhs.cr.first) { rv[0] = const_copy(); rv[1].set_empty();    }
    else if(c <= rhs.cr.first && c >= rhs.cr.second)
                         { rv[0].set_empty();    rv[1].set_empty();    }
    else                 { rv[0].set_empty();    rv[1] = const_copy(); }
  } else if(rtype == TR_CRange && rhs.rtype == TR_Const) { // this range
    rv[0] = const_copy(); rv[1] = const_copy();
    rv[0].cr.second = rhs.c + 1 > cr.second ? rhs.c + 1 : cr.second;
    rv[1].cr.first = rhs.c - 1 < cr.first ? rhs.c - 1 : cr.first;

    if(rv[0].cr.first == rv[0].cr.second) { 
      rv[0].c = rv[0].cr.first; rv[0].rtype = TR_Const;
    } else if(rv[0].cr.first < rv[0].cr.second) rv[0].set_empty();

    if(rv[1].cr.first == rv[1].cr.second) { 
      rv[1].c = rv[1].cr.first; rv[1].rtype = TR_Const;
    } else if(rv[1].cr.first < rv[1].cr.second) rv[0].set_empty(); 
  } else {                      // both ranges
    rv[0] = const_copy(); rv[1] = const_copy();
    rv[0].cr.second = rhs.cr.first >= cr.second ? rhs.cr.first + 1 : cr.second;
    rv[1].cr.first = rhs.cr.second <= cr.first ? rhs.cr.second - 1 : cr.first;
    if(rv[0].cr.first == rv[0].cr.second) { 
      rv[0].c = rv[0].cr.first; rv[0].rtype = TR_Const;
    } else if(rv[0].cr.first < rv[0].cr.second) rv[0].set_empty();

    if(rv[1].cr.first == rv[1].cr.second) { 
      rv[1].c = rv[1].cr.first; rv[1].rtype = TR_Const;
    } else if(rv[1].cr.first < rv[1].cr.second) rv[1].set_empty();
  }
  return rv;
}

vector<Range> netlist::Range::op_normalise_tree(const Range& rhs) const {
  vector<Range> rv(3);
  assert(is_valid() && is_valuable() && rhs.is_valid() && rhs.is_valuable());

  // calculate the shared area
  rv[1] = *this & rhs;
  if(!rv[1].is_empty()) rv[1].child = RangeArrayCommon::op_or(rhs.child);
  
  // get the higher area and the lower area
  vector<Range> lhs_rhs = op_deduct(rhs);
  vector<Range> rhs_lhs = rhs.op_deduct(*this);

  // the higher area
  if(!lhs_rhs[0].is_empty()) {
    assert(rhs_lhs[0].is_empty());
    rv[0] = lhs_rhs[0].const_copy();
    rv[0].child = RangeArrayCommon::const_copy(Range());
  } else if(!rhs_lhs[0].is_empty()) {
    assert(lhs_rhs[0].is_empty());
    rv[0] = rhs_lhs[0].const_copy();
    rv[0].child = rhs.RangeArrayCommon::const_copy(Range());
  } else rv[0].set_empty();
    
  // the lower area
  if(!lhs_rhs[1].is_empty()) {
    assert(rhs_lhs[1].is_empty());
    rv[1] = lhs_rhs[1].const_copy();
    rv[1].child = RangeArrayCommon::const_copy(Range());
  } else if(!rhs_lhs[0].is_empty()) {
    assert(lhs_rhs[0].is_empty());
    rv[1] = rhs_lhs[0].const_copy();
    rv[1].child = rhs.RangeArrayCommon::const_copy(Range());
  } else rv[1].set_empty();
  return rv;
}

bool netlist::Range::op_equ(const Range& rhs) const {
  switch(rtype) {
  case TR_Empty: return rhs.rtype == TR_Empty;
  case TR_Const: return rhs.rtype == TR_Const && c == rhs.c;
  case TR_CRange: return rhs.rtype == TR_CRange && cr.first == rhs.cr.first && cr.second == rhs.cr.second;
  default: return false;
  }
}

bool netlist::Range::op_equ_tree(const Range& rhs) const {
  bool rv = true;
  rv &= op_equ(rhs);
  if(rv && !is_empty()) rv &= RangeArrayCommon::op_equ(rhs.child);
  return rv;
}

bool netlist::Range::op_belong_to(const Range& rhs) const {
  if(rtype == TR_Err || rhs.rtype == TR_Err) return false;      // error
  assert(is_valuable() && rhs.is_valuable());
  if(rtype == TR_Empty) return true;                            // empty
  else if(rhs.rtype == TR_Empty) return false;                  // rhs empty

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
  assert(is_valuable() && rhs.is_valuable());
  if(rtype == TR_Empty || rhs.rtype == TR_Empty) return false;  // either empty

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

bool netlist::Range::op_higher(const Range& rhs) const {
  if(rhs.rtype == TR_Empty) return false;
  if(rtype == TR_Empty) return true;
  assert(is_valuable() && rhs.is_valuable());
  Number first = rtype == TR_Const ? c : cr.first;
  Number second = rhs.rtype == TR_Const ? rhs.c : rhs.cr.first;
  return first > second;
}

void netlist::Range::get_flat_range(const Range& select, pair<Number, Number>& flat_range) const {
  // calculate the size of this dimension
  Number dim_size;
  Number base;
  if(rtype == TR_CRange) {
    base = cr.second;
    dim_size = cr.first - cr.second + 1;
  } else {
    dim_size = 1;
    base = c;
  }

  // update flat range
  flat_range.first = flat_range.first * dim_size;
  flat_range.second = flat_range.second * dim_size;
  if(select.is_valid()) {
    if(select.child.empty() || select.rtype == TR_CRange) {    // leaf
      if(select.rtype == TR_CRange) {
        flat_range.first = flat_range.first + select.cr.first - base + 1;
        flat_range.second = flat_range.second + select.cr.second - base;
      } else {
        flat_range.first = flat_range.first + select.c - base + 1;
        flat_range.second = flat_range.second + select.c - base;
      }
    } else {                    // not leaf
      assert(select.rtype == TR_Const);
      flat_range.first = flat_range.first + select.c - base;
      flat_range.second = flat_range.second + select.c - base;
    }
  }
  
  if(child.empty()) return;
  else {
    if(!select.is_valid() || select.child.empty() || select.rtype == TR_CRange)
      front().get_flat_range(Range(), flat_range);
    else
      front().get_flat_range(select.front(), flat_range);
    return;
  }
}
      

Range& netlist::Range::const_reduce(const Range& maxRange) {
  switch(rtype) {
  case TR_Err:
  case TR_Empty: child.clear(); break;
  case TR_Range:
  case TR_Var: *this = const_copy(true, maxRange); // convert variable range to const range
  case TR_Const:
  case TR_CRange: 
    if(child.size()) {          // non-leaf
      RangeArrayCommon::const_reduce(maxRange.RangeArrayCommon::is_empty() ? Range() : *(maxRange.child.front()));
    }
  }
  return *this;
}

void netlist::Range::reduce(bool dim) {
  switch(rtype) {
  case TR_Range: {
    r.first->reduce();
    r.second->reduce();
    if(r.first->is_valuable() && r.second->is_valuable()) {
      Number h = r.first->get_value();
      Number l = r.second->get_value();
      if(!dim && h == l) {
        rtype = TR_Const;
        c = h;
      } else if(h <= l) {
        rtype = TR_CRange;
        cr.first = l;
        cr.second = h;
      } else {
        rtype = TR_CRange;
        cr.first = h;
        cr.second = l;
      }
    }
    break;
  }
  case TR_Var: {
    v->reduce();
    if(v->is_valuable()) {
      rtype = TR_Const;
      c = v->get_value();
    }
    break;
  }
  case TR_Const:
  case TR_CRange:
    break;
  default:
    child.clear();
  }

  if(child.size()) {          // non-leaf
    RangeArrayCommon::reduce(dim);
  }
}

ostream& netlist::Range::streamout(ostream& os, unsigned int indent, const string& prefix, bool decl, bool dim_or_range) const {
  std::ostringstream sos;
  sos << string(indent, ' ') << prefix;
  if( (decl && dim_or_range && dim) ||     // declaration, show dimension field 
      (decl && !dim_or_range && !dim ) ||  // declaration, show range field
      (!decl)                              // selector
      ) {
    sos << "[";
    switch(rtype) {
    case TR_Const: {
      sos << c.get_value();
      if(decl) sos << ":" << c.get_value();
      break;
    }
    case TR_Var: {
      sos << *v;
      assert(!decl);              // this is odd, assert to check the first legal situation
      if(decl) sos << ":" << *v;
      break;
    }
    case TR_Range: {
      if(r.first->is_valuable())  sos << r.first->get_value().get_value();
      else                        sos << *(r.first); 
      sos << ":"; 
      if(r.second->is_valuable()) sos << r.second->get_value().get_value();
      else                        sos << *(r.second); 
      break;
    }
    case TR_CRange: sos << cr.first.get_value() << ":" << cr.second.get_value(); break;
    case TR_Empty: break;
    default: // should not go here
      assert(0 == "Wrong range type");
    }
    sos << "]";
  }

  if(child.empty()) {           // the leaf node
    os << sos.str();
  } else {
    RangeArrayCommon::streamout(os, 0, sos.str(), decl, dim_or_range);
  }
  return os;
}

void netlist::Range::db_register(int) {
  switch(rtype) {
  case TR_Var: v->db_register(1); break;
  case TR_Range: r.first->db_register(1); r.second->db_register(1); break;
  default: ;
  }
  RangeArrayCommon::db_register(1);
}

void netlist::Range::db_expunge() {
  switch(rtype) {
  case TR_Var: v->db_expunge(); break;
  case TR_Range: r.first->db_expunge(); r.second->db_expunge(); break;
  default: ;
  }
  RangeArrayCommon::db_expunge();
}

void netlist::Range::set_father(Block *pf) {
  if(father == pf) return;
  father = pf;
  switch(rtype) {
  case TR_Var: v->set_father(pf); break;
  case TR_Range: r.first->set_father(pf); r.second->set_father(pf); break;
  default: ;
  }
  RangeArrayCommon::set_father(pf);
}

ostream& netlist::Range::streamout(ostream& os, unsigned int indent) const {
  return streamout(os, indent, "");
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
  rv->child = RangeArrayCommon::deep_copy();
  rv->loc = loc;
  rv->width = width;
  return rv;
}


// get the data width represented by this range
unsigned int netlist::Range::get_width() const {
  unsigned int rv = 0;
  switch(rtype) {
  case TR_Const:
  case TR_Var:
    rv = 1;
    break;
  case TR_Range:
    assert(0 == "unable to calculate the width of a variable range expression!");
    break;
  case TR_CRange: {
    long m = (cr.first - cr.second + Number(1)).get_value().get_si();
    assert(m >= 0);
    rv = static_cast<unsigned int>(m);
    break;
  }
  default:
    break;
  }
  return rv;
}
unsigned int netlist::Range::get_width(const Range& r) {
  if(width) return width;
  width = get_width();
  if(!r.child.empty()) {
    if(child.empty()) 
      width *= r.RangeArrayCommon::get_width(*(r.child.front()));
    else 
      width *= RangeArrayCommon::get_width(*(r.child.front()));
  }
  return width;
}

unsigned int netlist::Range::get_width(const Range& r) const {
  if(width) return width;
  unsigned int rv = get_width();
  if(!r.child.empty()) {
    if(child.empty()) 
      rv *= r.RangeArrayCommon::get_width(*(r.child.front()));
    else 
      rv *= RangeArrayCommon::get_width(*(r.child.front()));
  }
  return rv;
}

// set a new range value by width
void netlist::Range::set_width(const unsigned int& w, const Range& r) {
  assert(get_width(r) >= w);
  if(get_width(r) == w) return;
  // need to reduce the size
  if(!r.child.empty()) {        // the current range must be 1-bit wide
    assert(1 == get_width());
    if(!child.empty()) child = r.RangeArrayCommon::const_copy(*(r.child.front()));
    RangeArrayCommon::set_width(w, *(r.child.front()));
  } else {
    assert(w > 0);
    assert(rtype == TR_CRange);
    if(w == 1) {
      rtype = TR_Const;
      c = cr.second;
    } else {
      cr.first = cr.second + Number(static_cast<int>(w - 1));
    }
  }
  width = w;
}

void netlist::Range::scan_vars(shared_ptr<SDFG::RForest> rf, bool ctl) const {
  if(rtype == TR_Var)
    v->scan_vars(rf, ctl);
  
  if(rtype == TR_Range) {
    r.first->scan_vars(rf, ctl);
    r.second->scan_vars(rf, ctl);
  }

  RangeArrayCommon::scan_vars(rf, ctl);
    
}

void netlist::Range::replace_variable(const VIdentifier& var, const Number& num) {
  if(rtype == TR_Var)
    v->replace_variable(var, num);
  
  if(rtype == TR_Range) {
    r.first->replace_variable(var, num);
    r.second->replace_variable(var, num);
  }

  RangeArrayCommon::replace_variable(var, num);
  
}
