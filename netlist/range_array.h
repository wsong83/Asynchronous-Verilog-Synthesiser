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
    RangeArray() : NetComp(tRangeArray), valuable(false) {}
    RangeArray(const std::list<boost::shared_ptr<Range> >& rhs) 
      : NetComp(tRangeArray), RangeArrayCommon(rhs), valuable(false) { } /* valuable needs to be calculated!! */

    // helpers
    bool empty() const { return child.size() == 0;}
    bool is_valuable() const { return valuable; }
    RangeArray& op_and(const RangeArray&); /* helper for operator & */

    // inherit from NetComp
    //NETLIST_SET_FATHER_DECL;
    //NETLIST_STREAMOUT_DECL;
    //NETLIST_CHECK_INPARSE_DECL;
    virtual RangeArray* deep_copy() const;
    //virtual void db_register(int iod = 1);
    //virtual void db_expunge();
    //NETLIST_ELABORATE_DECL;

  private:
    bool valuable;

  };

  RangeArray operator& ( const RangeArray&, const RangeArray&);
  RangeArray operator| ( const RangeArray&, const RangeArray&);
  bool operator>= ( const RangeArray& lhs, const RangeArray& rhs); /* whether rhs belongs to lhs */

  NETLIST_STREAMOUT(RangeArray)

}

#endif

// Local Variables:
// mode: c++
// End:
