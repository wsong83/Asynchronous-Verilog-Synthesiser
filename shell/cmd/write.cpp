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
 * write command
 * 25/06/2012   Wei Song
 *
 *
 */

#include "cmd_define.h"
#include "cmd_parse_base.h"
#include "shell/env.h"
#include "shell/macro_name.h"

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
    bool bHierarchy;            // write out hierarchical design
    std::string sDesign;        // target design to be written out
    std::string sOutput;        // output file name
    
    Argument() : 
      bHelp(false),
      bHierarchy(false),
      sDesign(""),
      sOutput("") {}
  };
}

BOOST_FUSION_ADAPT_STRUCT
(
 Argument,
 (bool, bHelp)
 (bool, bHierarchy)
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
          (lit("hierarchy")         >> blanks) [at_c<1>(_r1) = true]  ||
          (lit("output") >> blanks >> text >> blanks) [at_c<3>(_r1) = _1]
          );
      
      start = 
        *(args(_val))
        >> -(text >> blanks) [at_c<2>(_val) = _1] 
        >> *(args(_val))
        ;
    }
  };
}

const std::string shell::CMD::CMDWrite::name = "write"; 
const std::string shell::CMD::CMDWrite::description = 
  "write out a design to a file.";

void shell::CMD::CMDWrite::help(Env& gEnv) {
  gEnv.stdOs << name << ": " << description << endl;
  gEnv.stdOs << "    write [options] [design_name]" << endl;
  gEnv.stdOs << "    design_name         the design to be written out (default the current" << endl;
  gEnv.stdOs << "                        design)." << endl;
  gEnv.stdOs << "Options:" << endl;
  gEnv.stdOs << "   -help                show this help information." << endl;
  gEnv.stdOs << "   -hierarchy           write out the whole design hiarchy." << endl;
  gEnv.stdOs << "                        (in default only the design module is written out)" << endl;
  gEnv.stdOs << "   -output file_name    specify the output file name." << endl;
  gEnv.stdOs << "                        (in default is \"design_name.v\")" << endl;
}

bool shell::CMD::CMDWrite::exec ( const std::string& str, Env * pEnv){

  using std::string;

  Env &gEnv = *pEnv;

  // parse
  string::const_iterator it = str.begin(), end = str.end();
  ArgParser parser;             // argument parser
  Argument arg;                 // argument struct
  bool r = qi::parse(it, end, parser, arg);

  if(!r || it != end) {
    gEnv.stdOs << "Error: Wrong command syntax error! See usage by write -help." << endl;
    gEnv.stdOs << "    write [options] [design_name]" << endl;
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

  // specify the output file name
  string outputFileName;
  if(arg.sOutput.empty()) outputFileName = designName + ".v";
  else outputFileName = arg.sOutput;

  // open the file
  ofstream fhandler;
  fhandler.open(system_complete(outputFileName), std::ios_base::out|std::ios_base::trunc);

  // show an instruction
  gEnv.stdOs << "Write out design \"" << designName << "\" to file " << system_complete(outputFileName) << "." << endl;

  // do the write out
  if(arg.bHierarchy) {         // hierarchical
    // prepared the module map and queue
    list<shared_ptr<netlist::Module> > moduleQueue; // recursive module tree
    std::set<netlist::MIdentifier>  moduleMap;
    // add the top module to the queue
    moduleQueue.push_back(tarDesign);
    moduleMap.insert(tarDesign->name);
    // find all modules
    tarDesign->get_hier(moduleQueue, moduleMap);
    // do the write out
    BOOST_FOREACH(shared_ptr<netlist::Module>& m, moduleQueue) {
      fhandler << *m << std::endl;
    }
  } else {
    fhandler << *tarDesign << std::endl;
  } 

  fhandler.close();

  return true;
}
