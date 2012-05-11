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

#include "shell_top.h"

using namespace shell;

shell::Env::Env() 
  : stdOs(cout.rdbuf()), errOs(cerr.rdbuf())
{}

bool shell::Env::initialise() {
  // set up the default work library and add it in the link library
  shared_ptr<Library> work(new Library("work"));
  link_lib["work"] = work;

  // set work to be the current library
  curLib = work;

  // the env pointer in lexer
  lexer.set_env(this);

  return true;
}

void shell::Env::show_cmd(bool first_time) {
  if(first_time) {
    stdOs << "       Asynchronous Verilog Synthesis (AVS) System      " << endl;
    stdOs << "--------------------------------------------------------" << endl;
    stdOs << "Copyright (c) 2011-2012 Wei Song <songw@cs.man.ac.uk>   " << endl;
    stdOs << "  Compilation Date: " << __DATE__ << endl;
    stdOs << endl;
  }

  stdOs << "avs>";
}

