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
 * the operation in expression
 * 11/02/2011   Wei Song
 *
 *
 */

#include <cassert>
#include "component.h"

using namespace netlist;

netlist::Operation::Operation()
  : otype(oNULL), valuable(false)
{}

netlist::Operation::Operation(operation_t otype)
  : otype(otype), valuable(false)
{
  // only operators do not need an operand
  assert(otype > oFun);
}

netlist::Operation::Operation(boost::shared_ptr<Number> num)
  : otype(oNum), valuable(num->is_valuable()), data(num)
{ }

netlist::Operation::Operation(boost::shared_ptr<Identifier> id)
  : otype(oVar), valuable(false), data(id)
{ }

// dummy yet
void netlist::Operation::execute(
                                 const Operation& d1,
                                 const Operation& d2,
                                 const Operation& d3
                                 ) {
}

boost::shared_ptr<Number> netlist::Operation::get_num() const {
  assert(otype == oNum);
  return boost::static_pointer_cast<Number>(data);
}

boost::shared_ptr<Identifier> netlist::Operation::get_var() const {
  assert(otype == oVar);
  return boost::static_pointer_cast<Identifier>(data);
}

std::ostream& netlist::Operation::streamout(std::ostream& os) const {
  switch(otype) {
  case oNULL:                          return os;
  case oNum:        os << *(boost::static_pointer_cast<Number>(data)); return os;
  case oVar:        os << *(boost::static_pointer_cast<Identifier>(data)); return os;
  case oCon: // dummy
  case oFun: // dummy
  case oUPos:
  case oAdd:        os << "+";         return os;
  case oUNeg:
  case oMinus:      os << "-";         return os;
  case oULRev:      os << "!";         return os;
  case oURev:       os << "~";         return os;
  case oUAnd:
  case oAnd:        os << "&";         return os;
  case oUNand:      os << "~&";        return os;
  case oUOr:
  case oOr:         os << "|";         return os;
  case oUNor:       os << "~|";        return os;
  case oUXor:
  case oXor:        os << "^";         return os;
  case oUNxor:
  case oNxor:       os << "~^";        return os;
  case oPower:      os << "**";        return os;
  case oTime:       os << "*";         return os;
  case oDiv:        os << "/";         return os;
  case oMode:       os << "%";         return os;
  case oRS:         os << ">>";        return os;
  case oLS:         os << "<<";        return os;
  case oLRS:        os << ">>>";       return os;
  case oLess:       os << "<";         return os;
  case oLe:         os << "<=";        return os;
  case oGreat:      os << ">";         return os;
  case oGe:         os << ">=";        return os;
  case oEq:         os << "==";        return os;
  case oNeq:        os << "!=";        return os;
  case oCEq:        os << "===";       return os;
  case oCNeq:       os << "!==";       return os;
  case oLAnd:       os << "&&";        return os;
  case oLOr:        os << "||";        return os;
  case oQuestion:   os << "?";         return os;
  };
  // should not run to here
  assert(1==0);
  return os;
}

