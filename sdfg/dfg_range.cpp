/*
 * Copyright (c) 2014-2014 Wei Song <songw@cs.man.ac.uk> 
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
 * the range expression used in the SDFG library
 * 09/05/2014   Wei Song
 *
 *
 */

#include "dfg_range.hpp"
#include <boost/foreach.hpp>

using std::pair;
using std::list;


SDFG::dfgRangeElement::dfgRangeElement(int r)
  : r_pair(r, r) {}

SDFG::dfgRangeElement::dfgRangeElement(int lr, int rr)
{
  if(lr >= rr)
    r_pair = pair<int, int>(lr, rr);
  else
    r_pair = pair<int, int>(rr, lr);
}

bool SDFG::dfgRangeElement::is_enclosed(const dfgRangeElement& r) const {
  return (r_pair.first >= r.r_pair.first && r_pair.second <= r.r_pair.second);
}

bool SDFG::dfgRangeElement::is_same(const dfgRangeElement& r) const {
  return (r_pair.first == r.r_pair.first && r_pair.second == r.r_pair.second);
}

void SDFG::dfgRange::push_front(const dfgRangeElement& r) {
  r_array.push_front(r);
}

void SDFG::dfgRange::push_back(const dfgRangeElement& r) {
  r_array.push_back(r);
}

bool SDFG::dfgRange::is_enclosed(const dfgRange& r) const {
  list<dfgRangeElement>::const_iterator lit = r_array.begin();
  list<dfgRangeElement>::const_iterator rit = r.r_array.begin();
  while(lit != r_array.end() && rit != r.r_array.end()) {
    if(!(*lit >= *rit))
      return false;
  }
  if(lit == r_array.end()) 
    return true;   // a sub range is always enclosed to a full range
  else
    return false;  // a full range is not considered enclosed in a sub range
}

bool SDFG::dfgRange::is_same(const dfgRange& r) const {
  list<dfgRangeElement>::const_iterator lit = r_array.begin();
  list<dfgRangeElement>::const_iterator rit = r.r_array.begin();
  while(lit != r_array.end() && rit != r.r_array.end()) {
    if(!(*lit == *rit))
      return false;
  }
  if(lit != r_array.end() || rit != r.r_array.end()) 
    return false;
  else
    return true;
}

