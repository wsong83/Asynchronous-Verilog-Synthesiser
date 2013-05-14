/*
 * Copyright (c) 2011-2013 Wei Song <songw@cs.man.ac.uk> 
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
 * report possible partitions
 * 24/04/2013   Wei Song
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
    double dRatio;              // the accept ratio
    bool bFsm;                  // whether to use FSM to help datapath detection
    bool bVerbose;              // verbose output, report all sub modules
    std::string sOutput;        // output file name
    
    Argument() : 
      bHelp(false),
      dRatio(0.8),
      bFsm(false),
      bVerbose(false),
      sOutput("") {}
  };
}


BOOST_FUSION_ADAPT_STRUCT
(
 Argument,
 (bool, bHelp)
 (double, dRatio)
 (bool, bFsm)
 (bool, bVerbose)
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
      using phoenix::push_back;
      using ascii::space;
      using phoenix::at_c;
      using namespace qi::labels;

      args = lit('-') >> 
        ( (lit("help")   >> blanks)                         [at_c<0>(_r1) = true] ||
          (lit("ratio")  >> blanks >> num_double >> blanks) [at_c<1>(_r1) = _1]   ||
          (lit("use_fsm") >> blanks)                        [at_c<2>(_r1) = true] ||
          (lit("verbose") >> blanks)                        [at_c<2>(_r1) = true] ||
          (lit("output") >> blanks >> filename >> blanks)   [at_c<3>(_r1) = _1]
          );
      
      start = *(args(_val));

#ifdef BOOST_SPIRIT_QI_DEBUG
      BOOST_SPIRIT_DEBUG_NODE(args);
      BOOST_SPIRIT_DEBUG_NODE(start);
      BOOST_SPIRIT_DEBUG_NODE(identifier);
      BOOST_SPIRIT_DEBUG_NODE(filename);
#endif
    }
  };
}

const std::string shell::CMD::CMDReportPartition::name = "report_partition"; 
const std::string shell::CMD::CMDReportPartition::description = 
  "report possible partitions of the current design.";

void shell::CMD::CMDReportPartition::help(Env& gEnv) {
  gEnv.stdOs << name << ": " << description << endl;
  gEnv.stdOs << "    report_partition [options]" << endl;
  gEnv.stdOs << "Options:" << endl;
  gEnv.stdOs << "   -help                show this help information." << endl;
  gEnv.stdOs << "   -ratio double        the accept ratio (default 0.80)." << endl;
  gEnv.stdOs << "   -use_fsm             set to use extracted FSM in analyses." << endl;
  gEnv.stdOs << "   -verbose             report all sub-modules." << endl;
  gEnv.stdOs << "   -output file_name    specify an output file otherwise print out." << endl;
}

bool shell::CMD::CMDReportPartition::exec ( const std::string& str, Env * pEnv){

  using std::string;

  Env &gEnv = *pEnv;

  // parse
  string::const_iterator it = str.begin(), end = str.end();
  ArgParser parser;             // argument parser
  Argument arg;                 // argument struct
  bool r = qi::parse(it, end, parser, arg);

  if(!r || it != end) {
    gEnv.stdOs << "Error: Wrong command syntax error! See usage by report_partition -help." << endl;
    gEnv.stdOs << "    report_ports [options]" << endl;
    return false;
  }

  if(arg.bHelp) {        // print help information
    help(gEnv);
    return true;
  }

  // find the target design
  string designName = gEnv.macroDB[MACRO_CURRENT_DESIGN].get_string();;
  shared_ptr<netlist::Module> tarDesign;
  tarDesign = gEnv.find_module(designName);
  if(tarDesign.use_count() == 0) {
    gEnv.stdOs << "Error: Failed to find the target design \"" << designName << "\"." << endl;
    return false;
  }

  // check DFG is ready
  shared_ptr<SDFG::dfgGraph> G;
  if(!tarDesign->DataDFG) {
    gEnv.stdOs << "Error: Data DFG is not extracted for the target design \"" << designName << "\"." << endl;
    gEnv.stdOs << "       Use extract_datapath before report partition." << endl;
    return false;
  } else {
    G = tarDesign->DataDFG;
  }

  if(arg.sOutput.size() > 0) {
    ofstream fhandler;
    fhandler.open(system_complete(arg.sOutput), std::ios_base::out|std::ios_base::trunc);
    tarDesign->cal_partition(arg.dRatio, fhandler, arg.bVerbose);
    fhandler.close();
  } else {
    tarDesign->cal_partition(arg.dRatio, gEnv.stdOs, arg.bVerbose);
  }
  
  return true;
}
