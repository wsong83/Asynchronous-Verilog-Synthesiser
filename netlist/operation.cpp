/*
 * Copyright (c) 2012 Wei Song <songw@cs.man.ac.uk> 
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
 * 11/02/2012   Wei Song
 *
 *
 */

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

netlist::Operation::Operation(shared_ptr<Number> num)
  : otype(oNum), valuable(num->is_valuable()), data(num)
{ }

netlist::Operation::Operation(shared_ptr<Identifier> id)
  : otype(oVar), valuable(false), data(id)
{ }

netlist::Operation::Operation(shared_ptr<Concatenation> con)
  : otype(oCon), valuable(false), data(con)
{ }

Number& netlist::Operation::get_num_ref(){
  assert(otype == oNum);
  return *(static_pointer_cast<Number>(data));
}

Number netlist::Operation::get_num() const{
  assert(otype == oNum);
  return *(static_pointer_cast<Number>(data));
}

shared_ptr<Concatenation> netlist::Operation::get_con() {
  assert(otype == oCon);
  return static_pointer_cast<Concatenation>(data);
}

shared_ptr<Identifier> netlist::Operation::get_var(){
  assert(otype == oVar);
  return static_pointer_cast<Identifier>(data);
}

Operation netlist::Operation::deep_copy() const{
  switch(otype) {
  case oNum: {
    return Operation(shared_ptr<Number>(new Number(get_num())));
  }
  case oVar: {
    if(data->get_type() == NetComp::tVarName) {
      return Operation((static_pointer_cast<VIdentifier>(data))->deep_copy());
    } else {
      assert(0 == "Currently impossible to be other types");
      return Operation();
    }
  }
  case oCon: {
    return Operation(static_pointer_cast<Concatenation>(data)->deep_copy());
  }
  default:
    return Operation(otype);
  }
}

ostream& netlist::Operation::streamout(ostream& os) const {
  switch(otype) {
  case oNULL:                          return os;
  case oNum:        os << *(static_pointer_cast<Number>(data)); return os;
  case oVar:        os << *(static_pointer_cast<Identifier>(data)); return os;
  case oCon:        os << *(static_pointer_cast<Concatenation>(data)); return os;
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

void netlist::Operation::reduce() {
  if(data.use_count() != 0)
    data->reduce();
}

// dummy yet
void netlist::execute_operation( Operation::operation_t op,
                                 list<Operation>& d1,
                                 list<Operation>& d2,
                                 list<Operation>& d3
                                 ) {
  // check parameters
  assert(op >= Operation::oUPos);
  assert(!d1.empty());
  if(op >= Operation::oPower) assert(!d2.empty());
  if(op >= Operation::oQuestion) assert(!d3.empty());

  // will use d1 as return
  switch(op) {
  case Operation::oUPos:     execute_UPos(d1); return;
  case Operation::oUNeg:     execute_UNeg(d1); return;
  case Operation::oULRev:    break;
  case Operation::oURev:     break;
  case Operation::oUAnd:     break;
  case Operation::oUNand:    break;
  case Operation::oUOr:      break;
  case Operation::oUNor:     break;
  case Operation::oUXor:     break;
  case Operation::oUNxor:    break;
  case Operation::oPower:    break;
  case Operation::oTime:     break;
  case Operation::oDiv:      break;
  case Operation::oMode:     break;
  case Operation::oAdd:      execute_Add(d1, d2); return;
  case Operation::oMinus:    break;
  case Operation::oRS:       break;
  case Operation::oLS:       break;
  case Operation::oLRS:      break;
  case Operation::oLess:     break;
  case Operation::oLe:       break;
  case Operation::oGreat:    break;
  case Operation::oGe:       break;
  case Operation::oEq:       break;
  case Operation::oNeq:      break;
  case Operation::oCEq:      break;
  case Operation::oCNeq:     break;
  case Operation::oAnd:      break;
  case Operation::oXor:      break;
  case Operation::oNxor:     break;
  case Operation::oOr:       break;
  case Operation::oLAnd:     break;
  case Operation::oLOr:      break;
  case Operation::oQuestion: break;
  default:  // should not run to here
    assert(1 == 0);
  }
  assert(1 == 0);
  return;
}

// unary +
void netlist::execute_UPos(list<Operation>& d1) {
  // for unary +, do nothing but omit the operator
  return;
}

// unary -
void netlist::execute_UNeg(list<Operation>& d1) {
  if(d1.front().is_valuable()) {
    assert(d1.front().get_type() == Operation::oNum);
    d1.front().get_num().negate();
  } else if(d1.front().get_type() == Operation::oUNeg) {
    // continueous unary negate
    d1.pop_front();
  } else {
    d1.push_front(Operation(Operation::oUNeg));
  }
  return;
}
  
// +
void netlist::execute_Add(list<Operation>& d1, list<Operation>& d2) {
  if(d1.front().is_valuable() && d2.front().is_valuable()) {
    d1.front().get_num_ref() = d1.front().get_num() + d2.front().get_num();
  } else {
    d1.push_front(Operation(Operation::oAdd));
    d1.splice(d1.end(), d2);
  }
  return;
}