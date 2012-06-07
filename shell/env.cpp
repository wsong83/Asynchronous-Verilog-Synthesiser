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

#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

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
  shared_ptr<Library> work(new Library(MACRO_DEFAULT_WORK_LIB));
  link_lib[MACRO_DEFAULT_WORK_LIB] = work;

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

shared_ptr<netlist::NetComp> shell::Env::hierarchical_search(const string& m) const {
  vector<string> nameVector;
  shared_ptr<netlist::NetComp> rv;

  boost::split(nameVector, m, boost::is_any_of("/"), boost::token_compress_on);
  vector<string>::iterator it, end;
  it = nameVector.begin();
  end = nameVector.end();
  
  // the first level
  // assuming it is an item of current design
  if(curDgn.use_count() != 0) {
    rv = curDgn->search(*it);
  }

  if(rv.use_count() == 0) {     
    // failed to search it in current design
    // then search it as a module name
    rv = curLib->find(*it);

    // TODO: if failed to find it in current lib
    // it may be necessary to find it in all link libraries
  }

  // if failed, return an empty pointer
  if(rv.use_count() == 0) return rv;

  while(it != end) {
    // TODO:
    // find the hierarchical item in this module
    break;
  }

  return rv;

}

shared_ptr<netlist::Module> shell::Env::find_module(const netlist::MIdentifier& key) const {
  shared_ptr<netlist::Module> rv;
  
  // first try to find it in the current library
  if(curLib.use_count() != 0) {
    rv = curLib->find(key);
    if(rv.use_count() != 0) return rv;
  }

  // search it in all link library
  map<string, shared_ptr<netlist::Library> >::const_iterator it, end;
  for(it=link_lib.begin(), end=link_lib.end(); it!=end; it++) {
    rv = it->second->find(key);
    if(rv.use_count() != 0) return rv;
  }

  return rv;                    // return an empty pointer as failed
}
