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
 * suppress some warning messages
 * 23/05/2012   Wei Song
 *
 *
 */

#include "suppress_message.h"
using std::string;
using std::vector;
using std::endl;
using namespace shell::CMD;

static po::options_description arg_opt("Options");
po::options_description_easy_init const dummy_arg_opt =
  arg_opt.add_options()
  ("help", "usage information.")
  ;

static po::options_description target_opt;
po::options_description_easy_init const dummy_target_opt =
  target_opt.add_options()
  ("msgName", po::value<vector<string> >()->composing(), "the warning to be suppressed")
  ;

po::options_description shell::CMD::CMDSuppressMessage::cmd_opt;
po::options_description const dummy_cmd_opt =
  CMDSuppressMessage::cmd_opt.add(arg_opt).add(target_opt);

po::positional_options_description shell::CMD::CMDSuppressMessage::cmd_position;
po::positional_options_description const dummy_position = 
  CMDSuppressMessage::cmd_position.add("msgName", -1);

void shell::CMD::CMDSuppressMessage::help(Env& gEnv) {
  gEnv.stdOs << "suppress_message: suppress the report of some messages." << endl;
  gEnv.stdOs << "    suppress_message [options] message_names" << endl;
  gEnv.stdOs << cmd_name_fix(arg_opt) << endl;
}

bool shell::CMD::CMDSuppressMessage::exec ( Env& gEnv, vector<string>& arg){

  po::variables_map vm;

  try {
    store(po::command_line_parser(arg).options(cmd_opt).style(cmd_style).positional(cmd_position).run(), vm);
    notify(vm);
  } catch (std::exception& e) {
    gEnv.errOs << "Error: Wrong command syntax error! See usage by suppress_message -help." << endl;
    return false;
  }

  if(vm.count("help")) {        // print help information
    shell::CMD::CMDSuppressMessage::help(gEnv);
    return true;
  }

  if(vm.count("msgName")) {
    vector<string> msg_name = vm["msgName"].as<vector<string> >();
    vector<string>::iterator it, end;
    for(it=msg_name.begin(), end=msg_name.end(); it!=end; it++) {
      if(!gEnv.error.suppress(*it)) {
        gEnv.errOs << "Error: Error message \"" << *it << "\" does not exist or cannot be suppressed."<< endl;
      }
    }
    return true;
  }

  shell::CMD::CMDSuppressMessage::help(gEnv);
  return true;
}
