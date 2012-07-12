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
 * The wrapper for Tcl interpreter
 * 03/07/2012   Wei Song
 *
 *
 */

#include "cmd_tcl_feed.h"
#include "cmd_tcl_interp.h"

using std::string;
using std::endl;

shell::CMD::CMDTclInterp::CMDTclInterp() 
  : gEnv(NULL), cmdFeed(NULL)
{
}

shell::CMD::CMDTclInterp::~CMDTclInterp() {
  gEnv->stdOs << "Release the embedded Tcl interpreter..." << endl;
}

void shell::CMD::CMDTclInterp::initialise(Env * mgEnv, CMDTclFeed * mfeed) {
  gEnv = mgEnv;
  cmdFeed = mfeed;
}

bool shell::CMD::CMDTclInterp::run() {

  while(true) {
    try {
      string rv = tcli.eval(cmdFeed->getline());
      if(!rv.empty())
        gEnv->stdOs << rv << endl;
    } catch(const Tcl::tcl_error& e) {
      if("CMD_TCL_EXIT" == string(e.what())) { // special exit message
        gEnv->stdOs << "\nThank you!" << endl;
        return true;
      }
      gEnv->stdOs << "[Tcl] " << e.what() << endl;
    } catch (const std::exception& e) {
      gEnv->errOs << "[OS Exception] " << e.what() << endl;
      return false;
    } 
  }
}
