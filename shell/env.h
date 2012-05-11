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

#ifndef _H_SHELL_SHELL_ENV_
#define _H_SHELL_SHELL_ENV_

#include "netlist/component.h"
using netlist::Library;
using netlist::Module;

#include "cmd_lexer.h"
#include "err_report.h"

namespace shell {

  class Env {
  public:
    Env();

    bool initialise();               /* set up basic environment */
    void show_cmd(bool first_time = false); /* show the command line prompt */

    // data member
    map<string, shared_ptr<Library> >  link_lib;     /* libraries used in design elaboration */
    map<string, shared_ptr<Library> >  target_lib;   /* libraries used in mapping */
    ErrReport error;                                 /* the gobal level error report function */
    CMD::CMDLexer lexer;                             /* the command line lexer */
    shared_ptr<Library> curLib;                      /* current library */
    shared_ptr<Module> curDgn;                       /* current design */
    ostream stdOs;                                   /* standard output stream */
    ostream errOs;                                   /* error output stream */
    
  };

}


#endif
