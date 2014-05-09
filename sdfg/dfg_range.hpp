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

#ifndef _SDFG_RANGE_H_
#define _SDFG_RANGE_H_

#include <list>
#include <utility>

namespace SDFG {
  
  class dfgRangeElement {
  public:
    dfgRangeElement(int);
    dfgRangeElement(int, int);

    bool is_enclosed(const dfgRangeElement&) const;
    bool is_same(const dfgRangeElement&) const;

  private:
    std::pair<int, int> r_pair; // specific range expression
  };

  inline bool operator> (const dfgRangeElement& lhs, const dfgRangeElement& rhs) {
    return lhs.is_enclosed(rhs) && !lhs.is_same(rhs);
  }

  inline bool operator>= (const dfgRangeElement& lhs, const dfgRangeElement& rhs) {
    return lhs.is_enclosed(rhs);
  }

  inline bool operator< (const dfgRangeElement& lhs, const dfgRangeElement& rhs) {
    return rhs.is_enclosed(lhs) && !rhs.is_same(lhs);
  }

  inline bool operator<= (const dfgRangeElement& lhs, const dfgRangeElement& rhs) {
    return rhs.is_enclosed(lhs);
  }
  
  inline bool operator== (const dfgRangeElement& lhs, const dfgRangeElement& rhs) {
    return rhs.is_same(lhs);
  }

  class dfgRange {
  public:    
    void push_front(const dfgRangeElement&);
    void push_back(const dfgRangeElement&);
    bool is_enclosed(const dfgRange&) const;
    bool is_same(const dfgRange&) const;

  private:
    std::list<dfgRangeElement> r_array; // the range array
  };

  inline bool operator> (const dfgRange& lhs, const dfgRange& rhs) {
    return lhs.is_enclosed(rhs) && !lhs.is_same(rhs);
  }

  inline bool operator>= (const dfgRange& lhs, const dfgRange& rhs) {
    return lhs.is_enclosed(rhs);
  }

  inline bool operator< (const dfgRange& lhs, const dfgRange& rhs) {
    return rhs.is_enclosed(lhs) && !rhs.is_same(lhs);
  }

  inline bool operator<= (const dfgRange& lhs, const dfgRange& rhs) {
    return rhs.is_enclosed(lhs);
  }
  
  inline bool operator== (const dfgRange& lhs, const dfgRange& rhs) {
    return rhs.is_same(lhs);
  }

}

#endif
