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

using std::pair;


// simplify the error type definitions
#define ERR_DEFINE(eID, eSev, eNum, eMsg)    \
  errList.insert(pair<string, ErrorType>(    \
      eID,                                   \
      ErrorType(ErrorType::eSev, eMsg, eNum) \
    ))

map<string, ErrorType> shell::ErrReport::errList;
ostream shell::ErrReport::os(cerr.rdbuf()); // in default send errors to cerr

shell::ErrReport::ErrReport() {
  // define the error types, may be move this part to a separated header file in the future
  ERR_DEFINE("PARSER-0",     EFatal,   0, "Unkown parser error. Please report to developers.");
  ERR_DEFINE("SYN-ASSIGN-0", EError,   0, "Wrong left-side concatenation.");
  ERR_DEFINE("SYN-ASSIGN-1", EError,   0, "Keyword \"assign\" can only be used in module enviornment.");
  ERR_DEFINE("SYN-FILE-0",   EError,   0, "File terminated in multi-line comments.");
  ERR_DEFINE("SYN-INST-0",   EError,   1, "Instance \"%1%\" is already defined.");
  ERR_DEFINE("SYN-MODULE-0", EError,   1, "Module \"%1%\" is already defined.");
  ERR_DEFINE("SYN-NUM-0",    EError,   1, "Unrecoginised format of number \"%1%\".");
  ERR_DEFINE("SYN-PARA-0",   EError,   1, "Parameter \"%1%\" is declared outside a module enviornment.");
  ERR_DEFINE("SYN-PORT-0",   EError,   2, "Port \"%1%\" is not found in the port declaration list in module \"%2%\".");
  ERR_DEFINE("SYN-PORT-1",   EWarning, 2, "Port \"%1%\" is redefined in the port list of module \"%2%\" and is overlooked.");
  ERR_DEFINE("SYN-VAR-0",    EError,   2, "\"%1% %2%\" is defined outside a block environment.");
  ERR_DEFINE("SYN-VAR-1",    EError,   1, "Lonely \"%1%\" declaration without a signal list");
  ERR_DEFINE("SYN-VAR-2",    EError,   3, "\"%1% %2%\" is already declared in block \"%3%\"");
  ERR_DEFINE("SYN-VAR-3",    EError,   1, "Variable (wire/reg/parameter) \"%1%\" is not found in current block.");
  ERR_DEFINE("SYN-VAR-4",    EWarning, 1, "Static assignment to variable \"%1%\" is overlooked.");
}

bool shell::ErrReport::suppress(const string& errID) {
  map<string, ErrorType>::iterator it, end;
  it = errList.find(errID);
  end = errList.end();
  
  assert(it != end);		// make sure the error id exist

  if(it->second.severe > ErrorType::EError) { // suppressible
    return it->second.suppressed = true;
  } else { return false; }
}

void shell::ErrReport::set_output(ostream& new_os) {
  os.rdbuf(new_os.rdbuf());     // redirect the streambuf to the new_os ostream
}

bool shell::ErrReport::operator () (const averilog::location& loc, const string& errID,
				     const string& p1, const string& p2, const string& p3) const {
  const string rtype[4] = {"Fatal Error: ", "Error: ", "Warning: ", "Information: "}; 
  map<string, ErrorType>::const_iterator it, end;
  it = errList.find(errID);
  end = errList.end();
  
  assert(it != end);		// make sure the error id exist

  const ErrorType& eT = it->second;

  if(eT.suppressed) return true;

  switch(it->second.num_of_para) {
  case 0: {
    os << loc << ": [" << errID << "] " << rtype[eT.severe]
       << eT.errMsg << endl; 
    return true;
  }
  case 1: {
    os << loc << ": [" << errID << "] " << rtype[eT.severe] 
       << format(eT.errMsg) % p1 << endl;
    return true;
  }
  case 2: {
    os << loc << ": [" << errID << "] " << rtype[eT.severe] 
       << format(eT.errMsg) % p1 % p2 << endl;
    return true;
  }
  case 3: {
    os << loc << ": [" << errID << "] " << rtype[eT.severe] 
       << format(eT.errMsg) % p1 % p2 % p3 << endl;
    return true;
  }
  default:
    // should not come here
    assert(0 == "wrong number of error parameters");
  } 
  return false;

}
