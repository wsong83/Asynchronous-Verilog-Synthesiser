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
 * current_design command
 * 24/05/2012   Wei Song
 *
 *
 */

#include "current_design.h"
#include "shell/env.h"
#include "shell/macro_name.h"
#include "shell/cmd_tcl_interp.h"

using std::string;
using std::endl;
using std::vector;

using namespace shell;
using namespace shell::CMD;


po::options_description shell::CMD::CMDCurrentDesign::cmd_opt;
static po::options_description_easy_init dummy_cmd_opt =
  CMDCurrentDesign::cmd_opt.add_options()
  ("help",     "usage information.")
  ("designName",  po::value<string>(), "the taregt design name.")
  ;

po::positional_options_description shell::CMD::CMDCurrentDesign::cmd_position;
static po::positional_options_description const dummy_position = 
  CMDCurrentDesign::cmd_position.add("designName", 1);


void shell::CMD::CMDCurrentDesign::help(Env& gEnv) {
  gEnv.stdOs << "current_design: set or show the current target design." << endl;
  gEnv.stdOs << "    current_design [DesignName]" << endl;
  gEnv.stdOs << "Options:" << endl;
  gEnv.stdOs << "   -help                usage information." << endl;
  gEnv.stdOs << endl;
}

void shell::CMD::CMDCurrentDesign::exec(const Tcl::object& tclObj, Env * pEnv) {
  po::variables_map vm;
  Tcl::interpreter& interp = pEnv->tclInterp->tcli;
  Env &gEnv = *pEnv;
  vector<string> arg = tclObj.get<vector<string> >(interp);

  try {
    store(po::command_line_parser(arg).options(cmd_opt).style(cmd_style).positional(cmd_position).run(), vm);
    notify(vm);
  } catch (std::exception& e) {
    gEnv.stdOs << "Error: Wrong command syntax error! See usage by current_design -help." << endl;
    return;
  }

  if(vm.count("help")) {        // print help information
    shell::CMD::CMDCurrentDesign::help(gEnv);
    return;
  }

  if(vm.count("designName")) {
    string designName = vm["designName"].as<string>();
    interp.set_variable(MACRO_CURRENT_DESIGN, designName);
    gEnv.macroDB[MACRO_CURRENT_DESIGN] = interp.read_variable<string>(MACRO_CURRENT_DESIGN);
  }

  // show the current design
  gEnv.stdOs << "current_design = " << gEnv.macroDB[MACRO_CURRENT_DESIGN] << endl;
}
