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
 * Shell environment
 * 18/02/2012   Wei Song
 *
 *
 */

#include <boost/filesystem.hpp>

#include "shell_top.h"

#define YYSTYPE shell::CMD::cmd_token_type

#include "cmd/cmd_define.h"
#include "macro_name.h"

//make sure the location of command.y is included
#undef BISON_LOCATION_HH
#undef BISON_POSITION_HH
#include "command.hh"

using namespace shell;
using std::endl;
using std::cout;
using std::cerr;
using std::map;
using std::vector;
using std::list;
using boost::shared_ptr;
using netlist::Library;
using boost::filesystem::path;
using boost::filesystem::create_directory;
using boost::filesystem::remove_all;
using boost::filesystem::exists;


shell::Env::Env() 
  : stdOs(cout.rdbuf()), errOs(cerr.rdbuf())
{}

shell::Env::~Env() {
  delete parser;
  try {
    if(exists(macroDB[MACRO_TMP_PATH].get_string())) {
      remove_all(macroDB[MACRO_TMP_PATH].get_string());
    }
  } catch (std::exception e) {
    throw("Error! problem with removing or creating the temporary work directory.");
  }
}

// the variable hooks
#include "hook.cpp"

bool shell::Env::initialise() {
  // set up the default work library and add it in the link library
  shared_ptr<Library> work(new Library("work"));
  link_lib["work"] = work;

  // set work to be the current library
  this->curLib = work;

  // the env pointer in lexer
  lexer.set_env(this);

  // initialise the parser
  parser = new CMD::cmd_parser(this);

  // initialise the macro database
  // file search path
  macroDB[MACRO_SEARCH_PATH] = MACRO_SEARCH_PATH_VALUE;

  // the temporary file directory
  macroDB[MACRO_TMP_PATH] = MACRO_TMP_PATH_VALUE;
  path tmp_path(macroDB[MACRO_TMP_PATH].get_string());
  try {
    if(!exists(tmp_path)) {
      if(!create_directory(tmp_path)) {
        throw std::exception();
      }
    }
  } catch (std::exception e) {
    errOs << "Error! Fail to create the default temporary work directory!" << endl;
    exit(0);
  }
  macroDB[MACRO_TMP_PATH].hook.reset(new CMD::CMDHook_TMP_PATH());

  // current_design
  macroDB[MACRO_CURRENT_DESIGN] = MACRO_CURRENT_DESIGN_VALUE;
  macroDB[MACRO_CURRENT_DESIGN].hook.reset(new CMD::CMDHook_CURRENT_DESIGN());

  // show the welcome message
  show_cmd(true);

  return true;
}

void shell::Env::show_cmd(bool first_time) {
  if(first_time) {
    stdOs << "       Asynchronous Verilog Synthesis (AVS) System      " << endl;
    stdOs << "--------------------------------------------------------" << endl;
    stdOs << "Copyright (c) 2011-2012 Wei Song <songw@cs.man.ac.uk>   " << endl;
    stdOs << "  Compilation Date: " << __DATE__ << endl;
    stdOs << endl;
  } else {
    stdOs << "avs>";
  }
}

void shell::Env::run() {
  parser->parse();
}

