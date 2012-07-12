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
 * shell command
 * 12/07/2012   Wei Song
 *
 *
 */

#include <string>
#include "shell.h"
#include "shell/env.h"
#include "shell/cmd_tcl_interp.h"
#include "shell/cmd_utility.h"
#include <boost/foreach.hpp>

using std::vector;
using std::endl;
using std::string;
using namespace shell;
using namespace shell::CMD;

po::options_description shell::CMD::CMDShell::cmd_opt;
static po::options_description_easy_init dummy_cmd_opt =
  CMDShell::cmd_opt.add_options()
  ("help",     "usage information.")
  ("command",  po::value<vector<string> >()->composing(), "shell command.")
  ;

po::positional_options_description shell::CMD::CMDShell::cmd_position;
static po::positional_options_description const dummy_position = 
  CMDShell::cmd_position.add("varStr", -1);


void shell::CMD::CMDShell::help(Env& gEnv) {
  gEnv.stdOs << "shell: run a shell command." << endl;
  gEnv.stdOs << "    shell command" << endl;
  gEnv.stdOs << "Options:" << endl;
  gEnv.stdOs << "   -help                usage information." << endl;
  gEnv.stdOs << endl;
}

string shell::CMD::CMDShell::exec(const Tcl::object& tclObj, Env * pEnv) {
  Env &gEnv = *pEnv;
  vector<string> arg = tclObj.get<vector<string> >(gEnv.tclInterp->tcli);
  
  if(arg.size()>0 && arg[0] == "-help") {        // print help information
    shell::CMD::CMDShell::help(gEnv);
    return "";
  } else {
    string command = tclObj.get_string();
    if(is_tcl_list(command)) command = command.substr(1, command.size()-2);
    string rv = shell_exec(command);
    if(rv.size() > 0 && rv[rv.size()-1] == '\n')
      rv.erase(rv.size()-1);
    return rv;
  }
}
