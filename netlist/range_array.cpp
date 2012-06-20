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
 * Range array
 * 14/06/2012   Wei Song
 *
 *
 */

#include "component.h"
#include <algorithm>

using namespace netlist;
using std::ostream;
using std::string;
using std::vector;
using std::for_each;
using boost::shared_ptr;
using shell::location;

bool netlist::RangeArray::is_valuable() {
  // if once const reduced, it must be valuable
  if(const_reduced) return true;
  
  // other wise check it
  bool rv = true;
  rv = RangeArrayCommon::is_valuable();

  if(rv) const_reduced = true;
  return rv;
}

bool netlist::RangeArray::is_declaration() const {
  if(child.size() != 1) return false;
  else {
    Range& m = *(child.front());
    while(!m.RangeArrayCommon::is_empty()) {
      if(m.RangeArrayCommon::size() != 1) return false;
      else m = m.RangeArrayCommon::front();
    }
    return true;
  }
}

RangeArray netlist::RangeArray::const_copy(const RangeArray& maxRange) const {
  RangeArray rv;
  assert(maxRange.child.size());
  rv.child = RangeArrayCommon::const_copy(maxRange.RangeArrayCommon::front());
  rv.const_reduced = const_reduced;
  return rv;
}

RangeArray netlist::RangeArray::deep_copy() const {
  RangeArray rv;
  for_each(child.begin(), child.end(), [&rv](const shared_ptr<Range>& m) {
      rv.child.push_back(shared_ptr<Range>(m->deep_copy()));
    });
  rv.const_reduced = const_reduced;
  return rv;
}

void netlist::RangeArray::const_reduce(const RangeArray& maxRange) {
  assert(maxRange.child.size());
  RangeArrayCommon::const_reduce(maxRange.RangeArrayCommon::front());
  const_reduced = true;
}

RangeArray netlist::RangeArray::op_and(const RangeArray& rhs) const {
  RangeArray rv;
  rv.child = RangeArrayCommon::op_and(rhs.child);
  rv.const_reduced = const_reduced & rhs.const_reduced;
  return rv;
}

RangeArray netlist::RangeArray::op_or(const RangeArray& rhs,
                                      const RangeArray& maxRange) const {
  RangeArray rv;
  rv.child = RangeArrayCommon::op_or(rhs.child, 
                                     maxRange.RangeArrayCommon::is_empty() ?
                                     Range() : maxRange.front()
                                     );
  rv.const_reduced = const_reduced & rhs.const_reduced;
  return rv;
}

bool netlist::RangeArray::op_equ(const RangeArray& rhs) const {
  return RangeArrayCommon::op_equ(rhs.child);
}

void netlist::RangeArray::set_father(Block* pf) {
  if(father == pf) return;
  father = pf;
  RangeArrayCommon::set_father(pf);
}

ostream& RangeArray::streamout (ostream& os, unsigned int indent) const {
  return RangeArrayCommon::streamout(os, indent, "");
}

bool netlist::RangeArray::check_inparse() {
  return RangeArrayCommon::check_inparse();
}

void netlist::RangeArray::db_register(int iod) {
  RangeArrayCommon::db_register(iod);
}

void netlist::RangeArray::db_expunge() {
  RangeArrayCommon::db_expunge();
}

bool netlist::RangeArray::elaborate(elab_result_t &result, const ctype_t mctype, const vector<NetComp *>& fp) {
  return RangeArrayCommon::elaborate(result, mctype, fp);
}

bool netlist::operator>= (const RangeArray& lhs, const RangeArray& rhs) {
  RangeArray tmp = lhs & rhs;
  return tmp == rhs;
}

