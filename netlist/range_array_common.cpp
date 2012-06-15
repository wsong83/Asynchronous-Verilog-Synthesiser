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
