/*
 * Copyright (c) 2011-2013 Wei Song <songw@cs.man.ac.uk> 
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
 * Common definitions in the SDFG library
 * 17/09/2012   Wei Song
 *
 *
 */

#include "dfg_common.hpp"

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>

using namespace SDFG;
using std::string;

namespace SDFG {

  unsigned long shash(const string& str) {
    unsigned int id_size = 32; // assuming all system has a long larger than 4 bytes
    unsigned long rv = 0;
    for(unsigned int i=0; i<str.size(); i++) {
      unsigned long highbit = rv >> (id_size - 2);
      rv <<= 7;
      rv &= 0xffffffff;
      rv |= str[i];
      rv ^= highbit;
    }
    return rv;
  }
  
  // display the hash id of a tring
  void show_hash(const string& str) {
    std::cout << "hash id of \"" << str << "\":" << boost::format("0x%x") % shash(str) << std::endl;
  }

  string format_double(double number, unsigned int sdec) {
    std::ostringstream sos;
    string format_string = "%1." + boost::lexical_cast<string>(sdec) + "f";
    sos << boost::format(format_string) % number;
    return sos.str();
  }
  
}
