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
 * extract the SDFG of a modulethe data paths from an SDFG
 * 11/03/2013   Wei Song
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
#include <ctime>

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
    bool bQuiet;                // suppress information
    bool bFsm;                  // output FSMs
    bool bCtl;                  // output related control
    bool bRRG;                  // reduce to RRG
    std::string sDesign;        // target design to be written out
    std::string sOutput;        // output file name
    
    Argument() : 
      bHelp(false),
      bQuiet(false),
      bFsm(false),
      bCtl(false),
      bRRG(false),
      sDesign(""),
      sOutput("") {}
  };
}

BOOST_FUSION_ADAPT_STRUCT
(
 Argument,
 (bool, bHelp)
 (bool, bQuiet)
 (bool, bFsm)
 (bool, bCtl)
 (bool, bRRG)
 (std::string, sDesign)
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
        ( (lit("help")              >> blanks) [at_c<0>(_r1) = true]  ||
          (lit("quiet")             >> blanks) [at_c<1>(_r1) = true]  ||
          (lit("with_fsm")          >> blanks) [at_c<2>(_r1) = true]  ||
          (lit("with_ctl")          >> blanks) [at_c<3>(_r1) = true]  ||
          (lit("to_rrg")            >> blanks) [at_c<4>(_r1) = true]  ||
          (lit("output") >> blanks >> filename >> blanks) [at_c<6>(_r1) = _1]
          );
      
      start = 
        *(args(_val))
        >> -(identifier >> blanks) [at_c<5>(_val) = _1] 
        >> *(args(_val))
        ;

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

const std::string shell::CMD::CMDExtractDatapath::name = "extract_datapath"; 
const std::string shell::CMD::CMDExtractDatapath::description = 
  "extract the datapaths from an SDFG.";

void shell::CMD::CMDExtractDatapath::help(Env& gEnv) {
  gEnv.stdOs << name << ": " << description << endl;
  gEnv.stdOs << "    extract_datapath [options] [design_name]" << endl;
  gEnv.stdOs << "    design_name         the design to be extracted (default the current design)" << endl;
  gEnv.stdOs << "Options:" << endl;
  gEnv.stdOs << "   -help                show this help information." << endl;
  gEnv.stdOs << "   -quiet               suppress the optimization information." << endl;
  gEnv.stdOs << "   -with_fsm            show the data path related FSMs." << endl;
  gEnv.stdOs << "   -with_ctl            show the data path related control logic." << endl;
  gEnv.stdOs << "   -to_rrg              reduce to RRG in the output graph." << endl;
  gEnv.stdOs << "   -output file_name    specify the output file name." << endl;
  gEnv.stdOs << "                        (in default is \"design_name.datapath\")" << endl;
}

void shell::CMD::CMDExtractDatapath::exec ( const std::string& str, Env * pEnv){

  using std::string;

  Env &gEnv = *pEnv;

  // parse
  string::const_iterator it = str.begin(), end = str.end();
  ArgParser parser;             // argument parser
  Argument arg;                 // argument struct
  bool r = qi::parse(it, end, parser, arg);

  if(!r || it != end) {
    gEnv.stdOs << "Error: Wrong command syntax error! See usage by extract_datapath -help." << endl;
    gEnv.stdOs << "    extract_datapath [options] [design_name]" << endl;
    return;
  }

  if(arg.bHelp) {        // print help information
    help(gEnv);
    return;
  }

  // settle the design to be extracted
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
    return;
  }

  // specify the output file name
  string outputFileName;
  if(arg.sOutput.empty()) outputFileName = designName + ".datapath";
  else outputFileName = arg.sOutput;

  // open the file
  ofstream fhandler;
  fhandler.open(system_complete(outputFileName), std::ios_base::out|std::ios_base::trunc);

  // make sure DFG is ready
  if(!tarDesign->DFG) {
    CMDUniquify::exec("-quiet", pEnv);
    tarDesign->DFG = tarDesign->extract_sdfg(true);
  }
  std::time_t rawtime;
  struct std::tm * tinfo;
  
  std::time(&rawtime);
  tinfo = std::localtime(&rawtime);

  gEnv.stdOs << "SDFG, I/O " << tarDesign->DFG->get_list_of_nodes(SDFG::dfgNode::SDFG_PORT).size();
  gEnv.stdOs << " Modules " << tarDesign->DFG->size_of_modules(true);
  gEnv.stdOs << " Nodes " << tarDesign->DFG->size_of_nodes(true) << endl;
  gEnv.stdOs << "Time: " << std::asctime(tinfo) << endl;

  shared_ptr<SDFG::dfgGraph> dataDFG = tarDesign->DFG->extract_datapath_new(arg.bFsm, arg.bCtl, arg.bRRG);
  tarDesign->DataDFG = dataDFG;
  tarDesign->assign_dataDFG();

  dataDFG->write(fhandler);

  std::time(&rawtime);
  tinfo = std::localtime(&rawtime);

  gEnv.stdOs << "DatPath, I/O " << dataDFG->get_list_of_nodes(SDFG::dfgNode::SDFG_PORT).size();
  gEnv.stdOs << " Modules " << dataDFG->size_of_modules(true);
  gEnv.stdOs << " Nodes " << dataDFG->size_of_nodes(true) << endl;
  gEnv.stdOs << "Time: " << std::asctime(tinfo) << endl;

  fhandler.close();
  gEnv.stdOs << "write the datapaths to " << outputFileName << endl;
}
