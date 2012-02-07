/*
 * Copyright (c) 2011 Wei Song <songw@cs.man.ac.uk> 
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
 * Averilog parser utilities.
 * 01/02/2011   Wei Song
 *
 *
 */

#ifndef _H_AVERILOG_UTIL_
#define _H_AVERILOG_UTIL_

#include <string>
#include <iostream>
#include "common/component.h"

namespace averilog {

  class avID {  // identifier without dimension decalration
  public:
    std::string name;
    avID(char* text, int leng)
      : name(text,leng) {}
  };


}

// copy from averilog.lex.h
#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

#include "averilog.hh"
typedef averilog::av_parser::token token;
typedef averilog::av_parser::semantic_type YYSTYPE;
typedef averilog::location YYLTYPE;

namespace averilog {

  // report errors in scanner
  void error_report(const std::string& err_msg, YYLTYPE * yyloc, std::ostream& os = std::cerr);


}

#endif
