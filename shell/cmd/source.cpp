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
 * argument definitions for source command
 * 15/05/2012   Wei Song
 *
 *
 */

#include "source.h"
#include<fstream>
#include<iostream>
using std::ifstream;
using std::endl;
using std::vector;
using std::string;
using namespace shell;
using namespace shell::CMD;

static po::options_description arg_opt("Options");
static po::options_description_easy_init const dummy_arg_opt =
  arg_opt.add_options()
  ("help", "usage information.")
  ;

static po::options_description file_opt;
static po::options_description_easy_init const dummy_file_opt =
  file_opt.add_options()
  ("file", po::value<string>(), "input files")
  ;

po::options_description shell::CMD::CMDSource::cmd_opt;
static po::options_description const dummy_cmd_opt =
  CMDSource::cmd_opt.add(arg_opt).add(file_opt);

po::positional_options_description shell::CMD::CMDSource::cmd_position;
static po::positional_options_description const dummy_position = 
  CMDSource::cmd_position.add("file", 1);

void shell::CMD::CMDSource::help(Env& gEnv) {
  gEnv.stdOs << "source: read and execute another script file." << endl;
  gEnv.stdOs << "    source [options] script_file" << endl;
  gEnv.stdOs << cmd_name_fix(arg_opt) << endl;
}

bool shell::CMD::CMDSource::exec ( Env& gEnv, vector<string>& arg){
  
  po::variables_map vm;

  try {
    store(po::command_line_parser(arg).options(cmd_opt).style(cmd_style).positional(cmd_position).run(), vm);
    notify(vm);
  } catch (std::exception& e) {
    gEnv.stdOs << "Error: Wrong command syntax error! See usage by source -help." << endl;
    return false;
  }

  if(vm.count("help") || vm.size() == 0) {        // print help information
    shell::CMD::CMDSource::help(gEnv);
    return true;
  }

  if(vm.count("file")) {
    string fname = vm["file"].as<string>();
    if(fname != "") {
      ifstream * file_handler = new ifstream(fname.c_str());
      if(file_handler->good()) { // success
        //gEnv.lexer.push(file_handler);
      } else {
        delete file_handler;
        gEnv.stdOs << "Error: Cannot open script file \"" << fname << "\"!" << endl;
        return false;
      }
    } else {
      gEnv.stdOs << "Error: File name is empty!" << endl; // should not come here
      return false;
    }
  }
  return true;
}
