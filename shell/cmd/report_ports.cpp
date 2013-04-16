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
 * report the equation of ports
 * 15/04/2013   Wei Song
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
    std::string sDesign;        // target design
    std::string sOutput;        // output file name
    
    Argument() : 
      bHelp(false),
      sDesign(""),
      sOutput("") {}
  };
}


BOOST_FUSION_ADAPT_STRUCT
(
 Argument,
 (bool, bHelp)
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
        ( (lit("help")   >> blanks)                         [at_c<0>(_r1) = true] ||
          (lit("design") >> blanks >> identifier >> blanks) [at_c<1>(_r1) = _1]   ||
          (lit("output") >> blanks >> filename >> blanks)   [at_c<2>(_r1) = _1]
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

const std::string shell::CMD::CMDReportPorts::name = "report_ports"; 
const std::string shell::CMD::CMDReportPorts::description = 
  "report paths in a DFG.";

void shell::CMD::CMDReportPorts::help(Env& gEnv) {
  gEnv.stdOs << name << ": " << description << endl;
  gEnv.stdOs << "    report_ports -design ID [options]" << endl;
  gEnv.stdOs << "Options:" << endl;
  gEnv.stdOs << "   -help                show this help information." << endl;
  gEnv.stdOs << "   -design ID           design name if not the current design." << endl;
  gEnv.stdOs << "   -output file_name    specify an output file otherwise print out." << endl;
}

bool shell::CMD::CMDReportPorts::exec ( const std::string& str, Env * pEnv){

  using std::string;

  Env &gEnv = *pEnv;

  // parse
  string::const_iterator it = str.begin(), end = str.end();
  ArgParser parser;             // argument parser
  Argument arg;                 // argument struct
  bool r = qi::parse(it, end, parser, arg);

  if(!r || it != end) {
    gEnv.stdOs << "Error: Wrong command syntax error! See usage by report_ports -help." << endl;
    gEnv.stdOs << "    report_ports -design ID [options]" << endl;
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
  
  // report the design with input/output ports, registers, and instances
  gEnv.stdOs << "Module \"" << designName << "\":" << std::endl;
  // report input ports
  gEnv.stdOs << "[Inputs]" << std::endl;
  netlist::DataBase<netlist::VIdentifier, netlist::Port, true>::DBTL::const_iterator pit, pend;
  for(pit = tarDesign->db_port.begin_order(), pend = tarDesign->db_port.end_order(); 
      pit != pend; ++pit) {
    if(pit->second->is_in() || pit->second->is_inout()) {
      gEnv.stdOs << pit->second->name << "; ";
    }
  }
  gEnv.stdOs << std::endl;
  
  // report outputs
  gEnv.stdOs << "\n[Outputs]" << std::endl;
  for(pit = tarDesign->db_port.begin_order(), pend = tarDesign->db_port.end_order(); 
      pit != pend; ++pit) {
    if(pit->second->is_out() || pit->second->is_inout()) {
      gEnv.stdOs << pit->second->name << "; ";
    }
  }
  gEnv.stdOs << std::endl;
  
  // report registers
  gEnv.stdOs << "\n[Registers]" << std::endl;
  netlist::DataBase<netlist::VIdentifier, netlist::Variable, true>::DBTL::const_iterator vit, vend;
  for(vit = tarDesign->db_var.begin_order(), vend = tarDesign->db_var.end_order(); 
      vit != vend; ++vit) {
    if(vit->second->get_vtype() == netlist::Variable::TReg) {
      gEnv.stdOs << vit->second->name << "; ";
    }
  }
  gEnv.stdOs << std::endl;
  
  // report instances
  gEnv.stdOs << "\n[Instances]" << std::endl;
  netlist::DataBase<netlist::IIdentifier, netlist::Instance>::DBTM::const_iterator iit, iend;
  for(iit = tarDesign->db_instance.begin(), iend = tarDesign->db_instance.end(); 
      iit != iend; ++iit) {
    gEnv.stdOs << iit->second->name << "(" << iit->second->mname << "); " << std::endl;;
  }
  gEnv.stdOs << std::endl;
  
  return true;
}
