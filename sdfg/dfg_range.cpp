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
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <string>

using std::pair;
using std::list;
using std::string;
using namespace SDFG;

pair<string, dfgRange> SDFG::divide_signal_name(const string& sname) {
  list<string> name_and_range;
  
  // get name and ranges
  boost::char_separator<char> sep("[]");
  boost::tokenizer<boost::char_separator<char> > tokens(sname, sep);
  BOOST_FOREACH(const string& m, tokens) {
    name_and_range.push_back(m);
  }

  // get the signal name
  pair<string, dfgRange> rv;
  rv.first = name_and_range.front();
  name_and_range.pop_front();
  
  // get ranges
  BOOST_FOREACH(const string& m, name_and_range) {
    boost::char_separator<char> rsep(":");
    boost::tokenizer<boost::char_separator<char> > rtokens(m, rsep);
    list<int> ranges;
    BOOST_FOREACH(const string& r, rtokens)
      ranges.push_back(boost::lexical_cast<int>(r));
    
    // push the range to the range expression
    if(ranges.size() == 1)
      rv.second.add_lower(dfgRangeElement(ranges.front()));
    else
      rv.second.add_lower(dfgRangeElement(ranges.front(), ranges.back()));
  }

  return rv;
  
}
