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

#ifndef _H_SHELL_ERR_REPORT_
#define _H_SHELL_ERR_REPORT_

#include "location.h"

namespace shell {
  // error type definition
  class ErrorType {
  public:
    enum severe_t {	/* severe level */
      EFatal,
      EError,
      EWarning,
      EInfo
    };
    
    ErrorType(severe_t severe, const std::string& errMsg, int nopara)
      : severe(severe), suppressed(false), errMsg(errMsg), num_of_para(nopara) {}
    
    ErrorType ( const ErrorType& dd)
      : severe(dd.severe), suppressed(dd.suppressed), errMsg(dd.errMsg), num_of_para(dd.num_of_para) {}

    const severe_t severe;      /* severe level */
    bool suppressed;		    /* whether this error message is suppressed */
    const std::string errMsg;	/* error message */
    const int num_of_para;	    /* number of parameters in the error message */

  };

  class ErrReport {		/* a error report function class */
  public:
    ErrReport();
    bool suppress(const std::string&);       /* try to suppress a type of error message */
    void set_output(std::ostream&);		/* set the output stream */
    bool operator() (const location& loc,               /* error location */
                     const std::string& errID,          /* the type of error to report */
                     const std::string& p1 = "",	    /* the first parameter */
                     const std::string& p2 = "",	    /* the second parameter */
                     const std::string& p3 = ""	        /* the third parameter, maximum three */
                     ) ;
    bool operator() (const std::string& errID,          /* the type of error to report */
                     const std::string& p1 = "",	    /* the first parameter */
                     const std::string& p2 = "",	    /* the second parameter */
                     const std::string& p3 = ""	        /* the third parameter, maximum three */
                     ) ;
    bool failure(const std::string& errID) const;
    bool is_failed() const { return fail; }
    void clear() { fail = false; }

  private:
    std::map<std::string, ErrorType> errList;  /* a record for all kinds of errors */
    std::ostream os;                           /* output stream */
    bool fail;                                 /* record the failure of current module, should be cleared for new modules */
  };

}

#endif
