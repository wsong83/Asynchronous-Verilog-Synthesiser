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
 * Error report mechanisms
 * 18/02/2012   Wei Song
 *
 *
 */

#include <boost/format.hpp>
using boost::format;

#include "shell_env.h"
using namespace shell;

using std::pair;


// simplify the error type definitions
#define ERR_DEFINE(eID, eSev, eMsg, eNum)    \
  errList.insert(pair<string, ErrorType>(    \
      eID,                                   \
      ErrorType(ErrorType::eSev, eMsg, eNum) \
    ))

map<string, ErrorType> shell::err_report::errList;

shell::err_report::err_report()
  : os(cerr) {
  // define the error types, may be move this part to a separated header file in the future
  ERR_DEFINE("Parser-0",   EFatal, "Unkown parser error. Please report to developer.", 0);
  ERR_DEFINE("SYN-NUM-0",  EError, "Unrecoginised format of number %1%.", 1);
  ERR_DEFINE("SYN-PORT-0", EError, "Port %1% is not found in the port declaration list in module %2%.", 2);
}

bool shell::err_report::suppress(const string& errID) {
  map<string, ErrorType>::iterator it, end;
  it = errList.find(errID);
  end = errList.end();
  
  assert(it != end);		// make sure the error id exist

  if(it->second.severe > ErrorType::EError) { // suppressible
    return it->second.suppressed = true;
  } else { return false; }
}

void shell::err_report::set_output(ostream& new_os) {
  os = new_os;
}

bool shell::err_report::operator () (const averilog::location& loc, const string& errID,
				     const string& p1, const string& p2, const string& p3) const {
  const string rtype[4] = {"Information ", "Warning ", "Error ", "Fatal Error "}; 
  map<string, ErrorType>::const_iterator it, end;
  it = errList.find(errID);
  end = errList.end();
  
  assert(it != end);		// make sure the error id exist

  const ErrorType& eT = it->second;

  if(eT.suppressed) return true;

  switch(it->second.num_of_para) {
  case 0: {
    os << loc << ": " << errID << " " << rtype[eT.severe] 
       << eT.errMsg << endl; 
    return true;
  }
  case 1: {
    os << loc << ": " << errID << " " << rtype[eT.severe] 
       << format(eT.errMsg) % p1 << endl;
    return true;
  }
  case 2: {
    os << loc << ": " << errID << " " << rtype[eT.severe] 
       << format(eT.errMsg) % p1 % p2 << endl;
    return true;
  }
  case 3: {
    os << loc << ": " << errID << " " << rtype[eT.severe] 
       << format(eT.errMsg) % p1 % p2 % p3 << endl;
    return true;
  }
  default:
    // should not come here
    assert(0 == "wrong number of error parameters");
  } 
  return false;

}
