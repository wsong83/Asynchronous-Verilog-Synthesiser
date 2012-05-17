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
 * The variable used in command line environment
 * 17/05/2012   Wei Song
 *
 *
 */

#include <netlist/component.h>
#include "cmd_variable.h"
using boost::shared_ptr;
using std::ostream;
using std::string;
using std::list;

using namespace shell;
using namespace shell::CMD;

ostream& shell::CMD::CMDVar::streamout( ostream& os) const {
  switch(var_type) {
  case vUnkown: return os;
  case vString: os << "\"" << m_str << "\""; return os;
  case vList: {
    list<string>::const_iterator it, end;
    for(it=m_list.begin(), end=m_list.end(); it!=end; it++)
      os << "\"" << *it << "\" ";
    return os;
  }
  case vCollection: {
    list<shared_ptr<netlist::NetComp> >::const_iterator it, end;
    os << "{";
    it=m_collection.begin();
    end=m_collection.end();
    while(it!=end) {
      os << (*it)->get_hier_name();
      it++;
      if(it!=end)
        os << ", ";
    }
    os << "}";
    return os;
  }
  default:
    return os;
  }
}

CMDVar& shell::CMD::CMDVar::operator= (const string& str) {
  var_type = vString;
  m_str = str;
  return *this;
}

CMDVar& shell::CMD::CMDVar::operator= (const list<string>& slist) {
  var_type = vList;
  m_list = slist;
  return *this;
}
