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
 * argument definitions
 * 10/05/2012   Wei Song
 *
 *
 */

#ifndef AV_CMD_CMD_DEFINE_
#define AV_CMD_CMD_DEFINE_

#include <boost/program_options.hpp>
  namespace po = boost::program_options;
#include <boost/algorithm/string/replace.hpp>
#include <iostream>
#include <sstream>
#include <string>

namespace shell { 
  namespace CMD {
    const po::command_line_style::style_t cmd_style = 
      po::command_line_style::style_t(
                                      po::command_line_style::unix_style |
                                      po::command_line_style::allow_long_disguise
                                      );

    // get rid of the --long-name problem
    inline std::string cmd_name_fix(const po::options_description& opt) {
      std::stringstream tmp;
      tmp << opt;
      std::string opt_str = tmp.str();
      boost::algorithm::replace_all(opt_str, " --", "  -");
      return opt_str;
    }
  }
}

#ifndef NEW_TCL_CMD
#define NEW_TCL_CMD(rt_type, cmd_name)                         \
struct cmd_name {                                              \
  static rt_type exec ( const std::string&, Env *);            \
  static void help ( Env& );                                   \
  static const std::string name;                               \
  static const std::string description;                        \
}
#endif                               

namespace shell {
  class Env;
  namespace CMD {
    NEW_TCL_CMD(bool, CMDWrite);
    NEW_TCL_CMD(bool, CMDAnalyze);
  }
}


#undef NEW_TCL_CMD
#endif
