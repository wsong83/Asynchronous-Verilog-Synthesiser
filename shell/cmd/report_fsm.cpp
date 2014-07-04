/*
 * Copyright (c) 2012-2014 Wei Song <songw@cs.man.ac.uk> 
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
 * report the FSMs found in a design
 * 17/10/2012   Wei Song
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
    bool bFast;                 // use the fast algorithm
    bool bSpace;                // space analysis
    bool bForce;                // force to re-extract FSMs
    bool bVerbose;              // show extra information
    bool bNew;                  // use the new algorithm
    std::string sDesign;        // target design
    
    Argument() : 
      bHelp(false),
      bFast(false),
      bSpace(false),
      bForce(false),
      bVerbose(false),
      bNew(false),
      sDesign("") {}
  };
}

BOOST_FUSION_ADAPT_STRUCT
(
 Argument,
 (bool, bHelp)
 (bool, bFast)
 (bool, bSpace)
 (bool, bForce)
 (bool, bVerbose)
 (bool, bNew)
 (std::string, sDesign)
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
        ( (lit("help")    >> blanks)                        [at_c<0>(_r1) = true] ||
          (lit("fast")    >> blanks)                        [at_c<1>(_r1) = true] ||
          (lit("space")   >> blanks)                        [at_c<2>(_r1) = true] ||
          (lit("force")   >> blanks)                        [at_c<3>(_r1) = true] ||
          (lit("verbose") >> blanks)                        [at_c<4>(_r1) = true] ||
          (lit("new")     >> blanks)                        [at_c<5>(_r1) = true] ||
          (lit("design") >> blanks >> identifier >> blanks) [at_c<6>(_r1) = _1]
          );
      
      start = *(args(_val));


#ifdef BOOST_SPIRIT_QI_DEBUG
      BOOST_SPIRIT_DEBUG_NODE(args);
      BOOST_SPIRIT_DEBUG_NODE(start);
      BOOST_SPIRIT_DEBUG_NODE(identifier);
#endif
    }
  };
}

const std::string shell::CMD::CMDReportFSM::name = "report_fsm"; 
const std::string shell::CMD::CMDReportFSM::description = 
  "report the FSMs in a design.";

void shell::CMD::CMDReportFSM::help(Env& gEnv) {
  gEnv.stdOs << name << ": " << description << endl;
  gEnv.stdOs << "    report_fsm [options]" << endl;
  gEnv.stdOs << "Options:" << endl;
  gEnv.stdOs << "   -help                show this help information." << endl;
  gEnv.stdOs << "   -fast                use the fast algorithm." << endl;
  gEnv.stdOs << "   -space               shown state space analyses." << endl;
  gEnv.stdOs << "   -force               whether to force extracting FSMs again." << endl;
  gEnv.stdOs << "   -verbose             show extra information." << endl;
  gEnv.stdOs << "   -design ID           design name if not the current design." << endl;
}

bool shell::CMD::CMDReportFSM::exec ( const std::string& str, Env * pEnv){

  using std::string;

  Env &gEnv = *pEnv;

  // parse
  string::const_iterator it = str.begin(), end = str.end();
  ArgParser parser;             // argument parser
  Argument arg;                 // argument struct
  bool r = qi::parse(it, end, parser, arg);

  if(!r || it != end) {
    gEnv.stdOs << "Error: Wrong command syntax error! See usage by report_fsm -help." << endl;
    gEnv.stdOs << "    report_fsm [options]" << endl;
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

  // make sure DFG and RRG are ready
  if(!tarDesign->DFG) {
    CMDUniquify::exec("-quiet", pEnv);
    tarDesign->DFG = tarDesign->extract_sdfg(true);
  }
  if(!tarDesign->RRG) tarDesign->RRG = tarDesign->DFG->get_RRG();
  
  if(!arg.bNew) {
    // do the FSM extraction
    unsigned int num_of_nodes = 0;
    unsigned int num_of_regs = 0;
    unsigned int num_of_p_fsms = 0;
    std::set<string> fsms = 
      tarDesign->extract_fsms(arg.bVerbose, arg.bForce, tarDesign->RRG, 
                              num_of_nodes, num_of_regs, num_of_p_fsms);
    
    // report
    if(num_of_nodes > 0) {
      std::cout << "\n\nSUMMARY:" << std::endl;
      std::cout << "In this extraction, " << 
        num_of_nodes << " nodes has been scanned, in which " << 
        num_of_regs << " nodes are registers." << std::endl;
      std::cout << "In total " << 
        fsms.size() << " FSM controllers has been found in " <<
        num_of_p_fsms << " potential FSM registers." << std::endl;
    }
    
    std::cout << "The extracted FSMs are listed below:" << std::endl;
    unsigned int index = 0;
    BOOST_FOREACH(const string& fsm_name, fsms) {
      gEnv.stdOs << "[" << ++index << "]  ";
      gEnv.stdOs <<  fsm_name << " ";
      gEnv.stdOs << tarDesign->RRG->get_node(SDFG::divide_signal_name(fsm_name))->get_fsm_type();
      gEnv.stdOs << endl;
      if(arg.bSpace) {
        std::set<shared_ptr<netlist::NetComp> > node_set = tarDesign->RRG->get_node(SDFG::divide_signal_name(fsm_name))->ptr;
        BOOST_FOREACH(shared_ptr<netlist::NetComp> pnode, node_set) {
          if(pnode->get_type() == netlist::NetComp::tSeqBlock)
            // use the local name rather than the full name
            boost::static_pointer_cast<netlist::SeqBlock>(pnode)->
              ssa_analysis(netlist::VIdentifier(tarDesign->RRG->get_node(SDFG::divide_signal_name(fsm_name))->name));
        }
      }
    }
    
    // build the fsm connection graph
    std::set<shared_ptr<SDFG::dfgNode> > fsms_nodes;
    BOOST_FOREACH(const string& fsm_name, fsms) {
      fsms_nodes.insert(tarDesign->RRG->get_node(SDFG::divide_signal_name(fsm_name)));
    }
    shared_ptr<SDFG::dfgGraph> fsm_graph = 
      tarDesign->RRG->build_reg_graph(fsms_nodes);
    
    // specify the output file name
    string outputFileName = designName + ".fsm";
    
    // open the file
    ofstream fhandler;
    fhandler.open(system_complete(outputFileName), std::ios_base::out|std::ios_base::trunc);
    
    fsm_graph->write(fhandler);
    fhandler.close();
    
    gEnv.stdOs << "write the FSM connection graph to " << outputFileName << endl;
    
    //outputFileName = designName + ".fsm.sim";
    //fhandler.open(system_complete(outputFileName), std::ios_base::out|std::ios_base::trunc);
    //fsm_graph->fsm_simplify();
    //fsm_graph->write(fhandler);
    //fhandler.close();
    //gEnv.stdOs << "write the simplified FSM connection graph to " << outputFileName << endl;
    
  } else {
    std::map<string, string> fsms = tarDesign->extract_fsms_new();
    typedef std::pair<const string, string> fsms_type;
    unsigned int i = 0;
    BOOST_FOREACH(fsms_type f, fsms) {
      gEnv.stdOs << "[" << ++i << "]  ";
      gEnv.stdOs <<  f.first << " ";
      gEnv.stdOs << f.second;
      gEnv.stdOs << endl;
    }   
  }

  return true;
}
