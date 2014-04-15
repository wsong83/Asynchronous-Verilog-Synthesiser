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
 * elaborate command
 * 26/05/2012   Wei Song
 *
 *
 */

// uncomment it when need to debug Spirit.Qi
//#define BOOST_SPIRIT_QI_DEBUG

#include "cmd_define.h"
#include "cmd_parse_base.h"
#include <boost/fusion/include/std_pair.hpp>
#include "shell/env.h"
#include "shell/cmd_tcl_interp.h"
#include "shell/macro_name.h"
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

using std::endl;
using boost::shared_ptr;
using namespace shell::CMD;

namespace {
  namespace qi = boost::spirit::qi;
  namespace phoenix = boost::phoenix;

  struct Argument {
    bool bHelp;                 // show help information
    std::string sLibrary;       // target work library
    std::vector<std::pair<std::string, unsigned int> >
    pvPara;                     // parameter list
    std::string sDesign;        // target design name
    
    Argument() : 
      bHelp(false),
      sLibrary(""),
      sDesign("") {}
  };
}

// damn, why must be global name space
typedef std::pair<std::string, unsigned int> AVS_SHELL_CMD_ELABORATE_paraType;

BOOST_FUSION_ADAPT_STRUCT
(
 Argument,
 (bool, bHelp)
 (std::string, sLibrary)
 (std::vector<AVS_SHELL_CMD_ELABORATE_paraType>, pvPara)
 (std::string, sDesign)
 )

namespace {
  typedef std::string::const_iterator SIter;
  struct ArgParser : qi::grammar<SIter, Argument()>, cmd_parse_base<SIter> {
    qi::rule<SIter, std::pair<std::string, unsigned int>()> paraAssign;
    qi::rule<SIter, void(Argument&)> args;
    qi::rule<SIter, Argument()> start;
    
    ArgParser() : ArgParser::base_type(start) {
      using qi::lit;
      using phoenix::at_c;
      using qi::_r1;
      using qi::_1;
      using qi::_val;
      using phoenix::push_back;
      
      paraAssign %= identifier >> -blanks >> (lit("<=")|"=>"|lit('=')) >> -blanks >> qi::uint_;

      args = lit('-') >> 
        (
         ("help"                        >> blanks                   ) [at_c<0>(_r1) = true] ||
         ((lit("library")|"lib"|"work") >> blanks >> text >> blanks ) [at_c<1>(_r1) = _1]   ||
          "parameters" >> blanks >> 
         (
          lit('\"') >> -blanks >>
          -((paraAssign [push_back(at_c<2>(_r1), _1)]) % (blanks || (lit(',')|';') || blanks))
          >> -blanks >> lit('\"') 
          ||
          (paraAssign [push_back(at_c<2>(_r1), _1)]) % (-blanks >> lit(',') >> -blanks)    
          )            >> blanks
         );
      
      start = *args(_val) 
        >> -((identifier >> blanks) [at_c<3>(_val) = _1])
        >> *args(_val);

#ifdef BOOST_SPIRIT_QI_DEBUG
      BOOST_SPIRIT_DEBUG_NODE(paraAssign);
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

const std::string shell::CMD::CMDElaborate::name = "elaborate"; 
const std::string shell::CMD::CMDElaborate::description = 
  "build up a design from a Verilog module.";


void shell::CMD::CMDElaborate::help(Env& gEnv) {
  gEnv.stdOs << name << ": " << description << endl;
  gEnv.stdOs << "    elaborate [options] [DesignName]" << endl;
  gEnv.stdOs << "    DesignName          the top-level design to be elaborated." << endl;
  gEnv.stdOs << "                        (if none, the current design is used)" << endl;
  gEnv.stdOs << "Options:" << endl;
  gEnv.stdOs << "   -help                usage information." << endl;
  gEnv.stdOs << "   -library libName    *extra search library other than work." << endl;
  gEnv.stdOs << "   -work libName       *alias of -library." << endl;
  gEnv.stdOs << "   -parameters list    *module parameter initialization list." << endl;
  gEnv.stdOs << "                        (eg. -parameters \"M=2;N<=3,L=>4 Q=5\")" << endl;
  gEnv.stdOs << "                        ( or -parameters M=2,N<=3,L=>4,Q=5 )" << endl;
}

bool shell::CMD::CMDElaborate::exec (const std::string& str, Env * pEnv){

  using std::string;
  using std::map;
  using std::pair;
  using std::list;

  Env &gEnv = *pEnv;

  // parse
  string::const_iterator it = str.begin(), end = str.end();
  ArgParser parser;             // argument parser
  Argument arg;                 // argument struct
  bool r = qi::parse(it, end, parser, arg);

  if(!r || it != end) {
    gEnv.stdOs << "Error: Wrong command syntax error! See usage by elaborate -help." << endl;
    gEnv.stdOs << "    elaborate [options] [DesignName]" << endl;
    return false;
  }

  if(arg.bHelp) {        // print help information
    help(gEnv);
    return true;
  }

  string tarDesignName = gEnv.macroDB[MACRO_CURRENT_DESIGN];
  shared_ptr<netlist::Library> workLib = gEnv.link_lib[MACRO_DEFAULT_WORK_LIB];
  shared_ptr<netlist::Library> extraLib;
  shared_ptr<netlist::Module>  tarDesign;

  // specify the target design name
  if(!arg.sDesign.empty())    // a target design is defined
    tarDesignName = arg.sDesign;

  // set up the extra work lib
  if(!arg.sLibrary.empty()) {
    if(gEnv.link_lib.count(arg.sLibrary))
      extraLib = gEnv.link_lib[arg.sLibrary];
    else {
      gEnv.stdOs << "Error: Fail to find the extra work library \"" << arg.sLibrary  << "\"."<< endl;
      return false;
    }
  }

  // try to find the target design
  if(extraLib) {
    tarDesign = extraLib->find(tarDesignName);
    if(!tarDesign)              // if failed, try default work lib
      tarDesign = workLib->find(tarDesignName);
    if(!tarDesign) {
      gEnv.stdOs << "Error: Fail to find the target design \"" << tarDesignName << "\" in library \"" << arg.sLibrary  << "\" and the deafult library."<< endl;
      return false;
    }
  } else {
    tarDesign = workLib->find(tarDesignName);
    if(!tarDesign) {
      gEnv.stdOs << "Error: Fail to find the target design \"" << tarDesignName << "\" in the deafult library."<< endl;
      return false;
    }
  }

  //std::cout << "---------- Design before copy ---------" << std::endl;
  //std::cout << *tarDesign ;

  // duplicate the design
  shared_ptr<netlist::Module> mDesign(tarDesign->deep_copy());
  
  //std::cout << "+++++++++ Design after copy ++++++++++ " << std::endl;
  //std::cout << *mDesign ;

  // check and extract parameters
  string pstr;
  if(arg.pvPara.size()) {
    typedef std::pair<string, unsigned int> paraType;
    BOOST_FOREACH(paraType& m, arg.pvPara) {
      shared_ptr<netlist::Variable> mpara = mDesign->db_param.find(m.first);
      if(!mpara) {
        gEnv.stdOs << "Error: Fail to find parameter \"" << m.first << "\" in module \"" << mDesign->name.name << "\"." << endl;
        gEnv.stdOs << "The available parameters are as follows:" << endl;
        gEnv.stdOs << tarDesign->db_param;
        return false;
      } else {
        // try to assign the parameter
        netlist::Number pv(m.second);
        mpara->set_value(pv);
      }
    }
  }
  
  // do the real elaboration
  std::deque<shared_ptr<netlist::Module> >        moduleQueue; // recursive module tree
  // avoid elaborating duplicated maps, and temporarily store the elaborated modules
  map<netlist::MIdentifier, shared_ptr<netlist::Module> >  moduleMap;
    
  // push the top level design into the module fifo
  moduleQueue.push_back(mDesign);

  // get the updated module name
  string newName;
  if(!mDesign->calculate_name(newName))  return false;
  
  // store it in the module map
  moduleMap[newName] = mDesign;

  // do the elaboration
  while(!moduleQueue.empty()) {
    // get a new design
    shared_ptr<netlist::Module> curDgn = moduleQueue.front();
    moduleQueue.pop_front();
    
    //std::cout << "------ Module Elaboration [Before calculate name] ----------" << std::endl;
    //std::cout << *curDgn;

    // get the updated module name
    curDgn->calculate_name(newName);
    
    // report the behaviour to user
    string param_str;
    if(!curDgn->db_param.empty()) {
      param_str = " with parameters \"";
      list<pair<const netlist::VIdentifier, shared_ptr<netlist::Variable> > >::const_iterator it, end;
      it = curDgn->db_param.begin_order();
      end = curDgn->db_param.end_order();
      while(it!=end) {
        param_str += it->second->get_short_string();
        it++;
        if(it!=end) param_str += " ";
      }
      param_str += "\"";
    }
    gEnv.error("ELAB-0", curDgn->name.name, param_str);
    
    // update the design name
    curDgn->set_name(newName);

    //std::cout << "------ Module Elaboration [After calculate name] ----------" << std::endl;
    //std::cout << *curDgn; 

    // elaborate it;
    if(!curDgn->elaborate(moduleQueue, moduleMap)) {
      //gEnv.stdOs << *curDgn;
      return false;
    }
    
    //std::cout << "------ Module Elaboration [After elaboration] ----------" << std::endl;
    //std::cout << *curDgn; 
    
  }
  
  // save all elaborated module to the current work library
  for_each(moduleMap.begin(), moduleMap.end(), [&](pair<const netlist::MIdentifier, shared_ptr<netlist::Module> >& m) {
      workLib->swap(m.second);
    });
  
  //set current design to this design
  gEnv.tclInterp->tcli.set_variable(MACRO_CURRENT_DESIGN, mDesign->name.name);
  return true;
}

