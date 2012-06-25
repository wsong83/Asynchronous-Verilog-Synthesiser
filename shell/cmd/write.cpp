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
 * write command
 * 25/06/2012   Wei Song
 *
 *
 */

#include "write.h"
#include "shell/macro_name.h"

using std::string;
using std::vector;
using std::endl;
using namespace shell::CMD;

static po::options_description arg_opt("Options");
static po::options_description_easy_init const dummy_arg_opt =
  arg_opt.add_options()
  ("help", "usage information.")
  ("hierarchy", "write out the whole hierarchy (default only the current level).")
  ("output", po::value<string>(), "the file name of the output file.")
  ;

static po::options_description design_opt;
static po::options_description_easy_init const dummy_design_opt =
  design_opt.add_options()
  ("design", po::value<string>(), "the target design name.")
  ;

po::options_description shell::CMD::CMDWrite::cmd_opt;
static po::options_description const dummy_cmd_opt =
  CMDWrite::cmd_opt.add(arg_opt).add(design_opt);

po::positional_options_description shell::CMD::CMDWrite::cmd_position;
static po::positional_options_description const dummy_position = 
  CMDWrite::cmd_position.add("design", 1);

void shell::CMD::CMDWrite::help(Env& gEnv) {
  gEnv.stdOs << "write: write out a design to a file." << endl;
  gEnv.stdOs << "    write [options] [design_name]" << endl;
  gEnv.stdOs << cmd_name_fix(arg_opt);
  gEnv.stdOs << "   design_name          the design to be written out (default the current " << endl;
  gEnv.stdOs << "                        design)." << endl << endl;
}

bool shell::CMD::CMDWrite::exec ( Env& gEnv, vector<string>& arg){
  
  po::variables_map vm;

  try {
    store(po::command_line_parser(arg).options(cmd_opt).style(cmd_style).positional(cmd_position).run(), vm);
    notify(vm);
  } catch (std::exception& e) {
    gEnv.stdOs << "Error: Wrong command syntax error! See usage by write -help." << endl;
    return false;
  }

  if(vm.count("help")) {        // print help information
    shell::CMD::CMDWrite::help(gEnv);
    return true;
  }

  // settle the design to be written out
  string designName;
  if(vm.count("design")) {
    designName = vm["design"].as<string>();
  } else {
    designName = gEnv.macroDB[MACRO_CURRENT_DESIGN].get_string();
  }

  // whether to output the hierarchy
  bool hierarchyOutPut = vm.count("hierarchy");

  // specify the output file name
  string outputFileName;
  if(vm.count("output")) outputFileName = vm["output"].as<string>();
  else outputFileName = designName + ".v";

  
  return true;
}
