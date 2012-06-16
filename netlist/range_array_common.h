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

#ifndef AVS_H_RANGE_ARRAY_COMMON_
#define AVS_H_RANGE_ARRAY_COMMON_

namespace netlist {

  class RangeArrayCommon {
  public:
    RangeArrayCommon() {}
    RangeArrayCommon(const std::list<boost::shared_ptr<Range> >& rhs) : child(rhs) {}

    // helpers
    // get the shared range of two range arrays
    std::list<boost::shared_ptr<Range> > op_and(const std::list<boost::shared_ptr<Range> >&) const;
    // get the combined range of two range arrays
    std::list<boost::shared_ptr<Range> > op_or(const std::list<boost::shared_ptr<Range> >&) const;  
    // check whether two range arrays are equal
    bool op_equ(const std::list<boost::shared_ptr<Range> >&) const; 
    
  protected:
    std::list<boost::shared_ptr<Range> > child; // the range expressions of the lower dimension

  };

}

#endif /* AVS_H_RANGE_ARRAY_COMMON_ */

// Local Variables:
// mode: c++
// End:
