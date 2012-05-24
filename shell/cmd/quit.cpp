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
 * argument definitions for quit/exit command
 * 15/05/2012   Wei Song
 *
 *
 */

#include "quit.h"

using namespace shell;
using namespace shell::CMD;
using std::endl;

static po::options_description arg_opt("Options");
po::options_description_easy_init const dummy_arg_opt =
  arg_opt.add_options()
  ("help", "usage information.")
  ;

po::options_description shell::CMD::CMDQuit::cmd_opt;
po::options_description const dummy_cmd_opt =
  CMDQuit::cmd_opt.add(arg_opt);

void shell::CMD::CMDQuit::help(Env& gEnv) {
  gEnv.stdOs << "exit/quit: leave the AVS shell environment." << endl;
  gEnv.stdOs << "    exit/quit [options]" << endl;
  gEnv.stdOs << cmd_name_fix(arg_opt) << endl;
}

bool shell::CMD::CMDQuit::exec ( Env& gEnv, vector<string>& arg){
  
  po::variables_map vm;

  try {
    store(po::command_line_parser(arg).options(cmd_opt).style(cmd_style).run(), vm);
    notify(vm);
  } catch (std::exception& e) {
    gEnv.stdOs << "Error: Wrong command syntax error! See usage by quit -help." << endl;
    return false;
  }

  if(vm.count("help")) {        // print help information
    shell::CMD::CMDQuit::help(gEnv);
    return false;               // do not quit when it is for help information
  } else {
    gEnv.stdOs << "Thank you." << endl;
    return true;                // quit
  }

}
