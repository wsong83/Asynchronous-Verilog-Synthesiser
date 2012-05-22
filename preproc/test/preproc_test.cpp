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
 * Test for the Verilog preprocessor
 * 27/04/2012   Wei Song
 *
 *
 */

#include "preproc/preproc.h"

#include <boost/program_options.hpp>
#include <fstream>

namespace po = boost::program_options;
using std::string;
using std::vector;
using std::endl;
using std::cout;
using namespace VPPreProc;

int main(int argc, char** argv)
{

  string inpfile;
  string outpfile;
  std::ofstream of_handle;


  unsigned int opt_keep_comment;
  unsigned int opt_keep_white_space;
  unsigned int opt_line_directive;
  unsigned int opt_pedantic;
  unsigned int opt_synthesis;

  po::options_description preproc_opt("Preprocessor options");
  preproc_opt.add_options()
    ("help", "print usage")
    ("include,I", po::value<vector<string> >()->composing(), "include paths")
    ("macro,D", po::value<vector<string> >()->composing(), "macro defines")
    ("output,o", po::value<string>(), "output file")
    ("keep_comment", po::value<unsigned int>(&opt_keep_comment)->default_value(1), 
     "keep comment (default true/1)")
    ("keep_white_space", po::value<unsigned int>(&opt_keep_white_space)->default_value(1), 
     "keep white space (default true/1)")
    ("line_directive", po::value<unsigned int>(&opt_line_directive)->default_value(1), 
     "insert line directive (default true/1)")
    ("pedantic", po::value<unsigned int>(&opt_pedantic)->default_value(0), 
     "pedantic, obey stanard (default false/0)")
    ("synthesis", po::value<unsigned int>(&opt_synthesis)->default_value(1), 
     "recognize synthesis on and off directives (default true/1)")
    ;

  po::options_description input_opt;
  input_opt.add_options()
    ("input", po::value<string>(), "input file")
    ;

  po::options_description all_opt;
  all_opt.add(preproc_opt).add(input_opt);

  po::positional_options_description p;
  p.add("input", 1);

  po::variables_map vm;

  try {
    store(po::command_line_parser(argc, argv).options(all_opt).positional(p).run(), vm);
    notify(vm);
  } catch(std::exception& e) {
    cout << "Wrong command syntax! See usage using --help." << endl;
    return 1;
  }

  if(vm.count("help")) {        // print help information
    cout << "Verilog preprocessor: vpreproc [options] source_file" << endl;
    cout << preproc_opt << endl;
    return 1;
  }

  if(vm.count("input")) {       // fetch input file
    inpfile = vm["input"].as<string>();
  } else {
    cout << "No source file provided! See usage using --help." << endl;
    return 1;
  } 
  
  if(vm.count("output")) {
    outpfile = vm["output"].as<string>();
    if(outpfile != "") {
      of_handle.open(outpfile.c_str(), std::ios::out);
    }
  }

  VFileLineXs* filelinep = new VFileLineXs(NULL);
  VPreProcXs* preprocp = new VPreProcXs();
  filelinep->setPreproc(preprocp);
  preprocp->configure(filelinep);
  preprocp->openFile(inpfile);

  preprocp->keepComments(opt_keep_comment);
  preprocp->keepWhitespace(opt_keep_white_space);
  preprocp->lineDirectives(opt_line_directive != 0);
  preprocp->pedantic(opt_pedantic != 0);
  preprocp->synthesis(opt_synthesis != 0);

  if(vm.count("macro")) {
    vector<string> mlist = vm["macro"].as<vector<string> >();
    vector<string>::iterator it, end;
    for(it=mlist.begin(), end=mlist.end(); it !=end; it++)
      preprocp->define(*it, "", "", true);
  }

  if(vm.count("include")) {
    vector<string> inclist = vm["include"].as<vector<string> >();
    vector<string>::iterator it, end;
    for(it=inclist.begin(), end=inclist.end(); it !=end; it++)
      preprocp->add_incr(*it);
  }

  while(true) {
    string rt = preprocp->getline();
    if(outpfile != "") {
      of_handle << rt;
    } else {
      cout << rt;
    }
    if(rt.size() == 0) break;
  }

  if(outpfile != "") 
    of_handle.close();  

  return 1;
}
