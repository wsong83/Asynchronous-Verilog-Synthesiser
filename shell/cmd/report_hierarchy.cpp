/*
 * Copyright (c) 2014-2014 Wei Song <songw@cs.man.ac.uk> 
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
 * report the hierarchy of current design
 * 24/03/2014   Wei Song
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
using std::string;
using namespace shell::CMD;


namespace {
  namespace qi = boost::spirit::qi;
  namespace phoenix = boost::phoenix;
  namespace ascii = boost::spirit::ascii;

  struct Argument {
    bool bHelp;                 // show help information
    
    Argument() : 
      bHelp(false){}
  };

  void report_hierarchy(shared_ptr<SDFG::dfgGraph>, unsigned int);
  unsigned int port_type();
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
      using ascii::char_;
      using ascii::space;
      using phoenix::at_c;
      using namespace qi::labels;

      args = lit('-') >> 
        ( (lit("help")    >> blanks)                        [at_c<0>(_r1) = true]);
      
      start = *(args(_val));


#ifdef BOOST_SPIRIT_QI_DEBUG
      BOOST_SPIRIT_DEBUG_NODE(args);
      BOOST_SPIRIT_DEBUG_NODE(start);
      BOOST_SPIRIT_DEBUG_NODE(identifier);
#endif
    }
  };
}

const std::string shell::CMD::CMDReportHierarchy::name = "report_hierarchy"; 
const std::string shell::CMD::CMDReportHierarchy::description = 
  "report the hierarchy of the current design.";

void shell::CMD::CMDReportHierarchy::help(Env& gEnv) {
  gEnv.stdOs << name << ": " << description << endl;
  gEnv.stdOs << "    report_hierarchy [-help]" << endl;
  gEnv.stdOs << "Options:" << endl;
  gEnv.stdOs << "   -help                show this help information." << endl;
}

bool shell::CMD::CMDReportHierarchy::exec ( const std::string& str, Env * pEnv){

  using std::string;

  Env &gEnv = *pEnv;

  // parse
  string::const_iterator it = str.begin(), end = str.end();
  ArgParser parser;             // argument parser
  Argument arg;                 // argument struct
  bool r = qi::parse(it, end, parser, arg);

  if(!r || it != end) {
    gEnv.stdOs << "Error: Wrong command syntax error! See usage by report_hierarchy -help." << endl;
    gEnv.stdOs << "    report_hierarchy [-help]" << endl;
    return false;
  }

  if(arg.bHelp) {        // print help information
    help(gEnv);
    return true;
  }

  // find the target design
  string designName;
  shared_ptr<netlist::Module> tarDesign;
  designName = gEnv.macroDB[MACRO_CURRENT_DESIGN].get_string();
  tarDesign = gEnv.find_module(designName);
  if(tarDesign.use_count() == 0) {
    gEnv.stdOs << "Error: Failed to find the target design \"" << designName << "\"." << endl;
    return false;
  }

  // check DFG is ready
  if(!tarDesign->DFG) {
    gEnv.stdOs << "Error: DFG is not extracted for the target design \"" << designName << "\"." << endl;
    gEnv.stdOs << "       Use extract_sdfg before report partition." << endl;
    return false;
  } 

  // check Datapath DFG is ready
  if(!tarDesign->DataDFG) {
    gEnv.stdOs << "Error: Datapaths are not extracted for the target design \"" << designName << "\"." << endl;
    gEnv.stdOs << "       Use extract_datapath before report partition." << endl;
    return false;      
  }

  report_hierarchy(tarDesign->DFG, 0);

  return true;
}

namespace {
  void report_hierarchy(shared_ptr<SDFG::dfgGraph> g, unsigned int indent) {
    // get the data path graph
    shared_ptr<SDFG::dfgGraph> dg = g->pModule->DataDFG;
    if(dg) {
      list<shared_ptr<SDFG::dfgNode> > plist = dg->get_list_of_nodes(SDFG::dfgNode::SDFG_PORT);
      list<shared_ptr<SDFG::dfgNode> > iports;
      list<shared_ptr<SDFG::dfgNode> > oports;
      BOOST_FOREACH(shared_ptr<SDFG::dfgNode> p, plist) {
        if(p->type == SDFG::dfgNode::SDFG_PORT || p->type == SDFG::dfgNode::SDFG_IPORT)
          iports.push_back(p);
        if(p->type == SDFG::dfgNode::SDFG_PORT || p->type == SDFG::dfgNode::SDFG_OPORT)
          oports.push_back(p);
      }
      list<shared_ptr<SDFG::dfgNode> > mlist = g->get_list_of_nodes(SDFG::dfgNode::SDFG_MODULE);

      if(iports.size()) {
        std::cout << string(indent, ' ') << " [I]";
        BOOST_FOREACH(shared_ptr<SDFG::dfgNode> p, iports)
          std::cout << " " << p->get_hier_name();
        std::cout << endl;
      }
      
      if(oports.size()) {
        std::cout << string(indent, ' ') << " [O]";
        BOOST_FOREACH(shared_ptr<SDFG::dfgNode> p, oports)
          std::cout << " " << p->get_hier_name();
        std::cout << endl;
      }

      if(mlist.size()) {
        BOOST_FOREACH(shared_ptr<SDFG::dfgNode> m, mlist) {
          std::cout << string(indent, ' ') << " **" << m->get_hier_name() << endl;
          report_hierarchy(m->child, indent + 2);
        }
      }
    }
  }
}
