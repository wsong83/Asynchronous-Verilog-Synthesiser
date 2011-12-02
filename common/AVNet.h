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
 * The base class definitions of asynchronous verilog netlist
 * 01/12/2011   Wei Song
 *
 *
 */

#ifndef _AVNET_H_
#define _AVNET_H_

#include<utility>
#include<ostream>

//////////////////////////////////////////////////////////////////////////
// The type of all nodes in netlist
class AVNetType {
 public:
  enum en {			/* type definitions */
    AVNT_always,
    AVNT_assign,
    AVNT_BASE,			/* base type, identify the base class for all nodes */
    AVNT_ConstExp,		/* const numerical expression, always evaluated after elabration */
    AVNT_function,
    AVNT_module,
    AVNT_reg,
    AVNT_wire,
    AVNT_unkown			/* must be the last */
  };

  en my_type;			/* store the node type */

  const char* operator() () const { /* return the ascii of my type */
    static const char* names[] = {
    "always",
    "assign",
    "BASE CLASS",	/* base type */
    "",			/* const numerical expression, no type name */
    "function",
    "module",
    "reg",
    "wire",
    "unkown"			/* must be the last */
    };
    return names[my_type];
  }

  // constructors
  AVNetType () : my_type(AVNT_unkown) {} /* uninitialied type are set to unkown type */
  AVNetType ( const AVNetType& tt ) : my_type(tt.my_type) {} /* copy constructor */
  AVNetType (en tt) : my_type(tt) {}
  AVNetType& operator= (const en& tt) { my_type = tt; }
  AVNetType& operator= (const AVNetType& tt) {my_type = tt.my_type;}
};

// standard output operator
std::ostream& operator<< (std::ostream& os, AVNetType rhs) { return os<<rhs(); }

//////////////////////////////////////////////////////////////////////////
// The base class for all AVNet nodes
class AVNetBase {
 public:
  AVNetType type;		/* the actual type of this node */
  std::string name;			/* the name of this node */
  
  // constructors
  AVNetBase () : type(AVNetType::AVNT_BASE), name("unkown") {} /* AVNetBase is not suppose to be built explicitly */
  AVNetBase ( const AVNetBase& bb ) : type(bb.type), name(bb.name) {} /* copy constructor */
  AVNetBase& operator= ( const AVNetBase& bb ) {type = bb.type; name = bb.name;}
  
  // member functions

};

// standard output operator
std::ostream& operator<< (std::ostream& os, AVNetBase rhs) { os << rhs.type << " " << rhs.name; return os; }

  
//////////////////////////////////////////////////////////////////////////
// Const expression (calculatations using macros and parameters, appear in range, if condition, port declaration, etc)
class AVNetConstExp : public AVNetBase {

};

//////////////////////////////////////////////////////////////////////////
// wire
class AVNetWire : public AVNetBase {
 public:
  
};

#endif

