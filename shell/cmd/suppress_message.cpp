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
 * suppress some warning messages
 * 23/05/2012   Wei Song
 *
 *
 */

// uncomment it when need to debug Spirit.Qi
//#define BOOST_SPIRIT_QI_DEBUG

#include "cmd_define.h"
#include "cmd_parse_base.h"
#include "shell/env.h"
#include "shell/macro_name.h"
#include <boost/foreach.hpp>

using std::endl;
using namespace shell::CMD;

namespace {
  namespace qi = boost::spirit::qi;
  namespace phoenix = boost::phoenix;

  struct Argument {
    bool bHelp;                         // show help information
    std::vector<std::string> svErrIDs;  // error IDs
    
    Argument() : 
      bHelp(false) {}
  };
}

BOOST_FUSION_ADAPT_STRUCT
(
 Argument,
 (bool, bHelp)
 (std::vector<std::string>, svErrIDs)
 )

namespace {
  typedef std::string::const_iterator SIter;

  struct ArgParser : qi::grammar<SIter, Argument()>, cmd_parse_base<SIter> {
    qi::rule<SIter, void(Argument&)> args;
    qi::rule<SIter, Argument()> start;
    
    ArgParser() : ArgParser::base_type(start) {
      using qi::lit;
      using phoenix::at_c;
      using phoenix::push_back;
      using qi::_r1;
      using qi::_1;
      using qi::_val;

      args = lit('-') >> "help" >> blanks [at_c<0>(_r1) = true];
      
      start = args(_val) || (+(text >> blanks) [push_back(at_c<1>(_val), _1)]);

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

const std::string shell::CMD::CMDSuppressMessage::name = "suppress_message"; 
const std::string shell::CMD::CMDSuppressMessage::description = 
  "suppress the report of some messages.";


void shell::CMD::CMDSuppressMessage::help(Env& gEnv) {
  gEnv.stdOs << name << ": " << description << endl;
  gEnv.stdOs << "    suppress_message [options] error_IDs" << endl;
  gEnv.stdOs << "    error_ID            the ID of the error message to be suppressed." << endl;
  gEnv.stdOs << "Options:" << endl;
  gEnv.stdOs << "   -help                usage information." << endl;
}

bool shell::CMD::CMDSuppressMessage::exec (const std::string& str, Env * pEnv){

  using std::string;

  Env &gEnv = *pEnv;

  // parse
  string::const_iterator it = str.begin(), end = str.end();
  ArgParser parser;             // argument parser
  Argument arg;                 // argument struct
  bool r = qi::parse(it, end, parser, arg);

  if(!r || it != end) {
    gEnv.stdOs << "Error: Wrong command syntax error! See usage by suppress_message -help." << endl;
    gEnv.stdOs << "    suppress_message [options] error_IDs" << endl;
    return false;
  }

  if(arg.bHelp) {        // print help information
    help(gEnv);
    return true;
  }

  BOOST_FOREACH(const string& it, arg.svErrIDs)
    if(!gEnv.error.suppress(it)) 
      gEnv.stdOs << "Error: Error message \"" 
                 << it 
                 << "\" does not exist or cannot be suppressed."<< endl;
  
  return true;
}
