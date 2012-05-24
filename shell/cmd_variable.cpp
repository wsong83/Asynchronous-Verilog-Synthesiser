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
#include <boost/regex.hpp>
#include "cmd_variable.h"
using boost::regex;
using boost::regex_match;
using boost::regex_search;
using boost::smatch;
using boost::shared_ptr;
using std::ostream;
using std::string;
using std::list;
using std::vector;

#define CMD_VAR_RESOLVE_MAX_LEVEL 128

using namespace shell;
using namespace shell::CMD;

ostream& shell::CMD::CMDVar::streamout( ostream& os) const {
  switch(var_type) {
  case vUnkown: return os;
  case vString: 
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
  m_list.assign(1, str);
  return *this;
}

CMDVar& shell::CMD::CMDVar::operator= (const list<string>& slist) {
  m_list = slist;
  var_type = vList;
  if(slist.size() == 1) {
    var_type = vString;
  } else {
    var_type = vUnkown;
  }
    
  return *this;
}

CMDVar& shell::CMD::CMDVar::operator= (const vector<string>& slist) {
  m_list.assign(slist.begin(), slist.end());
  var_type = vList;
  if(slist.size() == 1) {
    var_type = vString;
  } else {
    var_type = vUnkown;
  }
    
  return *this;
}

string shell::CMD::cmd_variable_resolver(const std::map<std::string, CMDVar>& db, const std::string& istr) {
  string m_string = istr;
  int level = 0;
  regex regex_exp("\\$((\\{\\w[\\w\\d]*\\})|(\\w[\\w\\d]*))");
  smatch mresult;

  //std::cout << "the original string: " << istr << std::endl;

  while(regex_search(m_string, mresult, regex_exp) && level < CMD_VAR_RESOLVE_MAX_LEVEL) {
    string mname;
    if(!mresult[2].str().empty()) {
      mname = mresult[2].str();
      assert(mname.size() >= 3);
      mname.erase(0, 1);
      mname.erase(mname.size()-1, 1);
    } else {
      mname = mresult[3].str();
    }

    //std::cout << mresult <<  " Varname: " << mname << std::endl;

    if(db.count(mname) && db.find(mname)->second.is_string()) {
      m_string = mresult.prefix().str() + db.find(mname)->second.get_string() + mresult.suffix().str();
      //std::cout << "level" << level << " " << m_string << std::endl;
    } else {
      break;
    }

    level++;
  }

  return m_string;
}

bool shell::CMD::cmd_variable_name_checker(const string& vn) {

  regex regex_exp("^\\w[\\w\\d]*$");

  return regex_match(vn, regex_exp);

}
