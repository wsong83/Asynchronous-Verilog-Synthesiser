/*
 * Copyright (c) 2011-2013 Wei Song <songw@cs.man.ac.uk> 
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

#ifndef AV_H_COMPONENT_
#define AV_H_COMPONENT_

#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <utility>
#include <boost/shared_ptr.hpp>
#include <cassert>

namespace netlist {

  // netcomp.h
  class NetComp;

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
  //class PoIdentifier;		/* port identifier */
  class VIdentifier;		/* variable name, including local variable, wire and reg */

  // concatenation.h
  class Concatenation;
  
  // lconcatenation.h
  class LConcatenation;

  // range.h
  class Range;			/* range expression */

  // array of range expressions
  class RangeArray;

  // expression.h
  class Expression;		/* expressions */

  // port.h
  class Port;

  // variable.h
  class Variable;

  // assign.h
  class Assign;

  // case.h
  class CaseState;
  class CaseItem;

  // if.h
  class IfState;

  // while.h
  class WhileState;

  // for.h
  class ForState;

  // block.h
  class Block;

  // module.h
  class Module;
  
  // instance.h
  class Instance;

  // portconn.h
  class PortConn;               /* port connection */
  class ParaConn;               /* parameter connection */

  // genblcok.h
  class GenBlock;

  // seqblock.h
  class SeqBlock;

  // library.h
  class Library;

  // uni_name.h
  class UniName;

}

#include "defines.h"

// root class
#include "netcomp.h"

// STL classes
#include "database.h"

// NON-STL classes
#include "operation.h"
#include "number.h"
#include "expression.h"
#include "concatenation.h"
#include "lconcatenation.h"
#include "range.h"
#include "range_array.h"
#include "identifier.h"
#include "port.h"
#include "variable.h"
#include "assign.h"
#include "case.h"
#include "if.h"
#include "while.h"
#include "for.h"
#include "block.h"
#include "module.h"
#include "instance.h"
#include "portconn.h"
#include "genblock.h"
#include "seqblock.h"
#include "library.h"
#include "uni_name.h"

// the environment
namespace shell {
  class Env;
}

extern shell::Env* G_ENV;

// SDFG
namespace SDFG {
  class dfgGraph;
}


#include <sstream>

// objects to string templates
template <typename T>
inline std::string toString (const T& obj) {
  std::ostringstream sos;
  sos << obj;
  return sos.str();
}

// blocks: Block, SeqBlock, GenBlock, Module
template <typename T>
inline std::string toString (const boost::shared_ptr<T>& obj) {
  return obj->name.name;
}


#endif

// Local Variables:
// mode: c++
// End:
