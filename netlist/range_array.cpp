/*
 * Copyright (c) 2011-2013 Wei Song <songw@cs.man.ac.uk> 
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
 * Range array
 * 14/06/2012   Wei Song
 *
 *
 */

#include "component.h"
#include "sdfg/rtree.hpp"
#include <algorithm>

using namespace netlist;
using std::ostream;
using std::string;
using std::list;
using std::vector;
using std::pair;
using std::for_each;
using boost::shared_ptr;
using shell::location;

netlist::RangeArray::RangeArray() : NetComp(tRangeArray), const_reduced(false){}

netlist::RangeArray::RangeArray(const list<shared_ptr<Range> >& rhs) 
  : NetComp(tRangeArray), RangeArrayCommon(rhs), const_reduced(false){ }

bool netlist::RangeArray::is_valuable() {
  // if once const reduced, it must be valuable
  if(const_reduced || child.empty() ) return true;
  
  // otherwise check it
  bool rv = RangeArrayCommon::is_valuable();

  if(rv) const_reduced = true;
  return rv;
}

bool netlist::RangeArray::is_valuable() const {
  // if once const reduced, it must be valuable
  if(const_reduced || child.empty() ) return true;
  
  // otherwise check it
  return RangeArrayCommon::is_valuable();
}

bool netlist::RangeArray::is_declaration() const {
  if(child.empty()) return true; // no range at all means 1 bit
  else if(child.size() > 1) return false;
  else {
    Range m = *(child.front());
    while(true) {
      if(m.is_empty()) return false; // declaration should not have empty range expression
      if(m.RangeArrayCommon::size() > 1) return false;
      if(m.RangeArrayCommon::size() == 1) m = m.RangeArrayCommon::front();
      else 
        break;
    }
    return true;
  }
}

bool netlist::RangeArray::is_selection() const {
  if(child.empty()) return true; // no range at all means 1 bit
  else if(child.size() > 1) return false;
  else {
    Range m = *(child.front());
    bool m_leaf = false;
    while(true) {
      if(!m.is_selection(m_leaf)) return false;
      if(m.RangeArrayCommon::size() > 1) return false;
      if(m.RangeArrayCommon::size() == 1) {
        m = m.RangeArrayCommon::front();
      }
      else 
        break;
    }
    return true;
  }
}

RangeArray netlist::RangeArray::const_copy(const RangeArray& maxRange) const {
  RangeArray rv;
  // if maxRange is empty, it is a one-bit signal
  rv.child = RangeArrayCommon::const_copy( maxRange.child.empty() ?
                                           Range(0) : maxRange.front()
                                           );
  rv.const_reduced = const_reduced;
  rv.width = width;
  return rv;
}

RangeArray netlist::RangeArray::deep_object_copy() const {
  RangeArray rv;
  for_each(child.begin(), child.end(), [&](const shared_ptr<Range>& m) {
      rv.child.push_back(shared_ptr<Range>(m->deep_copy()));
    });
  rv.const_reduced = const_reduced;
  rv.width = width;
  return rv;
}

RangeArray& netlist::RangeArray::const_reduce(const RangeArray& maxRange) {
  if(maxRange.child.empty()) return *this;
  RangeArrayCommon::const_reduce(maxRange.front());
  const_reduced = true;
  return *this;
}

void netlist::RangeArray::reduce(bool dim) {
  RangeArrayCommon::reduce(dim);
}

RangeArray netlist::RangeArray::op_and(const RangeArray& rhs) const {
  RangeArray rv;
  rv.child = RangeArrayCommon::op_and(rhs.child);
  rv.const_reduced = const_reduced & rhs.const_reduced;
  return rv;
}

RangeArray netlist::RangeArray::op_or(const RangeArray& rhs) const {
  RangeArray rv;
  rv.child = RangeArrayCommon::op_or(rhs.child);
  rv.const_reduced = const_reduced & rhs.const_reduced;
  //std::cout << *this << " | " << rhs << " = " << rv;
  //if(!maxRange.child.empty()) std::cout << " (" << maxRange.front() << ")";
  //std::cout << std::endl;
  return rv;
}

RangeArray netlist::RangeArray::op_deduct(const RangeArray& rhs) const {
  RangeArray rv;
  rv.child = RangeArrayCommon::op_deduct(rhs.child);
  rv.const_reduced = const_reduced & rhs.const_reduced;
  return rv;
}

bool netlist::RangeArray::op_equ(const RangeArray& rhs) const {
  RangeArray m;
  m.child.push_back(shared_ptr<Range>(new Range(0))); // 1-bit
  if(child.empty()) return (rhs.child.empty() || m.RangeArrayCommon::op_equ(rhs.child));
  else if(rhs.child.empty()) return m.RangeArrayCommon::op_equ(child);
  else return RangeArrayCommon::op_equ(rhs.child);
}

Range netlist::RangeArray::get_flat_range(const RangeArray& select) const {
  if(child.empty()) {
    return Range(0);
  }
  else {
    pair<Number, Number> raw_range(0,0);
    if(select.child.empty()) 
      front().get_flat_range(front(), raw_range);
    else {
      assert(*this >= select.const_copy(*this));
      front().get_flat_range(select.front(), raw_range);
    }
    return Range(raw_range.first - 1, raw_range.second);
  }
}

void netlist::RangeArray::add_range(const shared_ptr<Range>& rhs) {
  child.push_back(rhs);
}

void netlist::RangeArray::add_dimension(const shared_ptr<Range>& rhs) {
  rhs->set_child(child);        // set current child(lower dimensions to rhs)
  child.clear();                // clear current top level dimension
  child.push_back(rhs);         // store the new top level to range array
}

void netlist::RangeArray::set_father(Block* pf) {
  if(father == pf) return;
  father = pf;
  RangeArrayCommon::set_father(pf);
}

ostream& RangeArray::streamout (ostream& os, unsigned int indent) const {
  return RangeArrayCommon::streamout(os, indent, "");
}

RangeArray* netlist::RangeArray::deep_copy() const {
  RangeArray* rv = new RangeArray();
  for_each(child.begin(), child.end(), [&](const shared_ptr<Range>& m) {
      rv->child.push_back(shared_ptr<Range>(m->deep_copy()));
    });
  rv->const_reduced = const_reduced;
  rv->width = width;
  return rv;
}

void netlist::RangeArray::db_register(int iod) {
  RangeArrayCommon::db_register(iod);
}

void netlist::RangeArray::db_expunge() {
  RangeArrayCommon::db_expunge();
}

unsigned int netlist::RangeArray::get_width(const RangeArray& r) const {
  if(width) return width;
  return RangeArrayCommon::get_width(*(r.child.front()));
}

unsigned int netlist::RangeArray::get_width(const RangeArray& r) {
  if(width) return width;
  width = RangeArrayCommon::get_width(*(r.child.front()));
  return width;
}
  
void netlist::RangeArray::set_width(const unsigned int& w, const RangeArray& r) {
  assert(get_width(r) >= w);
  if(width != w) {
    RangeArrayCommon::set_width(w, *(r.child.front()));
    width = w;
  }
}

void netlist::RangeArray::replace_variable(const VIdentifier& var, const Number& num) {
  RangeArrayCommon::replace_variable(var, num);
}

void netlist::RangeArray::replace_variable(const VIdentifier& var, const VIdentifier& nvar) {
  RangeArrayCommon::replace_variable(var, nvar);
}

shared_ptr<SDFG::RTree> netlist::RangeArray::get_rtree() const {
  return RangeArrayCommon::get_rtree();
}
