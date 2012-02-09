/*
 * Copyright (c) 2011 Wei Song <songw@cs.man.ac.uk> 
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
 * Definition of netlist components.
 * 01/02/2011   Wei Song
 *
 *
 */

#ifndef _H_COMPONENT_
#define _H_COMPONENT_

#include <iostream>
#include <string>

// function macro for stream out operator <<
#ifndef NETLIST_STREAMOUT
#define NETLIST_STREAMOUT(COMP)                                          \
  inline std::ostream& operator<< ( std::ostream& os, const COMP& rhs) { \
    return rhs.streamout(os);                                            \
  }
#endif

#ifndef NETLIST_DEFAULT_CON
#define NETLIST_DEFAULT_CON(COMP, Father, CT) COMP() : Father(CT) { }
#endif

namespace netlist {

#include "comp_type.h"

  // the base class of all netlist components
  class NetComp {
  public:
    // no one should directly use this class
    NetComp() : ctype(tUnkown) {}
    NetComp(ctype_t tt) : ctype(tt) {}

    // force every component to implement a stream out function
    virtual std::ostream& streamout(std::ostream&) const = 0;

    ctype_t ctype;
  };
  
  // number.h
  class Number;			/* number */

  // identifier.h
  class Identifier;		/* identifier base class */
  class BIdentifier;		/* block name */
  class FIdentifier;		/* function name */
  class MIdentifier;		/* module name */
  class IIdentifier;		/* instance name */
  class PaIdentifier;		/* parameter name */
  class PoIdentifier;		/* port identifier */
  class VIdentifier;		/* variable name, including local variable, wire and reg */

  // range.h
  class Range;			/* range expression */

  // expression.h
  class Expression;		/* expressions */

}

#include "defines.h"
#include "number.h"
#include "identifier.h"
#include "range.h"
#include "expression.h"

#endif
