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
 * Components needed in av_parser
 * 17/02/2012   Wei Song
 *
 *
 */

#ifndef AV_H_AV_COMPONENT_
#define AV_H_AV_COMPONENT_

#include <string>

namespace averilog {

  class avID {  // identifier without dimension decalration
  public:
    std::string name;
    avID() {}
    avID(char* text, int leng)
      : name(text,leng) {}
  };

  std::ostream& operator<< (std::ostream&, const avID&);

}


#endif
// Local Variables:
// mode: c++
// End:
