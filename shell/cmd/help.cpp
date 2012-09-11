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
 * argument definitions for help command
 * 14/05/2012   Wei Song
 *
 *
 */

// uncomment it when need to debug Spirit.Qi
//#define BOOST_SPIRIT_QI_DEBUG

#include "cmd_define.h"
#include "cmd_parse_base.h"
#include "shell/env.h"

using namespace shell;
using namespace shell::CMD;
using std::endl;

// the command list
std::map<std::string, shell::CMD::CMDHelp::cmd_record > shell::CMD::CMDHelp::cmdDB;


namespace {
  namespace qi = boost::spirit::qi;
  namespace phoenix = boost::phoenix;
  namespace ascii = boost::spirit::ascii;

  struct Argument {
    bool bHelp;                           // show help information
    std::string sCMDName;                 // the target command to show help info
    
    Argument() : 
      bHelp(false),
      sCMDName("") {}
  };
}

BOOST_FUSION_ADAPT_STRUCT
(
 Argument,
 (bool, bHelp)
 (std::string, sCMDName)
 )

namespace {
  typedef std::string::const_iterator SIter;

  struct ArgParser : qi::grammar<SIter, Argument()>, cmd_parse_base<SIter> {
    qi::rule<SIter, void(Argument&)> args;
    qi::rule<SIter, Argument()> start;
    
    ArgParser() : ArgParser::base_type(start) {
      using qi::lit;
      using ascii::char_;
      using phoenix::at_c;
      using phoenix::push_back;
      using namespace qi::labels;
      
      args = (lit("-help") >> blanks) [at_c<0>(_r1) = true];
      
      start = -args(_val) >> -(text [at_c<1>(_val) = _1] >> blanks);

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

const std::string shell::CMD::CMDHelp::name = "help"; 
const std::string shell::CMD::CMDHelp::description = 
  "show and list the usage of commands.";


void shell::CMD::CMDHelp::help(Env& gEnv) {
  gEnv.stdOs << name << ": " << description << endl;
  gEnv.stdOs << "    help [-help] [command_name]" << endl;
  gEnv.stdOs << "    command_name        the one to show the detailed help info." << endl;
  gEnv.stdOs << "                        ( if none, show the list of commands)" << endl;
  gEnv.stdOs << "Options:" << endl;
  gEnv.stdOs << "   -help                show this help information." << endl;
}

void shell::CMD::CMDHelp::exec (const std::string& str, Env * pEnv){

  using std::string;
  using std::map;
  using std::pair;

  Env &gEnv = *pEnv;

  // parse
  string::const_iterator it = str.begin(), end = str.end();
  ArgParser parser;             // argument parser
  Argument arg;                 // argument struct
  bool r = qi::parse(it, end, parser, arg);

  if(!r || it != end) {
    gEnv.stdOs << "Error: Wrong command syntax error! See usage by help -help." << endl;
    gEnv.stdOs << "    help [-help] [command_name]" << endl;
    return;
  }

  if(arg.bHelp) {        // print help information
    help(gEnv);
    return;
  }

  // whether a target command is provided
  if(arg.sCMDName.empty()) {    // no
    map<string, cmd_record>::iterator it, end;
    for(it=cmdDB.begin(), end=cmdDB.end(); it!=end; it++)
      gEnv.stdOs << it->first << 
        string(it->first.size() < 24 ? 24 - it->first.size() : 1, ' ')
                 << it->second.first << endl;
    gEnv.stdOs << endl;
  } else {
    if(cmdDB.count(arg.sCMDName)) { // should be a AVS command
      cmdDB[arg.sCMDName].second(gEnv);
    } else {
      gEnv.stdOs << "Error: Wrong command name: \"" << arg.sCMDName << "\"."<< endl;
    }    
  }
}
