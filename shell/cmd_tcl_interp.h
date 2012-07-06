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

#ifndef _CMD_TCL_INTERP_H_
#define _CMD_TCL_INTERP_H_

#include "env.h"
//#include "cpptcl/cpptcl.h"

namespace Tcl {
  class interpreter;            // forward declaration
}

namespace shell {
  namespace CMD {
    
    class CMDTclInterp {
    public:
      CMDTclInterp();

      // helper
      void initialise(Env *, CMDTclFeed *);
      bool run();               // execute the interpreter

    private:
      Env * gEnv;
      CMDTclFeed * cmdFeed;
      boost::shared_ptr<Tcl::interpreter> interp;  // the dynamic Tcl interpreter
    };
  }
}

#endif /* _CMD_TCL_INTERP_H_ */

// Local Variables:
// mode: c++
// End:
