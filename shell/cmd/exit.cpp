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
 * exit command
 * 10/07/2012   Wei Song
 *
 *
 */

#include "exit.h"
#include "shell/env.h"
#include "shell/cmd_tcl_interp.h"

using std::vector;
using std::endl;
using std::string;
using namespace shell;
using namespace shell::CMD;

po::options_description shell::CMD::CMDExit::cmd_opt;
static po::options_description_easy_init dummy_cmd_opt =
  CMDExit::cmd_opt.add_options()
  ("help",     "usage information.")
  ;

void shell::CMD::CMDExit::help(Env& gEnv) {
  gEnv.stdOs << "exit/quit: quit the system." << endl;
  gEnv.stdOs << "    echo [-help]" << endl;
  gEnv.stdOs << endl;
}

void shell::CMD::CMDExit::exec(const Tcl::object& tclObj, Env * pEnv) {
  po::variables_map vm;
  Env& gEnv = *pEnv;
  vector<string> arg = tclObj.get<vector<string> >(gEnv.tclInterp->tcli);
  string rv;

  try {
    store(po::command_line_parser(arg).options(cmd_opt).style(cmd_style).run(), vm);
    notify(vm);
  } catch (std::exception& e) {
    gEnv.stdOs << "Error: Wrong command syntax error! See usage by exit -help." << endl;
    return;
  }

  if(vm.count("help")) {        // print help information
    shell::CMD::CMDExit::help(gEnv);
  } else {
    throw Tcl::tcl_error("CMD_TCL_EXIT");
  }
}
