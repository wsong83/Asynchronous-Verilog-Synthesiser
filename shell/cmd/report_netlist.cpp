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
 * report the internal structure of a netlist
 * 05/06/2012   Wei Song
 *
 *
 */

#include <list>
#include <vector>
#include <string>
#include "report_netlist.h"

using std::string;
using std::vector;
using std::endl;
using boost::shared_ptr;
using namespace shell;
using namespace shell::CMD;

po::options_description shell::CMD::CMDReportNetlist::cmd_opt;
static po::options_description_easy_init dummy_cmd_opt =
  CMDReportNetlist::cmd_opt.add_options()
  ("help",     "usage information.")
  ("net_item", po::value<string>(), "the hierarchy name of a netlist item.")
  ;

po::positional_options_description shell::CMD::CMDReportNetlist::cmd_position;
static po::positional_options_description const dummy_position = 
  CMDReportNetlist::cmd_position.add("net_item", 1);


void shell::CMD::CMDReportNetlist::help(Env& gEnv) {
  gEnv.stdOs << "report_netlist: display the internal structure of a netlist item." << endl;
  gEnv.stdOs << "    report_netlist netlist_item" << endl;
  gEnv.stdOs << "Options:" << endl;
  gEnv.stdOs << "   -help                usage information." << endl;
  gEnv.stdOs << endl;
}

bool shell::CMD::CMDReportNetlist::exec( Env& gEnv, vector<string>& arg) {
  po::variables_map vm;

  try {
    store(po::command_line_parser(arg).options(cmd_opt).style(cmd_style).positional(cmd_position).run(), vm);
    notify(vm);
  } catch (std::exception& e) {
    gEnv.stdOs << "Error: Wrong command syntax error! See usage by report_netlist -help." << endl;
    return false;
  }

  if(vm.count("help")) {        // print help information
    shell::CMD::CMDReportNetlist::help(gEnv);
    return true;
  } else {
    string netItem;
    if(vm.count("net_item")) {
      netItem = vm["net_item"].as<string>();
      shared_ptr<netlist::NetComp> mitem = gEnv.hierarchical_search(netItem);
      if(mitem.use_count() != 0)
        gEnv.stdOs << *mitem << endl;
      else {
        gEnv.stdOs << "Error: Fail to find any item named \"" << netItem << "\"." << endl;
        return false;
      }
      return true;
    } 
    shell::CMD::CMDReportNetlist::help(gEnv);
    return true;
  }
}
