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

#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include "elaborate.h"
#include "shell/macro_name.h"

using std::string;
using std::endl;
using std::list;
using std::vector;
using boost::shared_ptr;
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
      vector<string>::iterator it, end;
      for(it=fields.begin(), end=fields.end(); it!=end; it++) {
        boost::smatch result;
        if(!it->empty() && boost::regex_match(*it, result, boost::regex("(\\s*)(\\w+)(\\s*<?=>?\\s*)(\\w+)(\\s*)"))) {
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
          gEnv.stdOs << "Error: Wrong format \"" << *it << "\"." << endl;
          return false;
        }
      }
      return true;
    }
  }
}

bool shell::CMD::CMDElaborate::exec ( Env& gEnv, vector<string>& arg){
  
  po::variables_map vm;

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
    
    if(gEnv.link_lib.find(libName) != gEnv.link_lib.end())
      workLib = gEnv.link_lib.find(libName)->second;
    else {
      gEnv.stdOs << "Error: Fail to find the work library \"" <<  libName << "\"."<< endl;
      return false;
    }

    tarDesign = workLib->find(designName);
    if(tarDesign.use_count() == 0) {
      gEnv.stdOs << "Error: Fail to find the target design \"" << designName << "\" in library \"" <<  libName << "\"."<< endl;
      return false;
    }

    // if the design has parameters, duplicate the design
    if(!tarDesign->db_param.empty())
      tarDesign.reset(tarDesign->deep_copy());

    // check and extract parameters
    string pstr;
    if(vm.count("parameters")) {
      pstr = vm["parameters"].as<string>();
      if(!cmd_elaborate_parameter_checker(gEnv, pstr, tarDesign)) {
        gEnv.stdOs << "Error: Fail to resolve the parameter assignments \"" << pstr << "\" of the target design \"" << designName << "\"."<< endl;
        return false;
      }
    }
    return true;
  }

  shell::CMD::CMDElaborate::help(gEnv);    
  return true;
}
