/*
 * Copyright (c) 2013-2014 Wei Song <songw@cs.man.ac.uk> 
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
 * analyze the state space of a controller
 * 27/06/2013   Wei Song
 *
 *
 */

// uncomment it when need to debug Spirit.Qi
//#define BOOST_SPIRIT_QI_DEBUG

#include "cmd_define.h"
#include "cmd_parse_base.h"
#include "shell/env.h"
#include "shell/macro_name.h"
#include "sdfg/sdfg.hpp"

#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
using namespace boost::filesystem;

#include <boost/foreach.hpp>

using std::endl;
using boost::shared_ptr;
using std::list;
using namespace shell::CMD;

namespace {
  namespace qi = boost::spirit::qi;
  namespace phoenix = boost::phoenix;
  namespace ascii = boost::spirit::ascii;

  struct Argument {
    bool bHelp;                 // show help information
    std::string sController;    // target controller
    
    Argument() : 
      bHelp(false),
      sController("") {}
  };
}

BOOST_FUSION_ADAPT_STRUCT
(
 Argument,
 (bool, bHelp)
 (std::string, sController)
 )

namespace {
  typedef std::string::const_iterator SIter;

  struct ArgParser : qi::grammar<SIter, Argument()>, cmd_parse_base<SIter> {
    qi::rule<SIter, void(Argument&)> args;
    qi::rule<SIter, Argument()> start;
    
    ArgParser() : ArgParser::base_type(start) {
      using qi::lit;
      using ascii::char_;
      using ascii::space;
      using phoenix::at_c;
      using namespace qi::labels;

      args = lit('-') >> 
        ( (lit("help")    >> blanks)                        [at_c<0>(_r1) = true] );
      
      start = *(args(_val)) >> -((identifier >> blanks) [at_c<1>(_val) = _1]);


#ifdef BOOST_SPIRIT_QI_DEBUG
      BOOST_SPIRIT_DEBUG_NODE(args);
      BOOST_SPIRIT_DEBUG_NODE(start);
      BOOST_SPIRIT_DEBUG_NODE(identifier);
#endif
    }
  };
}

const std::string shell::CMD::CMDStateSpaceAnalysis::name = "state_space_analysis"; 
const std::string shell::CMD::CMDStateSpaceAnalysis::description = 
  "analyse the state space of a controller.";

void shell::CMD::CMDStateSpaceAnalysis::help(Env& gEnv) {
  gEnv.stdOs << name << ": " << description << endl;
  gEnv.stdOs << "    state_space_analysis ID" << endl;
  gEnv.stdOs << "Options:" << endl;
  gEnv.stdOs << "   -help                show this help information." << endl;
  gEnv.stdOs << "   ID                   name of the controller to be analysed." << endl;
}

void shell::CMD::CMDStateSpaceAnalysis::exec ( const std::string& str, Env * pEnv){

  using std::string;

  Env &gEnv = *pEnv;

  // parse
  string::const_iterator it = str.begin(), end = str.end();
  ArgParser parser;             // argument parser
  Argument arg;                 // argument struct
  bool r = qi::parse(it, end, parser, arg);

  if(!r || it != end) {
    gEnv.stdOs << "Error: Wrong command syntax error! See usage by state_space_analysis -help." << endl;
    gEnv.stdOs << "    state_space_analysis ID" << endl;
    return;
  }

  if(arg.bHelp) {        // print help information
    help(gEnv);
    return;
  }

  // get current design
  shared_ptr<netlist::Module> cDesign = gEnv.find_module(gEnv.macroDB[MACRO_CURRENT_DESIGN].get_string());

  // check current design is ready
  if(!cDesign) {
    gEnv.stdOs << "Error: no current design specified yet." << endl;
    return;
  }

  // check RRG is ready
  if(!cDesign->RRG) {
    gEnv.stdOs << "Error: No RRG has been extracted for current design \"" << cDesign->name << "\"." << endl;
    return;
  }

  // find the dfgNode
  shared_ptr<SDFG::dfgNode> pnode = cDesign->RRG->get_node(SDFG::divide_signal_name(arg.sController));
  if(!pnode) {
    gEnv.stdOs << "Error: target controller \"" << arg.sController << "\" not found." << endl;
    return;
  }

  BOOST_FOREACH(shared_ptr<netlist::NetComp> nnode, pnode->ptr) {
    if(nnode->get_type() == netlist::NetComp::tSeqBlock)
      // use the local name rather than the full name
      boost::static_pointer_cast<netlist::SeqBlock>(nnode)->
        ssa_analysis(netlist::VIdentifier(pnode->name));
  }

  return;
}
