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

// uncomment it when need to debug Spirit.Qi
//#define BOOST_SPIRIT_QI_DEBUG

#include "cmd_define.h"
#include "cmd_parse_base.h"
#include "shell/env.h"
#include "shell/macro_name.h"

#include <fstream>
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
#include "preproc/preproc.h"
#include "shell/macro_name.h"
#include "averilog/averilog_util.h"
#include "averilog/averilog.lex.h"
#include <boost/foreach.hpp>

using std::endl;
using std::ofstream;
using boost::shared_ptr;
using boost::filesystem::path;
using boost::filesystem::exists;
using namespace shell;
using namespace shell::CMD;


namespace {
  namespace qi = boost::spirit::qi;
  namespace phoenix = boost::phoenix;
  namespace ascii = boost::spirit::ascii;

  struct Argument {
    bool bHelp;                           // show help information
    std::string sFormat;                  // language
    std::string sLibrary;                 // target work library
    std::vector<std::string> svDefine;    // macro definitions
    std::vector<std::string> svFile;      // target soruce file 
    
    Argument() : 
      bHelp(false),
      sFormat("verilog"),
      sLibrary("work") {}
  };
}

BOOST_FUSION_ADAPT_STRUCT
(
 Argument,
 (bool, bHelp)
 (std::string, sFormat)
 (std::string, sLibrary)
 (std::vector<std::string>, svDefine)
 (std::vector<std::string>, svFile)
 )

namespace {
  typedef std::string::const_iterator SIter;

  struct ArgParser : qi::grammar<SIter, Argument()>, cmd_parse_base<SIter> {
    qi::rule<SIter, void(Argument&)> args;
    qi::rule<SIter, Argument()> start;
    
    ArgParser() : ArgParser::base_type(start) {
      using qi::lit;
      using ascii::char_;
      using phoenix::at_c;
      using phoenix::push_back;
      using namespace qi::labels;
      
      args = 
        ( lit('-') >> 
          ( (lit("help")    >> blanks)                         [at_c<0>(_r1) = true] ||
            (lit("format")  >> blanks >> text       >> blanks) [at_c<1>(_r1) = _1]   ||
            (lit("library") >> blanks >> text       >> blanks) [at_c<2>(_r1) = _1]   ||
            (lit("define")  >> blanks >> identifier >> blanks) [push_back(at_c<3>(_r1), _1)] 
           )
         ) 
        || +(text >> blanks)                    [push_back(at_c<4>(_r1), _1)]
        ;
      
      start = +(args(_val));

#ifdef BOOST_SPIRIT_QI_DEBUG
      BOOST_SPIRIT_DEBUG_NODE(args);
      BOOST_SPIRIT_DEBUG_NODE(start);
      BOOST_SPIRIT_DEBUG_NODE(text);
      BOOST_SPIRIT_DEBUG_NODE(blanks);
      BOOST_SPIRIT_DEBUG_NODE(identifier);
      BOOST_SPIRIT_DEBUG_NODE(filename);
#endif
    }
  };
}

const std::string shell::CMD::CMDAnalyze::name = "analyze"; 
const std::string shell::CMD::CMDAnalyze::description = 
  "read in the Verilog HDL design files.";


void shell::CMD::CMDAnalyze::help(Env& gEnv) {
  gEnv.stdOs << name << ": " << description << endl;
  gEnv.stdOs << "    analyze [options] source_files" << endl;
  gEnv.stdOs << "    source_file        +the design to be read in." << endl;
  gEnv.stdOs << "Options:" << endl;
  gEnv.stdOs << "   -help                show this help information." << endl;
  gEnv.stdOs << "   -format language     the source file language (default verilog)." << endl;
  gEnv.stdOs << "   -library lib_name    specify the target work library (default work)." << endl;
  gEnv.stdOs << "   -define MACRO_NAME  *define a macro in Verilog." << endl;
}

bool shell::CMD::CMDAnalyze::exec (const std::string& str, Env * pEnv){

  using std::string;

  Env &gEnv = *pEnv;

  // parse
  string::const_iterator it = str.begin(), end = str.end();
  ArgParser parser;             // argument parser
  Argument arg;                 // argument struct
  bool r = qi::parse(it, end, parser, arg);

  if(!r || it != end) {
    gEnv.stdOs << "Error: Wrong command syntax error! See usage by analyze -help." << endl;
    gEnv.stdOs << "    analyze [options] source_files" << endl;
    return false;
  }

  if(arg.bHelp) {        // print help information
    help(gEnv);
    return true;
  }

  //set the target library
  if(arg.sLibrary.empty()) {
    gEnv.stdOs << "Error: The target library name is empty." << endl;
    return false;
  } else {
    if(!gEnv.link_lib.count(arg.sLibrary))
      gEnv.link_lib[arg.sLibrary] = shared_ptr<netlist::Library>(new netlist::Library(arg.sLibrary, arg.sLibrary+".db"));
    
    gEnv.curLib = gEnv.link_lib[arg.sLibrary];
  }
  
  // analyse the target files
  if(arg.svFile.size()) {
    BOOST_FOREACH(const string& it, arg.svFile) {
      //find the correct file name
      path fname(it);
      if(!exists(fname)) {
        bool m_exist = false;
        BOOST_FOREACH(const string& sit, gEnv.macroDB[MACRO_SEARCH_PATH].get_list()) {
          fname = sit + "/" + it;
          if(exists(fname)) { m_exist = true; break;}
        }
        if(!m_exist) {       // fail to locate the file
          gEnv.stdOs << "Error: Cannot open file \"" << it << "\"." << endl;
          return false;
        }
      }

      // run the preprocessor first
      gEnv.stdOs << "Read in \"" << fname.string() << "\"" << endl;
      VPPreProc::VFileLineXs* filelinep = new VPPreProc::VFileLineXs(NULL);
      VPPreProc::VPreProcXs* preprocp = new VPPreProc::VPreProcXs();
      
      // ATTN: set parameters before configure the preprocessor
      preprocp->keepComments(1);
      preprocp->keepWhitespace(1);
      preprocp->lineDirectives(1);
      preprocp->pedantic(0);
      preprocp->synthesis(1);     // do not parse the lines between synopsys translate off and on

      filelinep->setPreproc(preprocp);
      preprocp->configure(filelinep);
      if(!preprocp->openFile(fname.string())) {
        gEnv.stdOs << "Error: Cannot open file \"" << fname << "\"." << endl;
        delete preprocp;
        return false;
      }
      
      //set the pre-defined macros
      if(arg.svDefine.size()) {
        BOOST_FOREACH(const string& mit, arg.svDefine)
          preprocp->define(mit, "", "", true);
      }

      // set the include paths
      BOOST_FOREACH(const string& iit, gEnv.macroDB[MACRO_SEARCH_PATH].get_list())
        preprocp->add_incr(iit);
      
      // set the output file
      ofstream of_handle;
      string tmp_file_name = gEnv.macroDB[MACRO_TMP_PATH].get_string() + "/" + fname.filename().string() + ".preproc";
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
  }
  
  return true;
}

