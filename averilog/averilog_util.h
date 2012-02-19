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
 * Averilog parser utilities.
 * 01/02/2012   Wei Song
 *
 *
 */

#ifndef _H_AVERILOG_UTIL_
#define _H_AVERILOG_UTIL_

#include <cstdio>

namespace averilog {
  class Parser;		/* the parser class that the program will really use */
  class avID;		/* identifier without dimension decalration */
}

#include "av_comp.h"
#include "shell/shell_top.h"
#include "av_token.h"

#include "averilog.hh"
typedef averilog::av_parser::token token;
typedef averilog::av_parser::semantic_type YYSTYPE;
typedef averilog::location YYLTYPE;

namespace averilog {

  // report errors in scanner
  void error_report(const string& err_msg, YYLTYPE * yyloc, ostream& os = std::cerr);

  class Parser {
  public:
    Parser(string, shell::Env& env);	/* constructor with a file name and the design library */
    ~Parser();
    bool parse();		/* the parser for user */
    bool initialize();		/* initialize and check all settings */

  private:
    FILE * sfile;		/* the file handler of the scanner */
    string fname;		/* the name of the file */
    av_parser bison_instance;	/* the bison parser instance */
  }; 

}

#endif
