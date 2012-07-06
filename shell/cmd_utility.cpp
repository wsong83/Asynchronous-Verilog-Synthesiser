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
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <algorithm>
#include "cmd_utility.h"

#include <iostream>

using std::string;
using std::vector;
using std::list;
using boost::tokenizer;
using boost::escaped_list_separator;

using namespace Tcl;

vector<string> shell::CMD::tcl_argu_parse(const object& obj) {
  string str = obj.get_string();
  unsigned int start = 0;            // start point
  unsigned int rp = 0;               // current read point
  unsigned int fp = str.size();      // finish point
  list<string> tok_list;             // tmp token storage

  unsigned int bracket_level = 0;
  
  while(rp != fp) {
    if(str[rp] == '{') {
      if(start != rp && bracket_level == 0) { // token ready
        tok_list.push_back(str.substr(start, rp-start));
        start = rp;
      } 
      rp++;
      bracket_level++;
      continue;
    }

    if(str[rp] == '}') {
      bracket_level = bracket_level >= 1 ? bracket_level - 1 : 0;
      if(bracket_level == 0 && start != rp) { // token ready
        tok_list.push_back(str.substr(start, rp-start+1));
        start = rp = rp + 1;
      } else rp++;
      continue;
    }
    
    // blank
    if(bracket_level == 0 && str[rp] == ' ') { // token ready
      if(start != rp) tok_list.push_back(str.substr(start, rp-start));
      start = rp = rp + 1;
      continue;
    } 
    
    // other
    rp++;
  }

  // the last token
  if(start != rp) tok_list.push_back(str.substr(start, rp-start));

  vector<string> rv(tok_list.size());
  rv.assign(tok_list.begin(), tok_list.end());
  return rv;
}

list<string> shell::CMD::tcl_list_break(const std::string& str) {
  unsigned int start = 0;            // start point
  unsigned int rp = 0;               // current read point
  unsigned int fp = str.size();      // finish point
  list<string> tok_list;             // tmp token storage

  unsigned int bracket_level = 0;
  
  while(rp != fp) {
    if(str[rp] == '{') {
      if(start != rp && bracket_level == 0) { // token ready
        tok_list.push_back(str.substr(start, rp-start));
        start = rp;
      }
      if(bracket_level == 0) start = rp + 1;
      rp++;
      bracket_level++;
      continue;
    }

    if(str[rp] == '}') {
      bracket_level = bracket_level >= 1 ? bracket_level - 1 : 0;
      if(bracket_level == 0) { // token ready
        if(rp - start > 0) tok_list.push_back(str.substr(start, rp-start));
        start = rp = rp + 1;
      } else rp++;
      continue;
    }
    
    // blank
    if(bracket_level == 0 && str[rp] == ' ') { // token ready
      if(start != rp) tok_list.push_back(str.substr(start, rp-start));
      start = rp = rp + 1;
      continue;
    } 
    
    // other
    rp++;
  }

  if(start != rp) tok_list.push_back(str.substr(start, rp-start));

  return tok_list;
}
  
list<string> shell::CMD::tcl_list_break_all(const string& org_str) {
  // use boost tokenizer to parse it
  escaped_list_separator<char> tFunc("", " {}", "");
  tokenizer<escaped_list_separator<char> > tok(org_str, tFunc);
  list<string> rv;
  BOOST_FOREACH(const string& m, tok) rv.push_back(m);
  list<string>::iterator pend = remove(rv.begin(), rv.end(), "");
  rv.erase(pend, rv.end());
  return rv;
}

bool shell::CMD::is_tcl_list(const std::string& str) {
  return str.size() >= 2 && str[0] == '{' && str[str.size()-1] == '}';
}
