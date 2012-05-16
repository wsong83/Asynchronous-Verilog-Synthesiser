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
 * argument definitions for help command
 * 14/05/2012   Wei Song
 *
 *
 */

#include "help.h"
#include "analyze.h"

using namespace shell;
using namespace shell::CMD;
using std::map;
using std::string;
using std::pair;
using std::endl;

// the command list
map<string, string> shell::CMD::CMDHelp::cmdDB;

int cmdDB_init( map<string, string>& db) {
  db.insert(pair<string, string>("analyze",  "show and list the usage of commands."    ));
  db.insert(pair<string, string>("exit",     "leave the AVS shell environment."        ));
  db.insert(pair<string, string>("help",     "read in the Verilog HDL design files."   ));
  db.insert(pair<string, string>("quit",     "leave the AVS shell environment."        ));
  return 0;
}

// use the dummy variable to initialize the db
static int const dummy_cmdDB = cmdDB_init(CMDHelp::cmdDB);

static po::options_description arg_opt("Options");
po::options_description_easy_init const dummy_arg_opt =
  arg_opt.add_options()
  ("help", "usage information.")
  ;

static po::options_description target_opt;
po::options_description_easy_init const dummy_target_opt =
  target_opt.add_options()
  ("target", po::value<vector<string> >()->composing(), "target command names")
  ;

po::options_description shell::CMD::CMDHelp::cmd_opt;
po::options_description const dummy_cmd_opt =
  CMDHelp::cmd_opt.add(arg_opt).add(target_opt);

po::positional_options_description shell::CMD::CMDHelp::cmd_position;
po::positional_options_description const dummy_position =
  CMDHelp::cmd_position.add("target", -1);

void shell::CMD::CMDHelp::help(Env& gEnv) {
  gEnv.stdOs << "help: show and list the usage of commands." << endl;
  gEnv.stdOs << "    analyze [options] commands" << endl;
  gEnv.stdOs << cmd_name_fix(arg_opt) << endl;
}

bool shell::CMD::CMDHelp::exec ( Env& gEnv, vector<string>& arg){
  
  po::variables_map vm;
  //arg.insert(arg.begin(), "help");

  try {
    store(po::command_line_parser(arg).options(cmd_opt).style(cmd_style).positional(cmd_position).run(), vm);
    notify(vm);
  } catch(std::exception& e) {
    gEnv.errOs << "Wrong command syntax error! See usage by help -help." << endl;
    return false;
  }

  if(vm.count("help")) {        // print help information
    shell::CMD::CMDHelp::help(gEnv);
  }
  else if(vm.count("target")) {
    vector<string> cmd_lst = vm["target"].as<vector<string> >();
    vector<string>::iterator it, end;
    for(it=cmd_lst.begin(), end=cmd_lst.end(); it!=end; it++) {
      if(cmdDB.find(*it) != cmdDB.end()) {
        gEnv.stdOs << *it << 
          string(it->size() < 16 ? 16-it->size() : 1, ' ') 
                   << ": " << cmdDB[*it] << endl;
      } else {
        gEnv.errOs << "Wrong command name: " << *it << endl;
        break;
      }
    }
  } else {
    // list all command;
    map<string, string>::iterator it, end;
    for(it=cmdDB.begin(), end=cmdDB.end(); it!=end; it++)
        gEnv.stdOs << it->first << 
          string(it->first.size() < 24 ? 24 - it->first.size() : 1, ' ')
                   << it->second << endl;
    gEnv.stdOs << endl;
  }

  return true;
}
