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
 * set the type (data or control) of a SDFG port (usually a top-level port)
 * 02/09/2013   Wei Song
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
    std::string sPort;          // name of the target port
    std::string sType;          // type of target port
    
    Argument() : 
      bHelp(false),
      sPort(""),
      sType("") {}
  };
}

BOOST_FUSION_ADAPT_STRUCT
(
 Argument,
 (bool, bHelp)
 (std::string, sPort)
 (std::string, sType)
 )

namespace {
  typedef std::string::const_iterator SIter;

  struct ArgParser : qi::grammar<SIter, Argument()>, cmd_parse_base<SIter> {
    qi::rule<SIter, void(Argument&)> args;
    qi::rule<SIter, void(Argument&)> port_set_stat;
    qi::rule<SIter, Argument()> start;
    
    ArgParser() : ArgParser::base_type(start) {
      using qi::lit;
      using ascii::char_;
      using ascii::space;
      using phoenix::at_c;
      using namespace qi::labels;

      args = lit('-') >> 
        ( (lit("help")              >> blanks) [at_c<0>(_r1) = true] );

      port_set_stat = 
        (identifier >> blanks) [at_c<1>(_r1) = _1] >>
        (text >> blanks)       [at_c<2>(_r1) = _1];
      
      start = (args(_val)) || (port_set_stat(_val));

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

const std::string shell::CMD::CMDSetPortType::name = "set_port_type"; 
const std::string shell::CMD::CMDSetPortType::description = 
  "set the type (control or data) of an SDFG port port (usually a top-level port).";

void shell::CMD::CMDSetPortType::help(Env& gEnv) {
  gEnv.stdOs << name << ": " << description << endl;
  gEnv.stdOs << "    set_port_type [-help] port_name type" << endl;
  gEnv.stdOs << "    port_name           the name of the SDFG port" << endl;
  gEnv.stdOs << "    type                data || control || unknown" << endl;
  gEnv.stdOs << "   -help                show this help information." << endl;
}

bool shell::CMD::CMDSetPortType::exec ( const std::string& str, Env * pEnv){

  using std::string;

  Env &gEnv = *pEnv;

  // parse
  string::const_iterator it = str.begin(), end = str.end();
  ArgParser parser;             // argument parser
  Argument arg;                 // argument struct
  bool r = qi::parse(it, end, parser, arg);

  if(!r || it != end) {
    gEnv.stdOs << "Error: Wrong command syntax error! See usage by set_port_type -help." << endl;
    gEnv.stdOs << "    set_port_type [-help] port_name type" << endl;
    return false;
  }

  if(arg.bHelp) {        // print help information
    help(gEnv);
    return true;
  }

  // settle the design to be extracted
  string designName;
  shared_ptr<netlist::Module> tarDesign;
  designName = gEnv.macroDB[MACRO_CURRENT_DESIGN].get_string();
  tarDesign = gEnv.find_module(designName);
  if(tarDesign.use_count() == 0) {
    gEnv.stdOs << "Error: Failed to find the target design \"" << designName << "\"." << endl;
    return false;
  }

  // check DFG is ready
  shared_ptr<SDFG::dfgGraph> G;
  if(!tarDesign->DFG) {
    gEnv.stdOs << "Error: DFG is not extracted for the target design \"" << designName << "\"." << endl;
    return false;
  } else {
    G = tarDesign->DFG;
  }

  // find the port 
  std::set<shared_ptr<SDFG::dfgNode> > node = G->search_node(arg.sPort+"_P");
  if(node.empty()) {
    gEnv.stdOs << "Error: the port \"" << arg.sPort << "\" cannot be found." << endl;
    return false;
  }

  // specify the type
  SDFG::dfgNode::datapath_type_t ntype = SDFG::dfgNode::SDFG_DP_NONE;
  if(arg.sType == "data") ntype = SDFG::dfgNode::SDFG_DP_DATA;
  else if(arg.sType == "control") ntype = SDFG::dfgNode::SDFG_DP_CTL;
  else if(arg.sType == "unknown") ntype = SDFG::dfgNode::SDFG_DP_NONE;
  else if(arg.sType == "fsm") ntype = SDFG::dfgNode::SDFG_DP_FSM;
  else {
    gEnv.stdOs << "Error: port type unrecognizable." << endl;
    return false;
  }

  BOOST_FOREACH(shared_ptr<SDFG::dfgNode> n, node)
    n->dp_type = ntype;
  return true;

}
