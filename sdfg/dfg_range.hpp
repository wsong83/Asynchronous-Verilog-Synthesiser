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

#include "cpprange/cpp_range.hpp"

namespace SDFG {

  typedef CppRange::RangeElement<int> dfgRangeElement;
  typedef CppRange::Range<int> dfgRange;
  typedef CppRange::RangeMap<int> dfgRangeMap;

  // helper function to handle the range of signals
  std::pair<std::string, dfgRange> divide_signal_name(const std::string&);
  std::string combine_signal_name(const std::string&, const dfgRange&);

  // helper function to fill the missing lower dimensions
  std::string get_full_selected_name(const std::string& selected_name, 
                                           const std::string& full_range);

}

#endif
