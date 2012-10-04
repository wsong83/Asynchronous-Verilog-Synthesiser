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

#include "shell_top.h"
using namespace shell;
using std::cerr;
using std::ostream;
using std::string;
using std::endl;
using std::pair;


// simplify the error type definitions
#define ERR_DEFINE(eID, eSev, eNum, eMsg)    \
  errList.insert(pair<string, ErrorType>(    \
      eID,                                   \
      ErrorType(ErrorType::eSev, eMsg, eNum) \
    ))

//map<string, ErrorType> shell::ErrReport::errList;
//ostream shell::ErrReport::os(cerr.rdbuf()); // in default send errors to cerr

shell::ErrReport::ErrReport()
  : os(cerr.rdbuf()), fail(false) {
  #include "err_def.h"
}

//bool shell::ErrReport::fail = false;

bool shell::ErrReport::suppress(const string& errID) {

  if(!errList.count(errID)) return false; // error id non-existed

  if(errList.find(errID)->second.severe > ErrorType::EError) { // suppressible
    return errList.find(errID)->second.suppressed = true;
  } else { return false; }
}

void shell::ErrReport::set_output(ostream& new_os) {
  os.rdbuf(new_os.rdbuf());     // redirect the streambuf to the new_os ostream
}

bool shell::ErrReport::operator () (const shell::location& loc, const string& errID,
				     const string& p1, const string& p2, const string& p3) {
  const string rtype[4] = {"Fatal Error: ", "Error: ", "Warning: ", "Information: "}; 

  if(!errList.count(errID)) return false; // error id non-existed

  ErrorType& eT = errList.find(errID)->second;
  if(eT.suppressed) return false;

  switch(eT.num_of_para) {
  case 0: {
    os << loc << ": [" << errID << "] " << rtype[eT.severe]
       << eT.errMsg << endl; 
    break;
  }
  case 1: {
    os << loc << ": [" << errID << "] " << rtype[eT.severe] 
       << format(eT.errMsg) % p1 << endl;
    break;
  }
  case 2: {
    os << loc << ": [" << errID << "] " << rtype[eT.severe] 
       << format(eT.errMsg) % p1 % p2 << endl;
    break;
  }
  case 3: {
    os << loc << ": [" << errID << "] " << rtype[eT.severe] 
       << format(eT.errMsg) % p1 % p2 % p3 << endl;
    break;
  }
  default:
    // should not come here
    assert(0 == "wrong number of error parameters");
  } 

  fail |= (eT.severe <= ErrorType::EError);
  return (eT.severe <= ErrorType::EError);
}

bool shell::ErrReport::operator () (const string& errID,
				     const string& p1, const string& p2, const string& p3) {
  const string rtype[4] = {"Fatal Error: ", "Error: ", "Warning: ", "Information: "}; 

  if(!errList.count(errID)) return false; // error id non-existed

  ErrorType& eT = errList.find(errID)->second;
  if(eT.suppressed) return false;

  switch(eT.num_of_para) {
  case 0: {
    os << "[" << errID << "] " << rtype[eT.severe]
       << eT.errMsg << endl; 
    break;
  }
  case 1: {
    os << "[" << errID << "] " << rtype[eT.severe] 
       << format(eT.errMsg) % p1 << endl;
    break;
  }
  case 2: {
    os << "[" << errID << "] " << rtype[eT.severe] 
       << format(eT.errMsg) % p1 % p2 << endl;
    break;
  }
  case 3: {
    os << "[" << errID << "] " << rtype[eT.severe] 
       << format(eT.errMsg) % p1 % p2 % p3 << endl;
    break;
  }
  default:
    // should not come here
    assert(0 == "wrong number of error parameters");
  } 

  fail |= (eT.severe <= ErrorType::EError);
  return (eT.severe <= ErrorType::EError);
}

bool shell::ErrReport::failure(const string& errID) const {
  assert(errList.count(errID));		// make sure the error id exist
  return (errList.find(errID)->second.severe <= ErrorType::EError);
} 
  
