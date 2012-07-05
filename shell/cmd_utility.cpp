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
 * Utilities for Tcl command line env.
 * 05/07/2012   Wei Song
 *
 *
 */

#include <list>
#include "cmd_utility.h"
using std::string;
using std::vector;
using std::list;

using namespace Tcl;

vector<string> shell::CMD::argu_parse(const object& obj) {
  string str = obj.get_string(); // fetch the string, seems no other way but read it all
  unsigned int start = 0;        // start point for current sub string
  unsigned int rp = 0;           // current reading point
  unsigned int fp = str.size();  // finish point
  list<string> argu_list;        // tmp storage

  // begin the parsing
  while(rp != fp) {

    // use boost/tokenizer escaped_list_separator !!
  }
}
