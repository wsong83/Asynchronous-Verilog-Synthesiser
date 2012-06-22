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

void netlist::RangeArrayCommon::set_dim() {
  for_each(child.begin(), child.end(), [](const shared_ptr<Range>& m) {
      m->set_dim(true);
    });
}  

list<shared_ptr<Range> > netlist::RangeArrayCommon::const_copy(const Range& maxRange) const {
  return const_copy(child, maxRange);
}

list<shared_ptr<Range> > netlist::RangeArrayCommon::const_copy(const list<shared_ptr<Range> >& rhs, const Range& maxRange) const {
  list<shared_ptr<Range> > rv;
  if(rhs.empty() && maxRange.is_valid()) { 
    // when the child is empty but maxRange is valid
    // it is actuall copy from a VIdentifier range
    // in this case, copy the rest from maxRange to this for range calculation
    rv.push_back(shared_ptr<Range>(maxRange.deep_copy()));
  } else {
    for_each(rhs.begin(), rhs.end(), [&rv, &maxRange](const shared_ptr<Range>& m) {
        rv.push_back(shared_ptr<Range>(new Range(m->const_copy(true, maxRange))));
      });
  }
  return rv;
}

list<shared_ptr<Range> > netlist::RangeArrayCommon::op_and(const list<shared_ptr<Range> >& rhs) const {
  if(child.empty() || rhs.empty()) return list<shared_ptr<Range> >();
    
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

  // if empty, identify it explicitly
  if(rv.empty()) {
    rv.push_back(shared_ptr<Range>(new Range()));
    rv.front()->set_empty();
  }
  return rv;
}

list<shared_ptr<Range> > netlist::RangeArrayCommon::op_or(const list<shared_ptr<Range> >& rhs) const {
  list<shared_ptr<Range> > rv = const_copy(Range());
  list<shared_ptr<Range> > mrhs = const_copy(rhs, Range());
  rv.insert(rv.end(), mrhs.begin(), mrhs.end()); // combine two lists
  return const_reduce(rv, Range()); // reduce the list
}

list<shared_ptr<Range> > netlist::RangeArrayCommon::op_deduct(const list<shared_ptr<Range> >& rhs) const {
  list<shared_ptr<Range> >::const_iterator mit, mend, nit, nend;
  list<shared_ptr<Range> > rv = const_copy(Range());
  for(mit=rhs.begin(), mend=rhs.end(); mit!=mend; mit++) {
    list<shared_ptr<Range> > tmp;
    for(nit=rv.begin(), nend=rv.end(); nit!=nend; nit++) {
      Range m_r = **nit & **mit;
      if(!m_r.is_empty()) { // do need a deduct
        vector<Range> m_v = (*nit)->op_deduct(**mit);
        m_r.set_child((*nit)->RangeArrayCommon::op_deduct((*mit)->get_child()));
        for_each(m_v.begin(), m_v.end(), [&tmp](Range& m){
            tmp.push_back(shared_ptr<Range>(new Range(m)));
          });
      } else if(!(*nit)->is_empty()) tmp.push_back(*nit); // otherwise push it back
    }
    rv = tmp;                   // prepare the next iteration
  }

  // sort and reduce the range array
  return const_reduce(rv, Range());
}

bool netlist::RangeArrayCommon::op_equ(const list<shared_ptr<Range> >& rhs) const {
  list<shared_ptr<Range> > mlhs = const_copy(Range()); 
  mlhs = const_reduce(mlhs, Range());
  list<shared_ptr<Range> > mrhs = const_copy(rhs, Range()); 
  mrhs = const_reduce(mrhs, Range());
  if(mlhs.size() != mrhs.size()) return false;
  // STL algorithm and lambda expressions are fantasic!
  return 
    equal(mlhs.begin(), mlhs.end(), mrhs.begin(), 
          [](shared_ptr<Range>& l, shared_ptr<Range>& r) -> bool {
            return *l == *r;
          });
}

ostream& netlist::RangeArrayCommon::streamout(ostream& os, unsigned int indent, const string& prefix, bool decl, bool dim_or_range) const {
  if(child.empty()) os << string(indent, ' ') << prefix; // at least show the prefix when it is empty
  list<shared_ptr<Range> >::const_iterator it, end;
  it = child.begin(); end = child.end();
  while(it != end) {
    (*it)->streamout(os, indent, prefix, decl, dim_or_range);
    it++;
    if(it != end && !decl) os << ", "; // add comma when a range(select) has many sub-ranges
  } 
  return os;
}

void netlist::RangeArrayCommon::const_reduce(const Range& maxRange) {
  child = const_reduce(child, maxRange);
}

void netlist::RangeArrayCommon::add_low_dimension(const shared_ptr<Range>& rhs) {
  if(child.empty()) child.push_back(rhs);
  else {
    for_each(child.begin(), child.end(), [&rhs](shared_ptr<Range>& m) {
        m->RangeArrayCommon::add_low_dimension(rhs);
      });
  }
}

list<shared_ptr<Range> > netlist::RangeArrayCommon::const_reduce(list<shared_ptr<Range> >& rhs, const Range& maxRange) const {

  // preprocess
  // reduce all
  list<shared_ptr<Range> > rlist;
  for_each(rhs.begin(), rhs.end(), [&rlist, &maxRange](shared_ptr<Range>& m) {
      m->const_reduce(maxRange);
      if(m->is_valid() && !m->is_empty()) rlist.push_back(m);
    });
  
  // do the reduction in iterations
  bool changed;
  sort(rlist);
  do {
    changed = false;
    if(rlist.size() <= 1) break;
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
          vector<Range> normResult = (*it)->op_normalise_tree(**next);
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
  if(rlist.empty()) {
    rlist.push_back(shared_ptr<Range>(new Range()));
    rlist.front()->set_empty();
  }

  return rlist;
}

void netlist::RangeArrayCommon::set_father(Block* pf) {
  for_each(child.begin(), child.end(), [&pf](shared_ptr<Range>& m) {
      m->set_father(pf);
    });
}

bool netlist::RangeArrayCommon::check_inparse() {
  bool rv = true;
  for_each(child.begin(), child.end(), [&rv](const shared_ptr<Range>& m) {
      rv &= m->check_inparse();
    });
  return rv;
}


list<shared_ptr<Range> > netlist::RangeArrayCommon::deep_copy() const {
  list<shared_ptr<Range> > rv;
  for_each(child.begin(), child.end(), [&rv](const shared_ptr<Range>& m) {
      rv.push_back(shared_ptr<Range>(m->deep_copy()));
    });
  return rv;
}

void netlist::RangeArrayCommon::db_register(int iod) {
  for_each(child.begin(), child.end(), [&iod](shared_ptr<Range>& m) {
      m->db_register(iod);
    });
}

void netlist::RangeArrayCommon::db_expunge() {
  for_each(child.begin(), child.end(), [](shared_ptr<Range>& m) {
      m->db_expunge();
    });
}

bool netlist::RangeArrayCommon::elaborate(NetComp::elab_result_t &result, const NetComp::ctype_t mctype, const vector<NetComp *>& fp) {
  bool rv = true;
  result = NetComp::ELAB_Normal;
  
  for_each(child.begin(), child.end(), [&result, &mctype, &fp, &rv](shared_ptr<Range>& m) {
      rv &= m->elaborate(result, mctype, fp);
    });

  return rv;
} 

list<shared_ptr<Range> >& netlist::RangeArrayCommon::sort(list<shared_ptr<Range> >& rhs) const {
  rhs.sort([](shared_ptr<Range>& first, shared_ptr<Range>& second) -> bool {
    return *first > *second;
  });
  return rhs;
}
