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
 * exit command
 * 10/07/2012   Wei Song
 *
 *
 */

// uncomment it when need to debug Spirit.Qi
// #define BOOST_SPIRIT_QI_DEBUG

#include "cmd_define.h"
#include "cmd_parse_base.h"
#include "shell/env.h"
#include "shell/cmd_tcl_interp.h"

using std::endl;
using namespace shell;
using namespace shell::CMD;


namespace {
  namespace qi = boost::spirit::qi;
  namespace phoenix = boost::phoenix;
  namespace ascii = boost::spirit::ascii;

  struct Argument {
    bool bHelp;                           // show help information
    
    Argument() : 
      bHelp(false) {}
  };
}

BOOST_FUSION_ADAPT_STRUCT
(
 Argument,
 (bool, bHelp)
 )

namespace {
  typedef std::string::const_iterator SIter;

  struct ArgParser : qi::grammar<SIter, Argument()>, cmd_parse_base<SIter> {
    qi::rule<SIter, Argument()> start;
    
    ArgParser() : ArgParser::base_type(start) {
      using qi::lit;
      using phoenix::at_c;
      using qi::_val;
      
      start = -(lit("-help") >> blanks [at_c<0>(_val) = true]);

#ifdef BOOST_SPIRIT_QI_DEBUG
      BOOST_SPIRIT_DEBUG_NODE(start);
#endif
    }
  };
}

const std::string shell::CMD::CMDExit::name = "exit/quit"; 
const std::string shell::CMD::CMDExit::description = 
  "quit the system.";


void shell::CMD::CMDExit::help(Env& gEnv) {
  gEnv.stdOs << name << ": " << description << endl;
  gEnv.stdOs << "    exit/quit [-help]" << endl;
}

void shell::CMD::CMDExit::exec(const std::string& str, Env * pEnv) {

  using std::string;

  Env &gEnv = *pEnv;

  // parse
  string::const_iterator it = str.begin(), end = str.end();
  ArgParser parser;             // argument parser
  Argument arg;                 // argument struct
  bool r = qi::parse(it, end, parser, arg);

  if(!r || it != end) {
    gEnv.stdOs << "Error: Wrong command syntax error! See usage by exit -help." << endl;
    gEnv.stdOs << "    exit/quit [-help]" << endl;
    return ;
  }

  if(arg.bHelp) {        // print help information
    help(gEnv);
    return;
  }

  throw Tcl::tcl_error("CMD_TCL_EXIT");
  
}
