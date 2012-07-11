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
 * elaborate command
 * 26/05/2012   Wei Song
 *
 *
 */

#include "elaborate.h"
#include "shell/macro_name.h"
#include "shell/env.h"
#include "shell/cmd_tcl_interp.h"
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

using std::string;
using std::endl;
using std::list;
using std::vector;
using boost::shared_ptr;
using std::map;
using std::deque;
using std::pair;
using namespace shell::CMD;

static po::options_description arg_opt("Options");
static po::options_description_easy_init const dummy_arg_opt =
  arg_opt.add_options()
  ("help", "usage information.")
  ("library", po::value<string>(), "look for the deign in another library (other than work).")
  ("work", po::value<string>(), "an alias of -library.")
  ("parameters", po::value<string>(), "module parameter initializations (\"M=>1,N=>5\").")
  ;

static po::options_description design_opt;
static po::options_description_easy_init const dummy_design_opt =
  design_opt.add_options()
  ("design", po::value<string>(), "the target design name.")
  ;

po::options_description shell::CMD::CMDElaborate::cmd_opt;
static po::options_description const dummy_cmd_opt =
  CMDElaborate::cmd_opt.add(arg_opt).add(design_opt);

po::positional_options_description shell::CMD::CMDElaborate::cmd_position;
static po::positional_options_description const dummy_position = 
  CMDElaborate::cmd_position.add("design", 1);

void shell::CMD::CMDElaborate::help(Env& gEnv) {
  gEnv.stdOs << "elaborate: build up a design from a Verilog module." << endl;
  gEnv.stdOs << "    elaborate [options] design_name" << endl;
  gEnv.stdOs << cmd_name_fix(arg_opt) << endl;
}


namespace shell{
  namespace CMD{
    // some help functions
    bool cmd_elaborate_parameter_checker(Env& gEnv, const string& mstr, shared_ptr<netlist::Module>& pmodule) {
      vector<string> fields;
      boost::split(fields, mstr, boost::is_any_of(";,"), boost::token_compress_on);
      BOOST_FOREACH(const string& it, fields) {
        boost::smatch result;
        if(!it.empty() && boost::regex_match(it, result, boost::regex("(\\s*)(\\w+)(\\s*<?=>?\\s*)(\\w+)(\\s*)"))) {
          shared_ptr<netlist::Variable> mpara = pmodule->db_param.find(result[2].str());
          if(mpara.use_count() == 0) {
            gEnv.stdOs << "Error: Fail to find parameter \"" << result[2].str() << "\" in module \"" << pmodule->name.name << "\"." << endl;
            gEnv.stdOs << "The available parameters are as follows:" << endl;
            gEnv.stdOs << pmodule->db_param;
            return false;
          } else {
            // try to assign the parameter
            netlist::Number pv(result[4].str().c_str(), result[4].str().size());
            if(!pv.is_valid()) {
              gEnv.stdOs << "Error: Wrong number format \"" <<  result[4].str() << "\"." << endl;
              return false;
            }
            mpara->set_value(pv);
            //gEnv.stdOs << *mpara;
          }
        } else {
          gEnv.stdOs << "Error: Wrong format \"" << it << "\"." << endl;
          return false;
        }
      }
      return true;
    }
  }
}

bool shell::CMD::CMDElaborate::exec (const Tcl::object& tclObj, Env * pEnv){
  po::variables_map vm;
  Env &gEnv = *pEnv;
  Tcl::interpreter& interp = gEnv.tclInterp->tcli;
  vector<string> arg = tclObj.get<vector<string> >(interp);

  try {
    store(po::command_line_parser(arg).options(cmd_opt).style(cmd_style).positional(cmd_position).run(), vm);
    notify(vm);
  } catch (std::exception& e) {
    gEnv.stdOs << "Error: Wrong command syntax error! See usage by elaborate -help." << endl;
    return false;
  }

  if(vm.count("help")) {        // print help information
    shell::CMD::CMDElaborate::help(gEnv);
    return true;
  }

  if(vm.count("design")) {      // a target design is defined
    // get the design name
    string designName = vm["design"].as<string>();

    // check the library
    string libName = MACRO_DEFAULT_WORK_LIB;
    if(vm.count("library")) { libName = vm["library"].as<string>(); }
    else if(vm.count("work")) { libName = vm["work"].as<string>(); }

    // find the design
    shared_ptr<netlist::Library> workLib;
    shared_ptr<netlist::Module>  tarDesign;
    
    if(gEnv.link_lib.count(libName))
      workLib = gEnv.link_lib[libName];
    else {
      gEnv.stdOs << "Error: Fail to find the work library \"" <<  libName << "\"."<< endl;
      return false;
    }

    tarDesign = workLib->find(designName);
    if(tarDesign.use_count() == 0) {
      gEnv.stdOs << "Error: Fail to find the target design \"" << designName << "\" in library \"" <<  libName << "\"."<< endl;
      return false;
    }

    // duplicate the design
    shared_ptr<netlist::Module> mDesign(tarDesign->deep_copy());
    
    // check and extract parameters
    string pstr;
    if(vm.count("parameters")) {
      pstr = vm["parameters"].as<string>();
      if(!cmd_elaborate_parameter_checker(gEnv, pstr, mDesign)) {
        gEnv.stdOs << "Error: Fail to resolve the parameter assignments \"" << pstr << "\" of the target design \"" << designName << "\"."<< endl;
        return false;
      }
    }

    // do the real elaboration
    deque<shared_ptr<netlist::Module> >        moduleQueue; // recursive module tree
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
      
      // get the updated module name
      curDgn->calculate_name(newName);

      // report the behaviour to user
      gEnv.stdOs << "elaborating module \"" << curDgn->name.name << "\"";
      
      if(!curDgn->db_param.empty()) {
        gEnv.stdOs << " with parameters \"";
        list<pair<netlist::VIdentifier, shared_ptr<netlist::Variable> > >::const_iterator it, end;
        it = curDgn->db_param.begin_order();
        end = curDgn->db_param.end_order();
        while(it!=end) {
          gEnv.stdOs << it->second->get_short_string();
          it++;
          if(it!=end) gEnv.stdOs << " ";
        }
        gEnv.stdOs << "\"";
      }
      gEnv.stdOs << "." << endl;

      // update the design name
      curDgn->set_name(newName);

      // elaborate it;
      if(!curDgn->elaborate(moduleQueue, moduleMap)) {
        //gEnv.stdOs << *curDgn;
        return false;
      }

    }
    
    // save all elaborated module to the current work library
    for_each(moduleMap.begin(), moduleMap.end(), [&workLib](pair<const netlist::MIdentifier, shared_ptr<netlist::Module> >& m) {
        workLib->swap(m.second);
      });

    //set current design to this design
    interp.set_variable(MACRO_CURRENT_DESIGN, mDesign->name.name);
    gEnv.macroDB[MACRO_CURRENT_DESIGN] = interp.read_variable<string>(MACRO_CURRENT_DESIGN);
    return true;
  }

  shell::CMD::CMDElaborate::help(gEnv);    
  return true;
}
