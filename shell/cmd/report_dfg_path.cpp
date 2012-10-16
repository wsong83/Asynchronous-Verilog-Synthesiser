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
 * report the paths in a DFG
 * 10/10/2012   Wei Song
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
    bool bFast;                 // fast version
    std::string sSource;        // source node
    std::string sTarget;        // target node
    std::string sDesign;        // target design
    int nMax;                   // the maximal number of paths to be reported
    std::string sOutput;        // output file name
    
    Argument() : 
      bHelp(false),
      bFast(false),
      sSource(""),
      sTarget(""),
      sDesign(""),
      nMax(-1),
      sOutput("") {}
  };
}

BOOST_FUSION_ADAPT_STRUCT
(
 Argument,
 (bool, bHelp)
 (bool, bFast)
 (std::string, sSource)
 (std::string, sTarget)
 (std::string, sDesign)
 (int, nMax)
 (std::string, sOutput)
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
        ( (lit("help")   >> blanks)                         [at_c<0>(_r1) = true] ||
          (lit("fast")   >> blanks)                         [at_c<1>(_r1) = true] ||
          (lit("from")   >> blanks >> identifier >> blanks) [at_c<2>(_r1) = _1]   ||
          (lit("to")     >> blanks >> identifier >> blanks) [at_c<3>(_r1) = _1]   ||
          (lit("design") >> blanks >> identifier >> blanks) [at_c<4>(_r1) = _1]   ||
          (lit("max")    >> blanks >> qi::uint_  >> blanks) [at_c<5>(_r1) = _1]   ||
          (lit("output") >> blanks >> filename >> blanks)   [at_c<6>(_r1) = _1]
          );
      
      start = +(args(_val));


#ifdef BOOST_SPIRIT_QI_DEBUG
      BOOST_SPIRIT_DEBUG_NODE(args);
      BOOST_SPIRIT_DEBUG_NODE(start);
      BOOST_SPIRIT_DEBUG_NODE(identifier);
      BOOST_SPIRIT_DEBUG_NODE(filename);
#endif
    }
  };
}

const std::string shell::CMD::CMDReportDFGPath::name = "report_dfg_path"; 
const std::string shell::CMD::CMDReportDFGPath::description = 
  "report paths in a DFG.";

void shell::CMD::CMDReportDFGPath::help(Env& gEnv) {
  gEnv.stdOs << name << ": " << description << endl;
  gEnv.stdOs << "    report_dfg_path -source ID [options]" << endl;
  gEnv.stdOs << "Options:" << endl;
  gEnv.stdOs << "   -help                show this help information." << endl;
  gEnv.stdOs << "   -fast                use the fast algorithm which omit intermediate nodes." << endl;
  gEnv.stdOs << "   -from ID             path starting points (FF/input)." << endl;
  gEnv.stdOs << "   -to ID               path ending points (FF/output)." << endl;
  gEnv.stdOs << "   -design ID           design name if not the current design." << endl;
  gEnv.stdOs << "   -max N               the maximal number of paths to be reported." << endl;
  gEnv.stdOs << "   -output file_name    specify an output file." << endl;
}

bool shell::CMD::CMDReportDFGPath::exec ( const std::string& str, Env * pEnv){

  using std::string;

  Env &gEnv = *pEnv;

  // parse
  string::const_iterator it = str.begin(), end = str.end();
  ArgParser parser;             // argument parser
  Argument arg;                 // argument struct
  bool r = qi::parse(it, end, parser, arg);

  if(!r || it != end) {
    gEnv.stdOs << "Error: Wrong command syntax error! See usage by report_dfg_path -help." << endl;
    gEnv.stdOs << "    report_dfg_path -source ID [options]" << endl;
    return false;
  }

  if(arg.bHelp) {        // print help information
    help(gEnv);
    return true;
  }

  // find the target design
  string designName;
  shared_ptr<netlist::Module> tarDesign;
  if(arg.sDesign.empty()) {
    designName = gEnv.macroDB[MACRO_CURRENT_DESIGN].get_string();
  } else {
    designName = arg.sDesign;
  }
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
  
  // check start and end points
  if(arg.sSource.empty() && arg.sTarget.empty()) {
    gEnv.stdOs << "Error: At least one of the starting point or the ending point must be specified." << endl;
  }
  
  shared_ptr<SDFG::dfgNode> src;
  shared_ptr<SDFG::dfgNode> tar;

  if(!arg.sSource.empty()) {
    src = G->search_node(arg.sSource);
    if(!src) {
      gEnv.stdOs << "Error: Fail to find the specified starting point \"" << arg.sSource << "\"." << endl;
    } else if(
              !(src->type & (SDFG::dfgNode::SDFG_FF|SDFG::dfgNode::SDFG_LATCH)) &&
              !((src->type & SDFG::dfgNode::SDFG_PORT) && (src->type != SDFG::dfgNode::SDFG_OPORT) && 
                (!src->pg->father))
              ) {
      gEnv.stdOs << "Error: The starting point of a path must be a FF/Latch or top-level input port." << endl;
      return false;
    }
  }

  if(!arg.sTarget.empty()) {
    tar = G->search_node(arg.sTarget);
    if(!tar) {
      gEnv.stdOs << "Error: Fail to find the specified ending point \"" << arg.sTarget << "\"." << endl;
    } else if(
              !(tar->type & (SDFG::dfgNode::SDFG_FF|SDFG::dfgNode::SDFG_LATCH)) &&
              !((tar->type & SDFG::dfgNode::SDFG_PORT) && (tar->type != SDFG::dfgNode::SDFG_IPORT) && 
                (!tar->pg->father))
              ) {
      gEnv.stdOs << "Error: The ending point of a path must be a FF/Latch or top-level output port." << endl;
      return false;
    }
  }

  list<shared_ptr<SDFG::dfgPath> > plist;
  std::set<shared_ptr<SDFG::dfgNode> > targets;
  if(tar)
    targets.insert(tar);
  std::set<shared_ptr<SDFG::dfgNode> > sources;
  if(src)
    sources.insert(src);

  if(!sources.empty()) {
    BOOST_FOREACH(shared_ptr<SDFG::dfgNode> s, sources) {
      list<shared_ptr<SDFG::dfgPath> > mp;
      if(arg.bFast)
        mp = s->get_out_paths_f((arg.nMax < 0 ? 10 : arg.nMax)-plist.size(), targets);  
      else {
        mp = s->get_out_paths((arg.nMax < 0 ? 10 : arg.nMax)-plist.size(), targets);  
      }
      plist.insert(plist.end(), mp.begin(), mp.end());
      if(plist.size() >= (arg.nMax < 0 ? 10 : arg.nMax)) 
        break;
    }
  } else {
    BOOST_FOREACH(shared_ptr<SDFG::dfgNode> t, targets) {
      list<shared_ptr<SDFG::dfgPath> > mp;
      if(arg.bFast)
        mp = t->get_in_paths_f((arg.nMax < 0 ? 10 : arg.nMax)-plist.size(), sources);  
      else {
        mp = t->get_in_paths((arg.nMax < 0 ? 10 : arg.nMax)-plist.size(), sources);  
      }
      plist.insert(plist.end(), mp.begin(), mp.end());
      if(plist.size() >= (arg.nMax < 0 ? 10 : arg.nMax)) 
        break;
    }
  }

  int index = 0;
  BOOST_FOREACH(shared_ptr<SDFG::dfgPath> p, plist) {
    if(!tar || (tar && p->tar == tar))
      gEnv.stdOs << "[" << ++index << "]  " << *p;
  }

  return true;
}
