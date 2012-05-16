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
 * argument definitions for analyze command
 * 10/05/2012   Wei Song
 *
 *
 */

#include "analyze.h"

using std::string;
using std::vector;
using std::endl;
using namespace shell;
using namespace shell::CMD;

static po::options_description arg_opt("Options");
po::options_description_easy_init const dummy_arg_opt =
  arg_opt.add_options()
  ("help", "usage information.")
  ("library", po::value<string>(), "set the output library (other than work).")
  ("define", po::value<vector<string> >()->composing(), "macro definitions ( {MACRO0, MACRO1, ... MACRON} ).")
  ;

static po::options_description file_opt;
po::options_description_easy_init const dummy_file_opt =
  file_opt.add_options()
  ("file", po::value<vector<string> >()->composing(), "input files")
  ;

po::options_description shell::CMD::CMDAnalyze::cmd_opt;
po::options_description const dummy_cmd_opt =
  CMDAnalyze::cmd_opt.add(arg_opt).add(file_opt);

po::positional_options_description shell::CMD::CMDAnalyze::cmd_position;
po::positional_options_description const dummy_position = 
  CMDAnalyze::cmd_position.add("file", -1);

void shell::CMD::CMDAnalyze::help(Env& gEnv) {
  gEnv.stdOs << "analyze: read in the Verilog HDL design files." << endl;
  gEnv.stdOs << "    analyze [options] source_files" << endl;
  gEnv.stdOs << cmd_name_fix(arg_opt) << endl;
}

bool shell::CMD::CMDAnalyze::exec ( Env& gEnv, vector<string>& arg){
  
  po::variables_map vm;

  try {
    store(po::command_line_parser(arg).options(cmd_opt).style(cmd_style).positional(cmd_position).run(), vm);
    notify(vm);
  } catch (std::exception& e) {
    gEnv.errOs << "Wrong command syntax error! See usage by analyze -help." << endl;
    return false;
  }

  // TODO: parse the file
  if(vm.count("help")) {        // print help information
    shell::CMD::CMDAnalyze::help(gEnv);
    return true;
  }


  return true;
}
