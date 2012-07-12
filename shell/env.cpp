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
#include "cmd_tcl_feed.h"
#include "cmd_tcl_interp.h"

#include <boost/foreach.hpp>

#define YYSTYPE shell::CMD::cmd_token_type

// the commands
#include "cmd/analyze.h"
#include "cmd/current_design.h"
#include "cmd/echo.h"
#include "cmd/elaborate.h"
#include "cmd/exit.h"
#include "cmd/help.h"
#include "cmd/report_netlist.h"
#include "cmd/shell.h"
#include "cmd/suppress_message.h"
#include "cmd/write.h"

#include "macro_name.h"

using namespace shell;
using std::endl;
using std::cout;
using std::cerr;
using std::map;
using std::vector;
using std::list;
using std::string;
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
  stdOs << "Release the Asynchronous Verilog Synthesis system..." << endl;
  try {
    if(exists(tclInterp->tcli.read_variable<string>(MACRO_TMP_PATH))) {
      remove_all(tclInterp->tcli.read_variable<string>(MACRO_TMP_PATH));
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

  // tcl interpreter
  tclFeed.reset(new CMD::CMDTclFeed());
  tclInterp.reset(new CMD::CMDTclInterp());

  tclFeed->initialise(this);
  tclInterp->initialise(this, tclFeed.get());

  Tcl::interpreter& i = tclInterp->tcli;

  // initialise the macro database
  // file search path
  i.set_variable(MACRO_SEARCH_PATH, MACRO_SEARCH_PATH_VALUE);

  // the temporary file directory
  i.set_variable(MACRO_TMP_PATH, MACRO_TMP_PATH_VALUE);
  path tmp_path(MACRO_TMP_PATH_VALUE);
  cmd_create_tmp_path(tmp_path, this, true);
  // also save it in AVS's own variable db
  macroDB[MACRO_TMP_PATH] = MACRO_TMP_PATH_VALUE;
  // trace the variable
  i.def_write_trace(MACRO_TMP_PATH, MACRO_TMP_PATH, CMDHook_TMP_PATH, this);
  
  // current_design
  i.set_variable(MACRO_CURRENT_DESIGN, MACRO_CURRENT_DESIGN_VALUE);
  macroDB[MACRO_CURRENT_DESIGN] = MACRO_CURRENT_DESIGN_VALUE;
  i.def_write_trace(MACRO_CURRENT_DESIGN, MACRO_CURRENT_DESIGN, 
                    CMDHook_CURRENT_DESIGN, this);

  // add the commands defined for AVS
  i.def("analyze",        shell::CMD::CMDAnalyze::exec,        this, Tcl::variadic());
  i.def("current_design", shell::CMD::CMDCurrentDesign::exec,  this, Tcl::variadic());
  i.def("echo",           shell::CMD::CMDEcho::exec,           this, Tcl::variadic());
  i.def("elaborate",      shell::CMD::CMDElaborate::exec,      this, Tcl::variadic());
  i.def("exit",           shell::CMD::CMDExit::exec,           this, Tcl::variadic());
  i.def("help",           shell::CMD::CMDHelp::exec,           this, Tcl::variadic());
  i.def("report_netlist", shell::CMD::CMDReportNetlist::exec,  this, Tcl::variadic());
  i.def("shell",          shell::CMD::CMDShell::exec,          this, Tcl::variadic());
  i.def("suppress_message", 
                          shell::CMD::CMDSuppressMessage::exec,this, Tcl::variadic());
  i.def("write",          shell::CMD::CMDWrite::exec,          this, Tcl::variadic());

  // make "quit" an alias of "exit"
  i.create_alias("quit", i, "exit");

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
  tclInterp->run();
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
