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

#include <string>
#include <iostream>
#include <cstdio>
#include <boost/shared_ptr.hpp>

namespace averilog {
  class Parser;		/* the parser class that the program will really use */
  class avID;		/* identifier without dimension decalration */
}

#include "common/component.h"

namespace averilog {

  class avID {  // identifier without dimension decalration
  public:
    std::string name;
    avID(char* text, int leng)
      : name(text,leng) {}
  };

  std::ostream& operator<< (std::ostream&, const avID&);

}

// copy from averilog.lex.h
#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

namespace averilog {

#define YYSTYPE av_token_type
  
  struct av_token_type {
    boost::shared_ptr<netlist::BIdentifier>   tBlockName;
    boost::shared_ptr<netlist::Expression>    tExp;		
    boost::shared_ptr<netlist::FIdentifier>   tFuncName;
    boost::shared_ptr<avID>                   tID;      
    boost::shared_ptr<netlist::IIdentifier>   tInstName;
    boost::shared_ptr<std::list<boost::shared_ptr<netlist::VIdentifier> > > tListVar;
    boost::shared_ptr<netlist::MIdentifier>   tModuleName;
    boost::shared_ptr<netlist::Number>        tNumber;    
    boost::shared_ptr<netlist::PaIdentifier>  tParaName;
    boost::shared_ptr<netlist::PoIdentifier>  tPortName;
    boost::shared_ptr<netlist::Range>         tRange;	
    boost::shared_ptr<netlist::VIdentifier>   tVarName;	
  };
}

#include "averilog.hh"
typedef averilog::av_parser::token token;
typedef averilog::av_parser::semantic_type YYSTYPE;
typedef averilog::location YYLTYPE;

namespace averilog {

  // report errors in scanner
  void error_report(const std::string& err_msg, YYLTYPE * yyloc, std::ostream& os = std::cerr);

  class Parser {
  public:
    Parser(std::string, netlist::Library& lib);	/* constructor with a file name and the design library */
    ~Parser();
    bool parse();		/* the parser for user */
    bool initialize();		/* initialize and check all settings */

  private:
    FILE * sfile;		/* the file handler of the scanner */
    std::string fname;		/* the name of the file */
    av_parser bison_instance;	/* the bison parser instance */
  }; 

}

#endif
