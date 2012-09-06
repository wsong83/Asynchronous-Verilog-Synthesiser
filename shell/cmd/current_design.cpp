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
 * current_design command
 * 24/05/2012   Wei Song
 *
 *
 */

#include "cmd_define.h"
#include "cmd_parse_base.h"
#include "shell/env.h"
#include "shell/cmd_tcl_interp.h"
#include "shell/macro_name.h"

using std::endl;

using namespace shell;
using namespace shell::CMD;

namespace {
  namespace qi = boost::spirit::qi;
  namespace phoenix = boost::phoenix;

  struct Argument {
    bool bHelp;                 // show help information
    std::string sDesign;        // target design name
    
    Argument() : 
      bHelp(false),
      sDesign("") {}
  };
}

BOOST_FUSION_ADAPT_STRUCT
(
 Argument,
 (bool, bHelp)
 (std::string, sDesign)
 )

namespace {
  typedef std::string::const_iterator SIter;

  struct ArgParser : qi::grammar<SIter, Argument()>, cmd_parse_base<SIter> {
    qi::rule<SIter, void(Argument&)> args;
    qi::rule<SIter, Argument()> start;
    
    ArgParser() : ArgParser::base_type(start) {
      args = qi::lit('-') >> qi::lit("help") >> blanks [phoenix::at_c<0>(qi::_r1) = true];
      
      start = (args(qi::_val) >> blanks)
        || ((text >> blanks) [phoenix::at_c<1>(qi::_val) = qi::_1])
        ;
    }
  };
}

const std::string shell::CMD::CMDCurrentDesign::name = "current_design"; 
const std::string shell::CMD::CMDCurrentDesign::description = 
  "set or show the current target design.";


void shell::CMD::CMDCurrentDesign::help(Env& gEnv) {
  gEnv.stdOs << name << ": " << description << endl;
  gEnv.stdOs << "    current_design [DesignName]" << endl;
  gEnv.stdOs << "    DesignName          the taregt design name." << endl;
  gEnv.stdOs << "                        (if none, the current design is shown)" << endl;
  gEnv.stdOs << "Options:" << endl;
  gEnv.stdOs << "   -help                usage information." << endl;
}

void shell::CMD::CMDCurrentDesign::exec(const std::string& str, Env * pEnv) {

  using std::string;

  Env &gEnv = *pEnv;

  // parse
  string::const_iterator it = str.begin(), end = str.end();
  ArgParser parser;             // argument parser
  Argument arg;                 // argument struct
  bool r = qi::parse(it, end, parser, arg);

  if(!r || it != end) {
    gEnv.stdOs << "Error: Wrong command syntax error! See usage by current_design -help." << endl;
    gEnv.stdOs << "    current_design [DesignName]" << endl;
    return;
  }

  if(arg.bHelp) {        // print help information
    help(gEnv);
    return;
  }

  if(!arg.sDesign.empty()) {
    gEnv.tclInterp->tcli.set_variable(MACRO_CURRENT_DESIGN, arg.sDesign);
  }

  // show the current design
  gEnv.stdOs << "current_design = " << gEnv.macroDB[MACRO_CURRENT_DESIGN] << endl;
}
