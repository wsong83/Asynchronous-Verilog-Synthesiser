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

#ifndef AV_H_RANGE_ARRAY_
#define AV_H_RANGE_ARRAY_

#include "range_array_common.h"

namespace netlist {

  class RangeArray : public NetComp, public RangeArrayCommon {
  public:
    // constructors
    RangeArray() : NetComp(tRangeArray), const_reduced(false){}
    RangeArray(const std::list<boost::shared_ptr<Range> >& rhs) 
      : NetComp(tRangeArray), RangeArrayCommon(rhs), const_reduced(false) { } /* valuable needs to be calculated!! */

    // helpers
    bool is_empty() const { return child.size() == 1 && child.front()->is_empty(); } 
    RangeArray& set_empty() { 
      child.push_back(boost::shared_ptr<Range>(new Range()));
      child.front()->set_empty();
      return *this;
    }
    bool is_valuable();
    bool is_declaration() const;
    // copy the symbolic value of a range array to a new one
    RangeArray const_copy(const RangeArray& mxRange) const;
    // deep copy but return an object rather than a pointer 
    RangeArray deep_object_copy() const;
    // reduce a variable range array to a symbolic one
    RangeArray& const_reduce(const RangeArray& mxRange);
    // return the shared area of two range arrays
    RangeArray op_and(const RangeArray&) const;
    // return the combined area of two range arrays
    RangeArray op_or(const RangeArray&) const;
    // return the area in this but not rhs
    RangeArray op_deduct(const RangeArray&) const;
    bool op_equ(const RangeArray&) const;

    // used in parser
    // add another range area in the top dimension
    void add_range(const boost::shared_ptr<Range>& rhs); 
    // add another level of dimension as the top level
    void add_dimension(const boost::shared_ptr<Range>& rhs); 

    // inherit from NetComp
    NETLIST_SET_FATHER_DECL;
    NETLIST_STREAMOUT_DECL;
    NETLIST_CHECK_INPARSE_DECL;
    RangeArray* deep_copy() const;
    virtual void db_register(int iod = 1);
    virtual void db_expunge();
    NETLIST_ELABORATE_DECL;

  private:
    bool const_reduced;

  };

  inline RangeArray operator& ( const RangeArray& lhs, const RangeArray& rhs) {return lhs.op_and(rhs); };
  inline RangeArray operator| ( const RangeArray& lhs, const RangeArray& rhs) { return lhs.op_or(rhs); }
  inline RangeArray operator- ( const RangeArray& lhs, const RangeArray& rhs) { return lhs.op_deduct(rhs); }
  bool operator>= ( const RangeArray& lhs, const RangeArray& rhs); /* whether rhs belongs to lhs */
  inline bool operator== ( const RangeArray& lhs, const RangeArray& rhs) {
    return lhs.op_equ(rhs);
  }

  NETLIST_STREAMOUT(RangeArray)

}

#endif

// Local Variables:
// mode: c++
// End:
