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
 * echo command
 * 21/05/2012   Wei Song
 *
 *
 */

#include <list>
#include <string>
#include "echo.h"
#include "shell/cmd_utility.h"
#include <boost/foreach.hpp>

using std::vector;
using std::endl;
using std::string;
using namespace shell;
using namespace shell::CMD;

po::options_description shell::CMD::CMDEcho::cmd_opt;
static po::options_description_easy_init dummy_cmd_opt =
  CMDEcho::cmd_opt.add_options()
  ("help",     "usage information.")
  ("varStr", po::value<vector<string> >()->composing(), "variable value.")
  ;

po::positional_options_description shell::CMD::CMDEcho::cmd_position;
static po::positional_options_description const dummy_position = 
  CMDEcho::cmd_position.add("varStr", -1);


void shell::CMD::CMDEcho::help(Env& gEnv) {
  gEnv.stdOs << "echo: display a string with variables." << endl;
  gEnv.stdOs << "    echo Strings" << endl;
  gEnv.stdOs << "Options:" << endl;
  gEnv.stdOs << "   -help                usage information." << endl;
  gEnv.stdOs << endl;
}

string shell::CMD::CMDEcho::exec(const Tcl::object& tclObj, Env * pEnv) {
  po::variables_map vm;
  Env &gEnv = *pEnv;
  vector<string> arg = tcl_argu_parse(tclObj);
  string rv;

  try {
    store(po::command_line_parser(arg).options(cmd_opt).style(cmd_style).positional(cmd_position).run(), vm);
    notify(vm);
  } catch (std::exception& e) {
    gEnv.stdOs << "Error: Wrong command syntax error! See usage by echo -help." << endl;
    return rv;
  }

  if(vm.count("help")) {        // print help information
    shell::CMD::CMDEcho::help(gEnv);
    return rv;
  } else {
    rv = tclObj.get_string();
    return rv;
  }
}
