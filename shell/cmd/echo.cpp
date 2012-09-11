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
 * echo command
 * 21/05/2012   Wei Song
 *
 *
 */

// uncomment it when need to debug Spirit.Qi
//#define BOOST_SPIRIT_QI_DEBUG

#include "cmd_define.h"
#include "cmd_parse_base.h"

#include "shell/env.h"
#include "shell/cmd_utility.h"
#include <boost/foreach.hpp>

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
    qi::rule<SIter, void(Argument&)> args;
    qi::rule<SIter, Argument()> start;
    
    ArgParser() : ArgParser::base_type(start) {
      using qi::lit;
      using phoenix::at_c;
      using qi::_r1;
      using qi::_val;
      
      args = lit('-') >> "help" >> blanks [at_c<0>(_r1) = true];
      
      start = -args(_val) >> qi::omit[*ascii::print];

#ifdef BOOST_SPIRIT_QI_DEBUG
      BOOST_SPIRIT_DEBUG_NODE(args);
      BOOST_SPIRIT_DEBUG_NODE(start);
      BOOST_SPIRIT_DEBUG_NODE(text);
      BOOST_SPIRIT_DEBUG_NODE(blanks);
      BOOST_SPIRIT_DEBUG_NODE(identifier);
      BOOST_SPIRIT_DEBUG_NODE(filename);
#endif
    }
  };
}

const std::string shell::CMD::CMDEcho::name = "echo"; 
const std::string shell::CMD::CMDEcho::description = 
  "display a string with variables.";


void shell::CMD::CMDEcho::help(Env& gEnv) {
  gEnv.stdOs << name << ": " << description << endl;
  gEnv.stdOs << "    echo [options] [string]" << endl;
  gEnv.stdOs << "    string              the string to be displayed." << endl;
  gEnv.stdOs << "Options:" << endl;
  gEnv.stdOs << "   -help                show this help informtion." << endl;
}

std::string shell::CMD::CMDEcho::exec(const std::string& str, Env * pEnv) {

  using std::string;

  Env &gEnv = *pEnv;

  // parse
  string::const_iterator it = str.begin(), end = str.end();
  ArgParser parser;             // argument parser
  Argument arg;                 // argument struct
  bool r = qi::parse(it, end, parser, arg);

  if(!r || it != end) {
    gEnv.stdOs << "Error: Wrong command syntax error! See usage by echo -help." << endl;
    gEnv.stdOs << "    echo [options] [string]" << endl;
    return string();
  }

  if(arg.bHelp) {        // print help information
    help(gEnv);
    return string();
  }

  if(is_tcl_list(str)) 
    return str.substr(1, str.size()-2);
  else
    return str;
}
