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
 * Definition of netlist components.
 * 01/02/2012   Wei Song
 *
 *
 */

#ifndef _H_COMPONENT_
#define _H_COMPONENT_

#include <iostream>
using std::cout;
using std::endl;
using std::ostream;

#include <string>
using std::string;

#include <list>
using std::list;

#include <vector>
using std::vector;

#include <stack>
using std::stack;

#include <map>
using std::map;

#include <utility>
using std::pair;

#include <boost/shared_ptr.hpp>
using boost::shared_ptr;
using boost::weak_ptr;
using boost::static_pointer_cast;

#include <cassert>


// function macro for stream out operator <<
#ifndef NETLIST_STREAMOUT
#define NETLIST_STREAMOUT(COMP)                                          \
  inline ostream& operator<< ( ostream& os, const COMP& rhs) { \
    return rhs.streamout(os);                                            \
  }
#endif

#ifndef NETLIST_DEFAULT_CON
#define NETLIST_DEFAULT_CON(COMP, CT) COMP() : NetComp(NetComp::CT) { }
#endif

namespace netlist {

  // the base class of all netlist components
  class NetComp {
  public:
#include "comp_type.h"
    // no one should directly use this class
    NetComp() : ctype(tUnkown) {}
    NetComp(ctype_t tt) : ctype(tt) {}
    
    ctype_t get_type() const { return ctype; }
    ctype_t ctype;

    virtual void reduce() {}	/* many netlist component need method to reduce itself */
  };

  // operation.h
  class Operation;

  // number.h
  class Number;			/* number */

  // identifier.h
  class Identifier;		/* identifier base class */
  class BIdentifier;		/* block name */
  class FIdentifier;		/* function name */
  class MIdentifier;		/* module name */
  class IIdentifier;		/* instance name */
  class PoIdentifier;		/* port identifier */
  class VIdentifier;		/* variable name, including local variable, wire and reg */

  // concatenation.h
  class Concatenation;
  
  // lconcatenation.h
  class LConcatenation;

  // range.h
  class Range;			/* range expression */

  // expression.h
  class Expression;		/* expressions */

  // port.h
  class Port;

  // variable.h
  class Variable;

  // assign.h
  class Assign;

  // module.h
  class Module;
  
  // instance.h
  class Instance;

  // portconn.h
  class PortConn;               /* port connection */
  class ParaConn;               /* parameter connection */

  // blcok.h
  class GenBlock;
  class SeqBlock;

  // case.h
  class CaseState;
  class CaseItem;

  // if.h
  class IfState;

  // while.h
  class WhileState;

  // library.h
  class Library;

  // uni_name.h
  class UniName;

}

#include "defines.h"

// STL classes
#include "database.h"

// NON-STL classes
#include "operation.h"
#include "number.h"
#include "identifier.h"
#include "expression.h"
#include "concatenation.h"
#include "lconcatenation.h"
#include "range.h"
#include "port.h"
#include "variable.h"
#include "assign.h"
#include "module.h"
#include "instance.h"
#include "portconn.h"
#include "block.h"
#include "library.h"
#include "uni_name.h"

#endif
