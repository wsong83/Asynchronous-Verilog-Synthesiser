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
 * write SDFG command
 * 27/02/2012   Wei Song
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
    std::string sDesign;        // target design to be written out
    std::string sOutput;        // output file name
    bool bSDFG;                 // write out sdfg
    bool bRRG;                  // write out rrg
    bool bDataPath;             // write out data path
    
    Argument() : 
      bHelp(false),
      sDesign(""),
      sOutput(""),
      bSDFG(false),
      bRRG(false),
      bDataPath(false) {}
  };
}

BOOST_FUSION_ADAPT_STRUCT
(
 Argument,
 (bool, bHelp)
 (std::string, sDesign)
 (std::string, sOutput)
 (bool, bSDFG)
 (bool, bRRG)
 (bool, bDataPath)
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
          (lit("output") >> blanks >> filename >> blanks) [at_c<2>(_r1) = _1] ||
          (lit("sdfg")              >> blanks) [at_c<3>(_r1) = true]  ||
          (lit("rrg")               >> blanks) [at_c<4>(_r1) = true]  ||
          (lit("datapath")          >> blanks) [at_c<5>(_r1) = true] 
          );
      
      start = 
        *(args(_val))
        >> -(identifier >> blanks) [at_c<1>(_val) = _1] 
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

const std::string shell::CMD::CMDWriteSDFG::name = "write_sdfg"; 
const std::string shell::CMD::CMDWriteSDFG::description = 
  "write out the SDFG graph (SDFG/RRG/DataPath) of a design to a file.";

void shell::CMD::CMDWriteSDFG::help(Env& gEnv) {
  gEnv.stdOs << name << ": " << description << endl;
  gEnv.stdOs << "    write_sdfg [options] [design_name]" << endl;
  gEnv.stdOs << "    design_name         the design to be written out (default the current" << endl;
  gEnv.stdOs << "                        design)." << endl;
  gEnv.stdOs << "Options:" << endl;
  gEnv.stdOs << "   -help                show this help information." << endl;
  gEnv.stdOs << "   -output file_name    specify the output file name." << endl;
  gEnv.stdOs << "   -sdfg                write out the SDFG graph." << endl;
  gEnv.stdOs << "   -rrg                 write out the RRG graph." << endl;
  gEnv.stdOs << "   -datapath            write out the datapath graph." << endl;
}

bool shell::CMD::CMDWriteSDFG::exec ( const std::string& str, Env * pEnv){

  using std::string;

  Env &gEnv = *pEnv;

  // parse
  string::const_iterator it = str.begin(), end = str.end();
  ArgParser parser;             // argument parser
  Argument arg;                 // argument struct
  bool r = qi::parse(it, end, parser, arg);

  if(!r || it != end) {
    gEnv.stdOs << "Error: Wrong command syntax error! See usage by write_sdfg -help." << endl;
    gEnv.stdOs << "    write_sdfg [options] [design_name]" << endl;
    return false;
  }

  if(arg.bHelp) {        // print help information
    help(gEnv);
    return true;
  }

  // settle the design to be written out
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

  // specify the specific graph to be outputed
  unsigned int m_graph = 0;
  if(arg.bSDFG) m_graph++;
  if(arg.bRRG) m_graph++;
  if(arg.bDataPath) m_graph++;

  if(m_graph == 0) {
    arg.bSDFG = true;
    m_graph = 1;
  }

  if(m_graph > 1) {
    if(!arg.sOutput.empty()) {
      gEnv.stdOs << "Error: Unable to name multiple graphs with the same file name. Please write them separately." << endl;
      return false;
    }
  }

  if(arg.bSDFG) {
    // write out SDFG
    if(!tarDesign->DFG) {
      gEnv.stdOs << "Error: DFG is not generated for this design." << endl;
      return false;
    } else {
      string outputFileName;
      if(arg.sOutput.empty())
        outputFileName = designName + ".sdfg";
      else
        outputFileName = arg.sOutput;
      ofstream fhandler;
      fhandler.open(system_complete(outputFileName), std::ios_base::out|std::ios_base::trunc);
      tarDesign->DFG->write(fhandler);
      fhandler.close();
      gEnv.stdOs << "Write out DFG of \"" << designName << "\" to file " << system_complete(outputFileName) << "." << endl;
    }
  }

  if(arg.bRRG) {
    // write out RRG
    if(!tarDesign->RRG) {
      gEnv.stdOs << "Error: RRG is not generated for this design." << endl;
      return false;
    } else {
      string outputFileName;
      if(arg.sOutput.empty())
        outputFileName = designName + ".rrg";
      else
        outputFileName = arg.sOutput;
      ofstream fhandler;
      fhandler.open(system_complete(outputFileName), std::ios_base::out|std::ios_base::trunc);
      tarDesign->RRG->write(fhandler);
      fhandler.close();
      gEnv.stdOs << "Write out RRG of \"" << designName << "\" to file " << system_complete(outputFileName) << "." << endl;
    }
  }

  if(arg.bDataPath) {
    // write out datapath
    if(!tarDesign->DataDFG) {
      gEnv.stdOs << "Error: Datapath is not generated for this design." << endl;
      return false;
    } else {
      string outputFileName;
      if(arg.sOutput.empty())
        outputFileName = designName + ".datapath";
      else
        outputFileName = arg.sOutput;
      ofstream fhandler;
      fhandler.open(system_complete(outputFileName), std::ios_base::out|std::ios_base::trunc);
      tarDesign->DataDFG->write(fhandler);
      fhandler.close();
      gEnv.stdOs << "Write out datapath of \"" << designName << "\" to file " << system_complete(outputFileName) << "." << endl;
    }
  }

  return true;
}
