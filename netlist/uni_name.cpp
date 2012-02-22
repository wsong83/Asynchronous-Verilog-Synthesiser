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
 * Generate a unique name for any unnamed components which will be stored in a database.
 * 21/02/2012   Wei Song
 *
 *
 */

#include "component.h"

// for regular expression
#include <boost/regex.hpp>

// large number library
#include <gmpxx.h>

string netlist::UniName::name = "uni_name_0";

string netlist::UniName::uni_name () {

  const boost::regex numbered_name("_(\\d+)\\z");
  boost::smatch mr;
  string rv = netlist::UniName::name;
  
  // update the suffix
  // find the old suffix
  boost::regex_search(netlist::UniName::name, mr, numbered_name);
  mpz_class dd(mr.str().substr(1).c_str(), 10);
  dd = dd + 1;
  netlist::UniName::name = 
    boost::regex_replace(netlist::UniName::name, numbered_name, string("_")+dd.get_str(10));
  
  return name;

}
