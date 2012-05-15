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
 * argument definitions for help command
 * 14/05/2012   Wei Song
 *
 *
 */

#ifndef _AV_CMD_HELP_
#define _AV_CMD_HELP_

#include <map>
using std::map;

#include "shell/env.h"
#include "cmd_define.h"

namespace shell {
  namespace CMD {
    
    class CMDHelp {
    public:
      static void exec ( Env&, const vector<string>& arg = vector<string>() );
      static void help ( Env& );

      //private:
      typedef void (*helper) (Env&);
      static po::options_description cmd_opt;
      static po::positional_options_description cmd_position;
      static map<string, helper> cmdDB;
    };
  }
}      


#endif
