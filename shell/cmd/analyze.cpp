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

#include <fstream>
#include "analyze.h"
#include "preproc/preproc.h"
#include "shell/macro_name.h"
#include "averilog/averilog_util.h"
#include "averilog/averilog.lex.h"

using std::string;
using std::vector;
using std::list;
using std::endl;
using std::ofstream;
using boost::shared_ptr;
using namespace shell;
using namespace shell::CMD;

static po::options_description arg_opt("Options");
po::options_description_easy_init const dummy_arg_opt =
  arg_opt.add_options()
  ("help", "usage information.")
  ("library", po::value<string>(), "set the output library (other than work).")
  ("define", po::value<vector<string> >()->composing(), "macro definitions.")
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
    gEnv.errOs << "Error: Wrong command syntax error! See usage by analyze -help." << endl;
    return false;
  }

  if(vm.count("help")) {        // print help information
    shell::CMD::CMDAnalyze::help(gEnv);
    return true;
  }

  //set the target library
  if(vm.count("library")) {
    string lib_name = vm["library"].as<string>();
    if(lib_name.empty()) {
      gEnv.errOs << "Error: The target library name is empty." << endl;
      return false;
    } else {
      if(!gEnv.link_lib.count(lib_name))
        gEnv.link_lib[lib_name] = shared_ptr<netlist::Library>(new netlist::Library(lib_name, lib_name+".db"));

      gEnv.curLib = gEnv.link_lib[lib_name];
    }
  }

  // analyse the target files
  if(vm.count("file")) {
    vector<string> target_files = vm["file"].as<vector<string> >();
    vector<string>::iterator it, end;
    for(it=target_files.begin(), end=target_files.end(); it!=end; it++) {
      // run the preprocessor first
      gEnv.stdOs << "Read in \"" << *it << "\"" << endl;
      VPPreProc::VFileLineXs* filelinep = new VPPreProc::VFileLineXs(NULL);
      VPPreProc::VPreProcXs* preprocp = new VPPreProc::VPreProcXs();
      filelinep->setPreproc(preprocp);
      preprocp->configure(filelinep);
      if(!preprocp->openFile(*it)) {
        gEnv.errOs << "Error: Cannot open file \"" << *it << "\"." << endl;
        delete preprocp;
        return false;
      }
      
      preprocp->keepComments(1);
      preprocp->keepWhitespace(1);
      preprocp->lineDirectives(1);
      preprocp->pedantic(0);
      preprocp->synthesis(1);

      //set the pre-defined macros
      if(vm.count("define")) {
        vector<string> macro_list = vm["define"].as<vector<string> >();
        vector<string>::const_iterator mit, mend;
        for(mit=macro_list.begin(), mend=macro_list.end(); mit!=mend; mit++){
          preprocp->define(*mit, "", "", true);
        }
      }

      // set the include paths
      if(gEnv.macroDB[MACRO_SEARCH_PATH].is_string()) {
        preprocp->add_incr(gEnv.macroDB[MACRO_SEARCH_PATH].get_string());
      } else if(gEnv.macroDB[MACRO_SEARCH_PATH].is_list()) {
        list<string>::const_iterator iit, iend;
        for( iit=gEnv.macroDB[MACRO_SEARCH_PATH].get_list().begin(), 
               iend=gEnv.macroDB[MACRO_SEARCH_PATH].get_list().end();
             iit!=iend; iit++) {
          preprocp->add_incr(*iit);
        }
      } else {
        assert(0 == "search_path is not a string nor a list!");
      }

      // set the output file
      ofstream of_handle;
      string tmp_file_name = gEnv.macroDB[MACRO_TMP_PATH].get_string() + "/verilog.preproc";
      //gEnv.stdOs << "write to temporary file \"" << tmp_file_name << "\"." << endl;
      of_handle.open(tmp_file_name.c_str(), std::ios::out);

      // preprocess the file
      while(true) {
        string rt = preprocp->getline();
        of_handle << rt;
        if(rt.size() == 0) break;
      }

      of_handle.close();
      //delete filelinep;
      delete preprocp;

      // call the verilog parser
      averilog::Parser m_parser(tmp_file_name.c_str(), gEnv);
      m_parser.initialize();
      m_parser.parse();
    }
    return true;
  }
  
  shell::CMD::CMDAnalyze::help(gEnv);    
  return true;
}
