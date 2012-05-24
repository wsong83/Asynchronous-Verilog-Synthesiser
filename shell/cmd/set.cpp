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
 * set command
 * 18/05/2012   Wei Song
 *
 *
 */

#include "set.h"

using std::vector;
using std::string; 
using std::endl;
using namespace shell;
using namespace shell::CMD;

po::options_description shell::CMD::CMDSet::cmd_opt;
static po::options_description_easy_init dummy_cmd_opt =
  CMDSet::cmd_opt.add_options()
  ("help",     "usage information.")
  ("VarName",  po::value<string>(), "variable name.")
  ("VarValue", po::value<vector<string> >()->composing(), "variable value.")
  ;

po::positional_options_description shell::CMD::CMDSet::cmd_position;
static po::positional_options_description const dummy_position = 
  CMDSet::cmd_position.add("VarName", 1).add("VarValue", -1);


void shell::CMD::CMDSet::help(Env& gEnv) {
  gEnv.stdOs << "set: set the value of a variable." << endl;
  gEnv.stdOs << "    set VarName VarValue" << endl;
  gEnv.stdOs << "Options:" << endl;
  gEnv.stdOs << "   -help                usage information." << endl;
  gEnv.stdOs << endl;
}

bool shell::CMD::CMDSet::exec( Env& gEnv, vector<string>& arg) {
  po::variables_map vm;

  try {
    store(po::command_line_parser(arg).options(cmd_opt).style(cmd_style).positional(cmd_position).run(), vm);
    notify(vm);
  } catch (std::exception& e) {
    gEnv.stdOs << "Error: Wrong command syntax error! See usage by set -help." << endl;
    return false;
  }

  if(vm.count("help") || vm.size() == 0) {        // print help information
    shell::CMD::CMDSet::help(gEnv);
    return true;
  }

  if(vm.count("VarName")) {
    string varName = vm["VarName"].as<string>();
    if(varName.empty() || !cmd_variable_name_checker(varName)) {
      gEnv.stdOs << "Error: A valid variable name must be provided." << endl;
      return false;
    }

    bool rv = true;
    if(vm.count("VarValue")) {
      vector<string> varValue = vm["VarValue"].as<vector<string> >();
      if(gEnv.macroDB[varName].hook.use_count() != 0)  
        rv = (*gEnv.macroDB[varName].hook)(gEnv, gEnv.macroDB[varName], varValue);
      else
        gEnv.macroDB[varName] = varValue;
    } else {
      if(gEnv.macroDB[varName].hook.use_count() != 0)  
        rv = (*gEnv.macroDB[varName].hook)(gEnv, gEnv.macroDB[varName], vector<string>());
      else
        gEnv.macroDB[varName] = CMDVar();
    }

    gEnv.stdOs << varName << " = " << gEnv.macroDB[varName] << endl;
    return rv;
  }
  
  shell::CMD::CMDSet::help(gEnv);
  return false;
}
