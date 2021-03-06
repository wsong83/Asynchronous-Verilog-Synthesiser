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

#include "location.h"
#include "netlist/component.h"
#include "err_report.h"
#include "cmd_variable.h"

namespace shell {

  namespace CMD {
    // forward declaration
    class CMDTclFeed;
    class CMDTclInterp;
  }

  class Env {
  public:
    Env();
    virtual ~Env();

    // key function
    bool initialise();                      /* set up basic environment */
    void show_cmd(bool first_time = false); /* show the command line prompt */
    void run();                             /* running the synthesiser */

    // helpers
    /* search a netlist item using hierarchical name */
    boost::shared_ptr<netlist::NetComp> hierarchical_search(const std::string&) const;
    boost::shared_ptr<netlist::Module> find_module(const netlist::MIdentifier&) const;

    // data member
    std::map<std::string, boost::shared_ptr<netlist::Library> >  link_lib;     /* libraries used in design elaboration */
    std::map<std::string, boost::shared_ptr<netlist::Library> >  target_lib;   /* libraries used in mapping */
    ErrReport error;                                 /* the gobal level error report function */
    boost::shared_ptr<CMD::CMDTclFeed> tclFeed;        // the feed to getline from file or cin
    boost::shared_ptr<CMD::CMDTclInterp> tclInterp;    // the dynamic Tcl interpreter
    boost::shared_ptr<netlist::Library> curLib;      /* current library */
    boost::shared_ptr<netlist::Module> curDgn;       /* current design */
    std::ostream stdOs;                              /* standard output stream */
    std::ostream errOs;                              /* error output stream */
    std::map<std::string, CMD::CMDVar> macroDB;      /* command line macros */
  };

}

// all the commands in the command environment

// new tcl interpreter
//#include "cmd_tcl_feed.h"
//#include "cmd_tcl_interp.h"


#endif
// Local Variables:
// mode: c++
// End:
