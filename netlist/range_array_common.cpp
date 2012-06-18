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
 * The common base class shared between range array and range
 * 15/06/2012   Wei Song
 *
 *
 */

#include "component.h"
#include <algorithm>

using namespace netlist;
using std::ostream;
using std::string;
using std::vector;
using std::list;
using std::for_each;
using boost::shared_ptr;
using shell::location;

bool netlist::RangeArrayCommon::is_valuable() const {
  bool rv = true;
  for_each(child.begin(), child.end(), [&rv](const shared_ptr<Range>& m) {
      rv &= m->is_valuable_tree();
    });
  return rv;
}

list<shared_ptr<Range> > netlist::RangeArrayCommon::const_copy(const Range& maxRange) const {
  list<shared_ptr<Range> > rv;
  for_each(child.begin(), child.end(), [&rv, &maxRange](const shared_ptr<Range>& m) {
      rv.push_back(shared_ptr<Range>(new Range(m->const_copy(true, maxRange))));
    });
  return rv;
}

list<shared_ptr<Range> > netlist::RangeArrayCommon::op_and(const list<shared_ptr<Range> >& rhs) const {
  assert(child.size() + rhs.size() > 0);
  
  if(child.size() == 0) return rhs;
  if(rhs.size() == 0) return child;
  
  // (a + b) & (c + d) = ac + ad + bc + bd
  list<shared_ptr<Range> > rv;
  list<shared_ptr<Range> >::const_iterator lit, lend, rit, rend;
  for(lit=child.begin(), lend=child.end(); lit!=lend; ++lit) {
    for(rit=rhs.begin(), rend=rhs.end(); rit!=rend; ++rit) {
      Range m = (*lit)->op_and_tree(**rit);
      assert(m.is_valid());     // & operation should not generate error range from any normal ranges
      if(!m.is_empty()) {       // insert a new 
        rv.push_back(shared_ptr<Range>(new Range(m)));
      }
    }
  }
  return rv;
}

list<shared_ptr<Range> > netlist::RangeArrayCommon::op_or(const list<shared_ptr<Range> >& rhs, const Range& maxRange) const {
  list<shared_ptr<Range> > rv = const_copy(maxRange);
  rv.insert(rv.end(), rhs.begin(), rhs.end()); // combine two lists
  sort(rv);                     // reorder it
  rv = const_reduce(rv, maxRange); // reduce the list
  return rv;
}

bool netlist::RangeArrayCommon::op_equ(const list<shared_ptr<Range> >& rhs) const {
  bool rv = true;
  // now it is still a naive compare, if there are multiple variable range, the comparison can fail.
  list<shared_ptr<Range> >::const_iterator lit, lend, rit, rend;
  lit = child.begin();
  lend = child.end();
  rit = rhs.begin();
  rend = rhs.end();

  //compare all childs
  while(lit != lend && rit != rend) {
    rv &= (*lit)->op_equ_tree(**rit);
    lit++;
    rit++;
  }

  // check that both range array are travelled
  if(lit!=lend || rit != rend) rv = false;

  return rv;

}

void netlist::RangeArrayCommon::const_reduce(const Range& maxRange) {
  child = const_reduce(child, maxRange);
}

list<shared_ptr<Range> > netlist::RangeArrayCommon::const_reduce(const list<shared_ptr<Range> >& rhs, const Range& maxRange) const {

  // preprocess
  // reduce all
  list<shared_ptr<Range> > rlist;
  for_each(rhs.begin(), rhs.end(), [&rlist, &maxRange](const shared_ptr<Range>& m) {
      Range tmp = m->const_copy(true, maxRange);
      tmp.const_reduce(maxRange);
      if(tmp.is_valid() && !tmp.is_empty()) 
        rlist.push_back(shared_ptr<Range>(new Range(tmp)));
          });
  
  // do the reduction in iterations
  bool changed;
  do {
    changed = false;
    if(rlist.size() <= 1) return rlist;
    // now all sub-ranges are reduced and non-empty
    list<shared_ptr<Range> >::iterator it, next, end;
    it = rlist.begin();
    next = rlist.begin();
    next++;
    end = rlist.end();
    while(next != end) {
      Range tmp = (*it)->op_and(**next);
      if((*it)->op_adjacent_to(**next)) { // adjacent
        if((*it)->RangeArrayCommon::op_equ((*next)->child)) { // child equal
          **it = **it | **next;
          (*it)->child = (*next)->child;
          next = rlist.erase(next);
          changed = true;
        } else if(tmp.is_valid() && !tmp.is_empty()) {
          vector<Range> normResult = (*it)->op_normalise_tree(**next, maxRange);
          if(normResult[0].is_valid() && !normResult[0].is_empty())
            rlist.insert(it, shared_ptr<Range>(new Range(normResult[0])));
          if(normResult[1].is_valid() && !normResult[1].is_empty())
            rlist.insert(it, shared_ptr<Range>(new Range(normResult[1])));
          if(normResult[2].is_valid() && !normResult[2].is_empty())
            rlist.insert(it, shared_ptr<Range>(new Range(normResult[2])));
          it = rlist.erase(it);
          it--;
          next = rlist.erase(next);
          changed = true;
        } else {                // adjacent but not shared
          it++; next++;
        }
      } else {
        it++; next++;
      }
    }
    // reorder the list
    sort(rlist);
  } while (changed);

  // final check
  assert(rlist.size() > 0);
  if( rlist.size() == 1 && maxRange.is_valid() &&
      *(rlist.front()) == maxRange && // full range
      rlist.front()->child.size() == 0 // sub-ranges also full
      )
    rlist.clear();

  return rlist;
}

void netlist::RangeArrayCommon::sort(list<shared_ptr<Range> >& rhs) const {
  rhs.sort([](shared_ptr<Range>& first, shared_ptr<Range>& second) -> bool {
    return *first > *second;
  });
  
}
