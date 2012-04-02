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

netlist::Operation::Operation(const Number& num)
  : otype(oNum), valuable(num.is_valuable()), data(new Number(num))
{ }

netlist::Operation::Operation(const VIdentifier& id)
  : otype(oVar), valuable(false), data(new VIdentifier(id))
{ }

netlist::Operation::Operation(const shared_ptr<Concatenation>& con)
  : otype(oCon), valuable(false), data(static_pointer_cast<NetComp>(con))
{ }

netlist::Operation::Operation(const shared_ptr<LConcatenation>& con)
  : otype(oCon), valuable(false) {
  if(con->size() == 1) {
    otype = oVar;
    data.reset(new VIdentifier(con->front()));
  } else {
    // copy all elements in LConcatenation to Concatenation
    shared_ptr<Concatenation> cp(new Concatenation());
    list<VIdentifier>::const_iterator it, end;
    for(it=con->data.begin(), end=con->data.end(); it!=end; it++) {
      shared_ptr<ConElem> m( new ConElem(shared_ptr<Expression>(new Expression(*it))));
      *cp + m;
    }
  }
}

Number& netlist::Operation::get_num(){
  assert(otype == oNum);
  return *(static_pointer_cast<Number>(data));
}

Number netlist::Operation::get_num() const{
  assert(otype == oNum);
  return *(static_pointer_cast<Number>(data));
}

Concatenation& netlist::Operation::get_con(){
  assert(otype == oCon);
  return *(static_pointer_cast<Concatenation>(data));
}

VIdentifier& netlist::Operation::get_var(){
  assert(otype == oVar);
  return *(static_pointer_cast<VIdentifier>(data));
}

void netlist::Operation::db_register(int iod) {
  if(data.use_count()) {
    switch(data->get_type()) {
    case NetComp::tVarName: {
      shared_ptr<VIdentifier> vp = static_pointer_cast<VIdentifier>(data);
      if(vp->db_registered()) {
        assert(vp->get_inout_dir() == iod);
      } else {
        vp->db_register(iod);
      }
      break;
    }
    case NetComp::tConcatenation: {
      (static_pointer_cast<Concatenation>(data))->db_register(iod);
      break;
    }
    default:;                   // do nothing
    }
  }
}

void netlist::Operation::db_expunge() {
  if(data.use_count()) {
    switch(data->get_type()) {
    case NetComp::tVarName: {
      shared_ptr<VIdentifier> vp = static_pointer_cast<VIdentifier>(data);
      vp->db_expunge();
      break;
    }
    case NetComp::tConcatenation: {
      (static_pointer_cast<Concatenation>(data))->db_expunge();
      break;
    }
    default:;                   // do nothing
    }
  }
}

ostream& netlist::Operation::streamout(ostream& os, unsigned int indent) const {
  os << string(indent, ' ');
  switch(otype) {
  case oNULL:                          return os;
  case oNum:
  case oVar:
  case oCon:        os << *data;       return os;
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

Operation* netlist::Operation::deep_copy() const {
  Operation* rv = new Operation();
  rv->otype = this->otype;
  rv->valuable = this->valuable;
  if(data.use_count() != 0) rv->data = shared_ptr<NetComp>(data->deep_copy());
  return rv;
}

void netlist::Operation::reduce() {
  if(data.use_count() != 0)
    data->reduce();

  // change to simpler types of operation if possible
  switch(otype) {
  case oCon: {
    Concatenation& mcon = *(static_pointer_cast<Concatenation>(data));
    if(mcon.data.size() == 1 &&
       mcon.data.front()->exp->is_valuable() &&
       mcon.data.front()->con.size() == 0
       ) {
      data.reset(new Number(mcon.data.front()->exp->get_value()));
      otype = oNum;
      valuable = true;
    }
    break;
  }
  default:;
  }
}

// dummy yet
void netlist::execute_operation( Operation::operation_t op,
                                 list<shared_ptr<Operation> >& d1,
                                 list<shared_ptr<Operation> >& d2,
                                 list<shared_ptr<Operation> >& d3
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
  case Operation::oULRev:    execute_ULRev(d1); return;
  case Operation::oURev:     execute_URev(d1); return;
  case Operation::oUAnd:     execute_UAnd(d1); return;
  case Operation::oUNand:    execute_UNand(d1); return;
  case Operation::oUOr:      execute_UOr(d1); return;
  case Operation::oUNor:     execute_UNor(d1); return;
  case Operation::oUXor:     execute_UXor(d1); return;
  case Operation::oUNxor:    execute_UNxor(d1); return;
  case Operation::oPower:    execute_Power(d1, d2); return;
  case Operation::oTime:     execute_Time(d1,d2); return;
  case Operation::oDiv:      execute_Div(d1,d2); return;
  case Operation::oMode:     execute_Mode(d1,d2); return;
  case Operation::oAdd:      execute_Add(d1, d2); return;
  case Operation::oMinus:    execute_Minus(d1, d2); return;
  case Operation::oRS:       execute_RS(d1,d2); return;
  case Operation::oLS:       execute_LS(d1,d2); return;
  case Operation::oLRS:      execute_LRS(d1,d2); return;
  case Operation::oLess:     execute_Less(d1,d2); return;
  case Operation::oLe:       execute_Le(d1,d2); return;
  case Operation::oGreat:    execute_Great(d1,d2); return;
  case Operation::oGe:       execute_Ge(d1,d2); return;
  case Operation::oEq:       execute_Eq(d1,d2); return;
  case Operation::oNeq:      execute_Neq(d1,d2); return;
  case Operation::oCEq:      execute_CEq(d1,d2); return;
  case Operation::oCNeq:     execute_CNeq(d1,d2); return;
  case Operation::oAnd:      execute_And(d1,d2); return;
  case Operation::oXor:      execute_Xor(d1,d2); return;
  case Operation::oNxor:     execute_Nxor(d1,d2); return;
  case Operation::oOr:       execute_Or(d1,d2); return;
  case Operation::oLAnd:     execute_LAnd(d1,d2); return;
  case Operation::oLOr:      execute_LOr(d1,d2); return;
  case Operation::oQuestion: execute_Question(d1,d2,d3); return;
  default:  // should not run to here
    assert(0 == "wrong operation type");
  }
  return;
}

// unary +
void netlist::execute_UPos(list<shared_ptr<Operation> >& d1) {
  // for unary +, do nothing but omit the operator
  return;
}

// unary -
void netlist::execute_UNeg(list<shared_ptr<Operation> >& d1) {
  if(d1.front()->is_valuable()) {
    assert(d1.front()->get_type() == Operation::oNum);
    d1.front()->get_num().negate();
  } else {
    d1.push_front(shared_ptr<Operation>(new Operation(Operation::oUNeg)));
  }
}

// unary !
void netlist::execute_ULRev(list<shared_ptr<Operation> >& d1) {
  if(d1.size() == 1 && d1.front()->get_type() == Operation::oNum) {
    string tval = d1.front()->get_num().get_txt_value();
    bool one = false;
    bool x = false;
    for(unsigned int i=0; i<tval.size(); i++) {
      one = tval[i] == '1' ? true : one;
      x = (tval[i] == 'x' || tval[i] == 'z') ? true : x;
    }
    if(one)
      d1.front()->get_num() = Number(0);
    else if(x)
      d1.front()->get_num() = Number("x");
    else
      d1.front()->get_num() = Number(1);
  } else {
    d1.push_front(shared_ptr<Operation>(new Operation(Operation::oULRev)));
  }
}

// unary ~
void netlist::execute_URev(list<shared_ptr<Operation> >& d1) {
  if(d1.size() == 1 && d1.front()->get_type() == Operation::oNum) {
    string tval = d1.front()->get_num().get_txt_value();
    for(unsigned int i=0; i<tval.size(); i++) {
      switch(tval[i]) {
      case '0': tval[i] = '1'; break;
      case '1': tval[i] = '0'; break;
      case 'x':
      case 'z': tval[i] = 'x'; break;
      default: assert(0 == "unkown value");
      } 
    }
    d1.front()->get_num() = Number(tval);
  } else {
    d1.push_front(shared_ptr<Operation>(new Operation(Operation::oURev)));
  }
}

// unary &
void netlist::execute_UAnd(list<shared_ptr<Operation> >& d1) {
  if(d1.size() == 1 && d1.front()->get_type() == Operation::oNum) {
    string tval = d1.front()->get_num().get_txt_value();
    char m = '1';
    for(unsigned int i=0; i<tval.size(); i++) {
      switch(tval[i]) {
      case '0': m = '0'; break;
      case '1': break;
      case 'x':
      case 'z': if (m == '1') m = 'x'; break;
      default: assert(0 == "unkown value");
      } 
    }
    d1.front()->get_num() = Number(string(1, m));
  } else {
    d1.push_front(shared_ptr<Operation>(new Operation(Operation::oUAnd)));
  }
}

// unary ~&
void netlist::execute_UNand(list<shared_ptr<Operation> >& d1) {
  if(d1.size() == 1 && d1.front()->get_type() == Operation::oNum) {
    string tval = d1.front()->get_num().get_txt_value();
    char m = '1';
    for(unsigned int i=0; i<tval.size(); i++) {
      switch(tval[i]) {
      case '0': m = '0'; break;
      case '1': break;
      case 'x':
      case 'z': if(m == '1') m = 'x'; break;
      default: assert(0 == "unkown value");
      } 
    }
    if(m == '0') m = '1'; else if(m == '1') m = '0';
    d1.front()->get_num() = Number(string(1, m));
  } else {
    d1.push_front( shared_ptr<Operation>(new Operation(Operation::oUNand)));
  }
}

// unary |
void netlist::execute_UOr(list<shared_ptr<Operation> >& d1) {
  if(d1.size() == 1 && d1.front()->get_type() == Operation::oNum) {
    string tval = d1.front()->get_num().get_txt_value();
    char m = '0';
    for(unsigned int i=0; i<tval.size(); i++) {
      switch(tval[i]) {
      case '0': break;
      case '1': m = '1'; break;
      case 'x':
      case 'z': if(m == '0') m = 'x'; break;
      default: assert(0 == "unkown value");
      } 
    }
    d1.front()->get_num() = Number(string(1, m));
  } else {
    d1.push_front(shared_ptr<Operation>( new Operation(Operation::oUOr)));
  }
}

// unary ~|
void netlist::execute_UNor(list<shared_ptr<Operation> >& d1) {
  if(d1.size() == 1 && d1.front()->get_type() == Operation::oNum) {
    string tval = d1.front()->get_num().get_txt_value();
    char m = '0';
    for(unsigned int i=0; i<tval.size(); i++) {
      switch(tval[i]) {
      case '0': break;
      case '1': m = '1'; break;
      case 'x':
      case 'z': if(m == '0') m = 'x'; break;
      default: assert(0 == "unkown value");
      } 
    }
    if(m == '0') m = '1'; else if(m == '1') m = '0';
    d1.front()->get_num() = Number(string(1, m));
  } else {
    d1.push_front( shared_ptr<Operation>(new Operation(Operation::oUNor)));
  }
}

// unary ^
void netlist::execute_UXor(list<shared_ptr<Operation> >& d1) {
  if(d1.size() == 1 && d1.front()->get_type() == Operation::oNum) {
    string tval = d1.front()->get_num().get_txt_value();
    char m = tval[0];
    for(unsigned int i=1; i<tval.size(); i++) {
      switch(tval[i]) {
      case '0': if(m == '0') m = '0'; else if(m == '1') m = '1'; break;
      case '1': if(m == '0') m = '1'; else if(m == '1') m = '0'; break;
      case 'x':
      case 'z': m = 'x'; break;
      default: assert(0 == "unkown value");
      } 
    }
    d1.front()->get_num() = Number(string(1, m));
  } else {
    d1.push_front( shared_ptr<Operation>(new Operation(Operation::oUXor)));
  }
}

// unary ~^
void netlist::execute_UNxor(list<shared_ptr<Operation> >& d1) {
  if(d1.size() == 1 && d1.front()->get_type() == Operation::oNum) {
    string tval = d1.front()->get_num().get_txt_value();
    char m = tval[0];
    for(unsigned int i=1; i<tval.size(); i++) {
      switch(tval[i]) {
      case '0': if(m == '0') m = '0'; else if(m == '1') m = '1'; break;
      case '1': if(m == '0') m = '1'; else if(m == '1') m = '0'; break;
      case 'x':
      case 'z': m = 'x'; break;
      default: assert(0 == "unkown value");
      } 
    }
    if(m == '0') m = '1'; else if(m == '1') m = '0';
    d1.front()->get_num() = Number(string(1, m));
  } else {
    d1.push_front( shared_ptr<Operation>(new Operation(Operation::oUNxor)));
  }
}
  
// **
void netlist::execute_Power(list<shared_ptr<Operation> >& d1, list<shared_ptr<Operation> >& d2) {
  if(d1.front()->is_valuable() && d2.front()->is_valuable()) {
    mpz_class res;
    mpz_pow_ui(res.get_mpz_t(), d1.front()->get_num().get_value().get_mpz_t(), d2.front()->get_num().get_value().get_ui());
    d1.front()->get_num() = mpz_class(res);
  } else {
    d1.push_front( shared_ptr<Operation>(new Operation(Operation::oPower)));
    d1.splice(d1.end(), d2);
  }
}

// *
void netlist::execute_Time(list<shared_ptr<Operation> >& d1, list<shared_ptr<Operation> >& d2) {
  if(d1.front()->is_valuable() && d2.front()->is_valuable()) {
    mpz_class res = d1.front()->get_num().get_value() * d2.front()->get_num().get_value();
    d1.front()->get_num() = res;
  } else {
    d1.push_front( shared_ptr<Operation>(new Operation(Operation::oTime)));
    d1.splice(d1.end(), d2);
  }
}

// /
void netlist::execute_Div(list<shared_ptr<Operation> >& d1, list<shared_ptr<Operation> >& d2) {
  if(d1.front()->is_valuable() && d2.front()->is_valuable()) {
    mpz_class res = d1.front()->get_num().get_value() / d2.front()->get_num().get_value();
    d1.front()->get_num() = res;
  } else {
    d1.push_front( shared_ptr<Operation>(new Operation(Operation::oDiv)));
    d1.splice(d1.end(), d2);
  }
}

// %
void netlist::execute_Mode(list<shared_ptr<Operation> >& d1, list<shared_ptr<Operation> >& d2) {
  if(d1.front()->is_valuable() && d2.front()->is_valuable()) {
    mpz_class res = d1.front()->get_num().get_value() % d2.front()->get_num().get_value();
    d1.front()->get_num() = res;
  } else {
    d1.push_front( shared_ptr<Operation>(new Operation(Operation::oMode)));
    d1.splice(d1.end(), d2);
  }
}


// +
void netlist::execute_Add(list<shared_ptr<Operation> >& d1, list<shared_ptr<Operation> >& d2) {
  if(d1.front()->is_valuable() && d2.front()->is_valuable()) {
    mpz_class res = d1.front()->get_num().get_value() + d2.front()->get_num().get_value();
    d1.front()->get_num() = res;
  } else {
    d1.push_front( shared_ptr<Operation>(new Operation(Operation::oAdd)));
    d1.splice(d1.end(), d2);
  }
}

// -
void netlist::execute_Minus(list<shared_ptr<Operation> >& d1, list<shared_ptr<Operation> >& d2) {
  if(d1.front()->is_valuable() && d2.front()->is_valuable()) {
    mpz_class res = d1.front()->get_num().get_value() - d2.front()->get_num().get_value();
    d1.front()->get_num() = res;
  } else {
    d1.push_front( shared_ptr<Operation>(new Operation(Operation::oMinus)));
    d1.splice(d1.end(), d2);
  }
}

// >>
void netlist::execute_RS(list<shared_ptr<Operation> >& d1, list<shared_ptr<Operation> >& d2) {
  if(d1.front()->get_type() == Operation::oNum && d2.front()->is_valuable()) {
    string tval = d1.front()->get_num().get_txt_value();
    tval.erase(tval.size() - d2.front()->get_num().get_value().get_ui());
    d1.front()->get_num() = Number(tval);
  } else {
    d1.push_front( shared_ptr<Operation>(new Operation(Operation::oRS)));
    d1.splice(d1.end(), d2);
  }
}

// <<
void netlist::execute_LS(list<shared_ptr<Operation> >& d1, list<shared_ptr<Operation> >& d2) {
  if(d1.front()->get_type() == Operation::oNum && d2.front()->is_valuable()) {
    string tval = d1.front()->get_num().get_txt_value();
    tval.insert(tval.end(), d2.front()->get_num().get_value().get_ui(), '0');
    d1.front()->get_num() = Number(tval);
  } else {
    d1.push_front( shared_ptr<Operation>(new Operation(Operation::oLS)));
    d1.splice(d1.end(), d2);
  }
}

// >>>, currently equivalent to >>, signed number not supported yet
void netlist::execute_LRS(list<shared_ptr<Operation> >& d1, list<shared_ptr<Operation> >& d2) {
  if(d1.front()->get_type() == Operation::oNum && d2.front()->is_valuable()) {
    string tval = d1.front()->get_num().get_txt_value();
    tval.erase(tval.size() - d2.front()->get_num().get_value().get_ui());
    d1.front()->get_num() = Number(tval);
  } else {
    d1.push_front( shared_ptr<Operation>(new Operation(Operation::oLRS)));
    d1.splice(d1.end(), d2);
  }
}

// <
void netlist::execute_Less(list<shared_ptr<Operation> >& d1, list<shared_ptr<Operation> >& d2) {
  if(d1.front()->get_type() == Operation::oNum && d2.front()->get_type() == Operation::oNum) {
    string tval1 = d1.front()->get_num().get_txt_value();
    string tval2 = d2.front()->get_num().get_txt_value();
    if(tval1.size() > tval2.size())
      tval2.insert(tval2.begin(), tval1.size() - tval2.size(), '0');
    else if(tval1.size() < tval2.size())
      tval1.insert(tval1.begin(), tval2.size() - tval1.size(), '0');

    for(unsigned int i=0; i<tval1.size(); i++) {
      if(tval1[i] == '1' && tval2[i] == '0') {
        d1.front()->get_num() = Number("0");
        return;
      } else if(tval1[i] == '0' && tval2[i] == '1') {
        d1.front()->get_num() = Number("1");
        return;
      } else if(tval1[i] == 'x' || tval2[i] == 'x' || tval1[i] == 'z' || tval2[i] == 'z' ) {
        d1.front()->get_num() = Number("x");
        return;
      }
    }
    // equal
    d1.front()->get_num() = Number("0");
  } else {
    d1.push_front( shared_ptr<Operation>(new Operation(Operation::oLess)));
    d1.splice(d1.end(), d2);
  }
}

// <=
void netlist::execute_Le(list<shared_ptr<Operation> >& d1, list<shared_ptr<Operation> >& d2) {
  if(d1.front()->get_type() == Operation::oNum && d2.front()->get_type() == Operation::oNum) {
    string tval1 = d1.front()->get_num().get_txt_value();
    string tval2 = d2.front()->get_num().get_txt_value();
    if(tval1.size() > tval2.size())
      tval2.insert(tval2.begin(), tval1.size() - tval2.size(), '0');
    else if(tval1.size() < tval2.size())
      tval1.insert(tval1.begin(), tval2.size() - tval1.size(), '0');

    for(unsigned int i=0; i<tval1.size(); i++) {
      if(tval1[i] == '1' && tval2[i] == '0') {
        d1.front()->get_num() = Number("0");
        return;
      } else if(tval1[i] == '0' && tval2[i] == '1') {
        d1.front()->get_num() = Number("1");
        return;
      } else if(tval1[i] == 'x' || tval2[i] == 'x' || tval1[i] == 'z' || tval2[i] == 'z' ) {
        d1.front()->get_num() = Number("x");
        return;
      }
    }
    // equal
    d1.front()->get_num() = Number("1");
  } else {
    d1.push_front( shared_ptr<Operation>(new Operation(Operation::oLe)));
    d1.splice(d1.end(), d2);
  }
}

// >
void netlist::execute_Great(list<shared_ptr<Operation> >& d1, list<shared_ptr<Operation> >& d2) {
  if(d1.front()->get_type() == Operation::oNum && d2.front()->get_type() == Operation::oNum) {
    string tval1 = d1.front()->get_num().get_txt_value();
    string tval2 = d2.front()->get_num().get_txt_value();
    if(tval1.size() > tval2.size())
      tval2.insert(tval2.begin(), tval1.size() - tval2.size(), '0');
    else if(tval1.size() < tval2.size())
      tval1.insert(tval1.begin(), tval2.size() - tval1.size(), '0');

    for(unsigned int i=0; i<tval1.size(); i++) {
      if(tval1[i] == '1' && tval2[i] == '0') {
        d1.front()->get_num() = Number("1");
        return;
      } else if(tval1[i] == '0' && tval2[i] == '1') {
        d1.front()->get_num() = Number("0");
        return;
      } else if(tval1[i] == 'x' || tval2[i] == 'x' || tval1[i] == 'z' || tval2[i] == 'z' ) {
        d1.front()->get_num() = Number("x");
        return;
      }
    }
    // equal
    d1.front()->get_num() = Number("0");
  } else {
    d1.push_front( shared_ptr<Operation>(new Operation(Operation::oGreat)));
    d1.splice(d1.end(), d2);
  }
}

// >=
void netlist::execute_Ge(list<shared_ptr<Operation> >& d1, list<shared_ptr<Operation> >& d2) {
  if(d1.front()->get_type() == Operation::oNum && d2.front()->get_type() == Operation::oNum) {
    string tval1 = d1.front()->get_num().get_txt_value();
    string tval2 = d2.front()->get_num().get_txt_value();
    if(tval1.size() > tval2.size())
      tval2.insert(tval2.begin(), tval1.size() - tval2.size(), '0');
    else if(tval1.size() < tval2.size())
      tval1.insert(tval1.begin(), tval2.size() - tval1.size(), '0');

    for(unsigned int i=0; i<tval1.size(); i++) {
      if(tval1[i] == '1' && tval2[i] == '0') {
        d1.front()->get_num() = Number("1");
        return;
      } else if(tval1[i] == '0' && tval2[i] == '1') {
        d1.front()->get_num() = Number("0");
        return;
      } else if(tval1[i] == 'x' || tval2[i] == 'x' || tval1[i] == 'z' || tval2[i] == 'z' ) {
        d1.front()->get_num() = Number("x");
        return;
      }
    }
    // equal
    d1.front()->get_num() = Number("1");
  } else {
    d1.push_front( shared_ptr<Operation>(new Operation(Operation::oLe)));
    d1.splice(d1.end(), d2);
  }
}

// ==
void netlist::execute_Eq(list<shared_ptr<Operation> >& d1, list<shared_ptr<Operation> >& d2) {
  if(d1.front()->get_type() == Operation::oNum && d2.front()->get_type() == Operation::oNum) {
    string tval1 = d1.front()->get_num().get_txt_value();
    string tval2 = d2.front()->get_num().get_txt_value();
    if(tval1.size() > tval2.size())
      tval2.insert(tval2.begin(), tval1.size() - tval2.size(), '0');
    else if(tval1.size() < tval2.size())
      tval1.insert(tval1.begin(), tval2.size() - tval1.size(), '0');

    for(unsigned int i=0; i<tval1.size(); i++) {
      if(tval1[i] == '1' && tval2[i] == '0') {
        d1.front()->get_num() = Number("0");
        return;
      } else if(tval1[i] == '0' && tval2[i] == '1') {
        d1.front()->get_num() = Number("0");
        return;
      } else if(tval1[i] == 'x' || tval2[i] == 'x' || tval1[i] == 'z' || tval2[i] == 'z' ) {
        d1.front()->get_num() = Number("x");
        return;
      }
    }
    // equal
    d1.front()->get_num() = Number("1");
  } else {
    d1.push_front( shared_ptr<Operation>(new Operation(Operation::oEq)));
    d1.splice(d1.end(), d2);
  }
}

// !=
void netlist::execute_Neq(list<shared_ptr<Operation> >& d1, list<shared_ptr<Operation> >& d2) {
  if(d1.front()->get_type() == Operation::oNum && d2.front()->get_type() == Operation::oNum) {
    string tval1 = d1.front()->get_num().get_txt_value();
    string tval2 = d2.front()->get_num().get_txt_value();
    if(tval1.size() > tval2.size())
      tval2.insert(tval2.begin(), tval1.size() - tval2.size(), '0');
    else if(tval1.size() < tval2.size())
      tval1.insert(tval1.begin(), tval2.size() - tval1.size(), '0');

    for(unsigned int i=0; i<tval1.size(); i++) {
      if(tval1[i] == '1' && tval2[i] == '0') {
        d1.front()->get_num() = Number("1");
        return;
      } else if(tval1[i] == '0' && tval2[i] == '1') {
        d1.front()->get_num() = Number("1");
        return;
      } else if(tval1[i] == 'x' || tval2[i] == 'x' || tval1[i] == 'z' || tval2[i] == 'z' ) {
        d1.front()->get_num() = Number("x");
        return;
      }
    }
    // equal
    d1.front()->get_num() = Number("0");
  } else {
    d1.push_front( shared_ptr<Operation>(new Operation(Operation::oNeq)));
    d1.splice(d1.end(), d2);
  }
}
// ===
void netlist::execute_CEq(list<shared_ptr<Operation> >& d1, list<shared_ptr<Operation> >& d2) {
  if(d1.front()->get_type() == Operation::oNum && d2.front()->get_type() == Operation::oNum) {
    string tval1 = d1.front()->get_num().get_txt_value();
    string tval2 = d2.front()->get_num().get_txt_value();
    if(tval1.size() > tval2.size())
      tval2.insert(tval2.begin(), tval1.size() - tval2.size(), '0');
    else if(tval1.size() < tval2.size())
      tval1.insert(tval1.begin(), tval2.size() - tval1.size(), '0');

    for(unsigned int i=0; i<tval1.size(); i++) {
      if(tval1[i] != tval2[i]) {
        d1.front()->get_num() = Number("0");
        return;
      } 
    }
    // equal
    d1.front()->get_num() = Number("1");
  } else {
    d1.push_front( shared_ptr<Operation>(new Operation(Operation::oCEq)));
    d1.splice(d1.end(), d2);
  }
}

// !==
void netlist::execute_CNeq(list<shared_ptr<Operation> >& d1, list<shared_ptr<Operation> >& d2) {
  if(d1.front()->get_type() == Operation::oNum && d2.front()->get_type() == Operation::oNum) {
    string tval1 = d1.front()->get_num().get_txt_value();
    string tval2 = d2.front()->get_num().get_txt_value();
    if(tval1.size() > tval2.size())
      tval2.insert(tval2.begin(), tval1.size() - tval2.size(), '0');
    else if(tval1.size() < tval2.size())
      tval1.insert(tval1.begin(), tval2.size() - tval1.size(), '0');

    for(unsigned int i=0; i<tval1.size(); i++) {
      if(tval1[i] != tval2[i]) {
        d1.front()->get_num() = Number("1");
        return;
      } 
    }
    // equal
    d1.front()->get_num() = Number("0");
  } else {
    d1.push_front( shared_ptr<Operation>(new Operation(Operation::oCNeq)));
    d1.splice(d1.end(), d2);
  }
}

// &
void netlist::execute_And(list<shared_ptr<Operation> >& d1, list<shared_ptr<Operation> >& d2) {
  if(d1.front()->get_type() == Operation::oNum && d2.front()->get_type() == Operation::oNum) {
    string tval1 = d1.front()->get_num().get_txt_value();
    string tval2 = d2.front()->get_num().get_txt_value();
    if(tval1.size() > tval2.size())
      tval2.insert(tval2.begin(), tval1.size() - tval2.size(), '0');
    else if(tval1.size() < tval2.size())
      tval1.insert(tval1.begin(), tval2.size() - tval1.size(), '0');

    for(unsigned int i=0; i<tval1.size(); i++) {
      if(tval1[i] != '0' && tval2[i] != '0') {
        if(tval1[i] == '1' && tval2[i] == '1')
          tval1[i] = '1';
        else
          tval1[i] = 'x';
      } else {
        tval1[i] = '0';
      } 
    }
  } else {
    d1.push_front( shared_ptr<Operation>(new Operation(Operation::oAnd)));
    d1.splice(d1.end(), d2);
  }
}

// ^
void netlist::execute_Xor(list<shared_ptr<Operation> >& d1, list<shared_ptr<Operation> >& d2) {
  if(d1.front()->get_type() == Operation::oNum && d2.front()->get_type() == Operation::oNum) {
    string tval1 = d1.front()->get_num().get_txt_value();
    string tval2 = d2.front()->get_num().get_txt_value();
    if(tval1.size() > tval2.size())
      tval2.insert(tval2.begin(), tval1.size() - tval2.size(), '0');
    else if(tval1.size() < tval2.size())
      tval1.insert(tval1.begin(), tval2.size() - tval1.size(), '0');

    for(unsigned int i=0; i<tval1.size(); i++) {
      if(tval1[i] == '0') {
        if(tval2[i] == '1')
          tval1[i] = '1';
        else if(tval2[i] == '0')
          tval1[i] = '0';
        else
          tval1[i] = 'x';
      } else if(tval1[i] == '1') {
        if(tval2[i] == '1')
          tval1[i] = '0';
        else if(tval2[i] == '0')
          tval1[i] = '1';
        else
          tval1[i] = 'x';
      } else
        tval1[i] = 'x';        
    }
  } else {
    d1.push_front( shared_ptr<Operation>(new Operation(Operation::oXor)));
    d1.splice(d1.end(), d2);
  }
}

// ~^
void netlist::execute_Nxor(list<shared_ptr<Operation> >& d1, list<shared_ptr<Operation> >& d2) {
  if(d1.front()->get_type() == Operation::oNum && d2.front()->get_type() == Operation::oNum) {
    string tval1 = d1.front()->get_num().get_txt_value();
    string tval2 = d2.front()->get_num().get_txt_value();
    if(tval1.size() > tval2.size())
      tval2.insert(tval2.begin(), tval1.size() - tval2.size(), '0');
    else if(tval1.size() < tval2.size())
      tval1.insert(tval1.begin(), tval2.size() - tval1.size(), '0');

    for(unsigned int i=0; i<tval1.size(); i++) {
      if(tval1[i] == '0') {
        if(tval2[i] == '1')
          tval1[i] = '0';
        else if(tval2[i] == '0')
          tval1[i] = '1';
        else
          tval1[i] = 'x';
      } else if(tval1[i] == '1') {
        if(tval2[i] == '1')
          tval1[i] = '1';
        else if(tval2[i] == '0')
          tval1[i] = '0';
        else
          tval1[i] = 'x';
      } else
        tval1[i] = 'x';        
    }
  } else {
    d1.push_front( shared_ptr<Operation>(new Operation(Operation::oNxor)));
    d1.splice(d1.end(), d2);
  }
}

// |
void netlist::execute_Or(list<shared_ptr<Operation> >& d1, list<shared_ptr<Operation> >& d2) {
  if(d1.front()->get_type() == Operation::oNum && d2.front()->get_type() == Operation::oNum) {
    string tval1 = d1.front()->get_num().get_txt_value();
    string tval2 = d2.front()->get_num().get_txt_value();
    if(tval1.size() > tval2.size())
      tval2.insert(tval2.begin(), tval1.size() - tval2.size(), '0');
    else if(tval1.size() < tval2.size())
      tval1.insert(tval1.begin(), tval2.size() - tval1.size(), '0');

    for(unsigned int i=0; i<tval1.size(); i++) {
      if(tval1[i] == '1' || tval2[i] == '1') {
        tval1[i] = '1';
      } else if(tval1[i] == '0' && tval2[i] == '0') {
        tval1[i] = '0';
      } else
        tval1[i] = 'x';        
    }
  } else {
    d1.push_front( shared_ptr<Operation>(new Operation(Operation::oOr)));
    d1.splice(d1.end(), d2);
  }
}

// &&
void netlist::execute_LAnd(list<shared_ptr<Operation> >& d1, list<shared_ptr<Operation> >& d2) {
  if(d1.front()->get_type() == Operation::oNum && d2.front()->get_type() == Operation::oNum) {
    string tval1 = d1.front()->get_num().get_txt_value();
    string tval2 = d2.front()->get_num().get_txt_value();
    
    if(string::npos != tval1.find('1') && string::npos != tval2.find('1') ) {
      d1.front()->get_num() = Number("1");
    } else if(string::npos != tval1.find('x') || 
              string::npos != tval2.find('x') ||
              string::npos != tval1.find('z') || 
              string::npos != tval2.find('z') ) {
      d1.front()->get_num() = Number("x");
    } else
      d1.front()->get_num() = Number("0");
  } else {
    d1.push_front( shared_ptr<Operation>(new Operation(Operation::oLAnd)));
    d1.splice(d1.end(), d2);
  }
}

// ||
void netlist::execute_LOr(list<shared_ptr<Operation> >& d1, list<shared_ptr<Operation> >& d2) {
  if(d1.front()->get_type() == Operation::oNum && d2.front()->get_type() == Operation::oNum) {
    string tval1 = d1.front()->get_num().get_txt_value();
    string tval2 = d2.front()->get_num().get_txt_value();
    
    if(string::npos != tval1.find('1') || string::npos != tval2.find('1') ) {
      d1.front()->get_num() = Number("1");
    } else if(string::npos != tval1.find('x') || 
              string::npos != tval2.find('x') ||
              string::npos != tval1.find('z') || 
              string::npos != tval2.find('z') ) {
      d1.front()->get_num() = Number("x");
    } else
      d1.front()->get_num() = Number("0");
  } else {
    d1.push_front( shared_ptr<Operation>(new Operation(Operation::oLOr)));
    d1.splice(d1.end(), d2);
  }
}

// ?:
void netlist::execute_Question(list<shared_ptr<Operation> >& d1, list<shared_ptr<Operation> >& d2, list<shared_ptr<Operation> >&d3) {
  if(d1.front()->get_type() == Operation::oNum) {
    string tval1 = d1.front()->get_num().get_txt_value();
    
    if(string::npos != tval1.find('1')) {
      d1.clear();
      d1.splice(d1.end(), d2);
    } else {
      d1.clear();
      d1.splice(d1.end(), d3);
    }
  } else {
    d1.push_front( shared_ptr<Operation>(new Operation(Operation::oQuestion)));
    d1.splice(d1.end(), d2);
    d1.splice(d1.end(), d3);
  }
}

