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
#include <boost/foreach.hpp>

using namespace netlist;
using std::ostream;
using std::string;
using std::list;
using std::vector;
using boost::shared_ptr;
using boost::static_pointer_cast;

netlist::Operation::Operation()
  : NetComp(tOperation), otype(oNULL), valuable(false)
{}

netlist::Operation::Operation(operation_t otype)
  : NetComp(tOperation), otype(otype), valuable(false)
{
  // only operators do not need an operand
  assert(otype > oFun);
}

netlist::Operation::Operation(const Number& num)
  : NetComp(tOperation), otype(oNum), valuable(true), data(new Number(num))
{ }

netlist::Operation::Operation(const VIdentifier& id)
  : NetComp(tOperation), otype(oVar), valuable(false), data(new VIdentifier(id))
{ }

netlist::Operation::Operation(const shared_ptr<Concatenation>& con)
  : NetComp(tOperation), otype(oCon), valuable(false), data(static_pointer_cast<NetComp>(con))
{ }

netlist::Operation::Operation(const shared_ptr<LConcatenation>& con)
  : NetComp(tOperation), otype(oCon), valuable(false)
{
  if(con->size() == 1) {
    otype = oVar;
    data.reset(new VIdentifier(con->front()));
  } else {
    // copy all elements in LConcatenation to Concatenation
    shared_ptr<Concatenation> cp(new Concatenation());
    BOOST_FOREACH(const VIdentifier& it, con->data) {
      shared_ptr<ConElem> m( new ConElem(shared_ptr<Expression>(new Expression(it))));
      *cp + m;
    }
  }
}

netlist::Operation::Operation(operation_t op, const boost::shared_ptr<Operation>& exp)
 : NetComp(tOperation), otype(op), valuable(false)
{
  child.push_back(exp);
  reduce();
}

netlist::Operation::Operation(operation_t op, const boost::shared_ptr<Operation>& exp1,
                              const boost::shared_ptr<Operation>& exp2)
 : NetComp(tOperation), otype(op), valuable(false)
{
  child.push_back(exp1);
  child.push_back(exp2);
  reduce();
}

netlist::Operation::Operation(operation_t op, 
                              const boost::shared_ptr<Operation>& exp1,
                              const boost::shared_ptr<Operation>& exp2,
                              const boost::shared_ptr<Operation>& exp3)
 : NetComp(tOperation), otype(op), valuable(false)
{
  child.push_back(exp1);
  child.push_back(exp2);
  child.push_back(exp3);
  reduce();
}

netlist::Operation::~Operation() {
  if(data.use_count() != 0) {
    data->db_expunge();
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

string netlist::Operation::toString() const {
  string op;
  switch(otype) {
  case oNum: 
  case oVar: 
  case oCon:    return ::toString(*data);
  case oUPos:   return ::toString(*(child[0]));
  case oUNeg:   op = "-";
  case oULRev:  op = "!";
  case oURev:   op = "~";
  case oUAnd:   op = "&";
  case oUNand:  op = "~&";
  case oUOr:    op = "|";
  case oUNor:   op = "~|";
  case oUXor:   op = "^";
  case oUNxor:  op = "~^";
    return 
      op + 
      (child[0]->otype > oUNxor 
       ? "("+::toString(*(child[0]))+")" : ::toString(*(child[0])));    
  case oPower:  op = "**";
    return 
      (child[0]->otype > oPower 
       ? "("+::toString(*(child[0]))+")" : ::toString(*(child[0])))
      + " " + op + " " + 
      (child[1]->otype > oPower 
       ? "("+::toString(*(child[1]))+")" : ::toString(*(child[1])));    
  case oTime:   op = "*";
  case oDiv:    op = "/";
    return 
      (child[0]->otype > oDiv 
       ? "("+::toString(*(child[0]))+")" : ::toString(*(child[0])))
      + " " + op + " " + 
      (child[1]->otype > oDiv 
       ? "("+::toString(*(child[1]))+")" : ::toString(*(child[1])));    
  case oMode:   op = "%";
  case oAdd:    op = "+";
  case oMinus:  op = "-";
    return 
      (child[0]->otype > oMinus 
       ? "("+::toString(*(child[0]))+")" : ::toString(*(child[0])))
      + " " + op + " " + 
      (child[1]->otype > oMinus 
       ? "("+::toString(*(child[1]))+")" : ::toString(*(child[1])));    
  case oRS:     op = ">>";
  case oLS:     op = "<<";
  case oLRS:    op = ">>>";
    return 
      (child[0]->otype >= oRS 
       ? "("+::toString(*(child[0]))+")" : ::toString(*(child[0])))
      + " " + op + " " + 
      (child[1]->otype >= oRS 
       ? "("+::toString(*(child[1]))+")" : ::toString(*(child[1])));    
  case oLess:   op = "<";
  case oLe:     op = "<=";
  case oGreat:  op = ">";
  case oGe:     op = ">=";
    return 
      (child[0]->otype >= oLess 
       ? "("+::toString(*(child[0]))+")" : ::toString(*(child[0])))
      + " " + op + " " + 
      (child[1]->otype >= oLess 
       ? "("+::toString(*(child[1]))+")" : ::toString(*(child[1])));    
  case oEq:     op = "==";
  case oNeq:    op = "!=";
  case oCEq:    op = "===";
  case oCNeq:   op = "!==";
    return 
      (child[0]->otype >= oEq 
       ? "("+::toString(*(child[0]))+")" : ::toString(*(child[0])))
      + " " + op + " " + 
      (child[1]->otype >= oEq 
       ? "("+::toString(*(child[1]))+")" : ::toString(*(child[1])));    
  case oAnd:    op = "&";
    return 
      (child[0]->otype > oAnd 
       ? "("+::toString(*(child[0]))+")" : ::toString(*(child[0])))
      + " " + op + " " + 
      (child[1]->otype > oAnd 
       ? "("+::toString(*(child[1]))+")" : ::toString(*(child[1])));    
  case oXor:    op = "^";
  case oNxor:   op = "~^";
    return 
      (child[0]->otype > oNxor 
       ? "("+::toString(*(child[0]))+")" : ::toString(*(child[0])))
      + " " + op + " " + 
      (child[1]->otype > oNxor 
       ? "("+::toString(*(child[1]))+")" : ::toString(*(child[1])));    
  case oOr:     op = "|";
    return 
      (child[0]->otype > oOr 
       ? "("+::toString(*(child[0]))+")" : ::toString(*(child[0])))
      + " " + op + " " + 
      (child[1]->otype > oOr 
       ? "("+::toString(*(child[1]))+")" : ::toString(*(child[1])));    
  case oLAnd:   op = "&&";
    return 
      (child[0]->otype > oLAnd 
       ? "("+::toString(*(child[0]))+")" : ::toString(*(child[0])))
      + " " + op + " " + 
      (child[1]->otype > oLAnd 
       ? "("+::toString(*(child[1]))+")" : ::toString(*(child[1])));    
  case oLOr:    op = "||";
    return 
      (child[0]->otype > oLOr 
       ? "("+::toString(*(child[0]))+")" : ::toString(*(child[0])))
      + " " + op + " " + 
      (child[1]->otype > oLOr 
       ? "("+::toString(*(child[1]))+")" : ::toString(*(child[1])));    
  case oQuestion:
    return 
      ::toString(*(child[0])) + " ? " 
      + ::toString(*(child[1])) + " : " + ::toString(*(child[2]));
  default:
    assert(0 == "fail to convert this type of operation to string!");
    return "";
  }
}


void netlist::Operation::db_register(int iod) {
  if(data.use_count() != 0) data->db_register(iod);
  if(child.size()) {
    BOOST_FOREACH(shared_ptr<Operation>& m, child) 
      m->db_register(iod);
  }
}

void netlist::Operation::db_expunge() {
  if(data.use_count() != 0) data->db_expunge();
  if(child.size()) {
    BOOST_FOREACH(shared_ptr<Operation>& m, child) 
      m->db_expunge();
  }
}

void netlist::Operation::set_father(Block *pf) {
  if(father == pf) return;
  father = pf;
  if(data.use_count() != 0) data->set_father(pf);
  if(child.size()) {
    BOOST_FOREACH(shared_ptr<Operation>& m, child) 
      m->set_father(pf);
  }
}

bool netlist::Operation::check_inparse() {
  if((otype <= oCon) && (otype >= oNum)) return data->check_inparse();
  else {
    bool rv = true;
    BOOST_FOREACH(shared_ptr<Operation>& m, child)
      rv &= m->check_inparse();
    return rv;
  }
}

ostream& netlist::Operation::streamout(ostream& os, unsigned int indent) const {
  os << string(indent, ' ') << toString();
  return os;
}

Operation* netlist::Operation::deep_copy() const {
  Operation* rv = new Operation();
  rv->otype = this->otype;
  rv->valuable = this->valuable;
  if(data.use_count() != 0) rv->data = shared_ptr<NetComp>(data->deep_copy());
  if(child.size()) {
    rv->child = vector<shared_ptr<Operation> >(child.size());
    for(unsigned int i=0; i<child.size(); i++) 
      rv->child[i].reset(child[i]->deep_copy());
  }
  return rv;
}

void netlist::Operation::reduce() {
  switch(otype) {
  case oNum:      reduce_Num();      break;
  case oVar:      reduce_Var();      break;
  case oCon:      reduce_Con();      break;
  case oUPos:     reduce_UPos();     break;
  case oUNeg:     reduce_UNeg();     break;
  case oULRev:    reduce_ULRev();    break;
  case oURev:     reduce_URev();     break;
  case oUAnd:     reduce_UAnd();     break;
  case oUNand:    reduce_UNand();    break;
  case oUOr:      reduce_UOr();      break;
  case oUNor:     reduce_UNor();     break;
  case oUXor:     reduce_UXor();     break;
  case oUNxor:    reduce_UNxor();    break;
  case oPower:    reduce_Power();    break;
  case oTime:     reduce_Time();     break;
  case oDiv:      reduce_Div();      break;
  case oMode:     reduce_Mode();     break;
  case oAdd:      reduce_Add();      break;
  case oMinus:    reduce_Minus();    break;
  case oRS:       reduce_RS();       break;
  case oLS:       reduce_LS();       break;
  case oLRS:      reduce_LRS();      break;
  case oLess:     reduce_Less();     break;
  case oLe:       reduce_Le();       break;
  case oGreat:    reduce_Great();    break;
  case oGe:       reduce_Ge();       break;
  case oEq:       reduce_Eq();       break;
  case oNeq:      reduce_Neq();      break;
  case oCEq:      reduce_CEq();      break;
  case oCNeq:     reduce_CNeq();     break;
  case oAnd:      reduce_And();      break;
  case oXor:      reduce_Xor();      break;
  case oNxor:     reduce_Nxor();     break;
  case oOr:       reduce_Or();       break;
  case oLAnd:     reduce_LAnd();     break;
  case oLOr:      reduce_LOr();      break;
  case oQuestion: reduce_Question(); break;
  default:  // should not run to here
    assert(0 == "wrong operation type");
  }
}

bool netlist::Operation::elaborate(NetComp::elab_result_t &result, const NetComp::ctype_t mctype, const vector<NetComp *>& fp) {
  bool rv = true;
  result = NetComp::ELAB_Normal;

  if(otype == oVar) {
    rv &= data->elaborate(result, NetComp::tExp);
    if(!rv) return rv;
    SP_CAST(m, VIdentifier, data);
    if(m->is_valuable()) {
      data.reset(new Number(m->get_value()));
      otype = oNum;
      valuable = true;
    }
  } else if(otype == oCon) {
    rv &= data->elaborate(result, NetComp::tExp);
    if(!rv) return rv;
    SP_CAST(m, Concatenation, data);
    if(m->is_valuable()) {
      data.reset(new Number(m->get_value()));
      otype = oNum;
      valuable = true;
    } 
  }

  // a number or operator does not need to be reduced further
  // function is not supported yet
  return rv;   
}

void netlist::Operation::reduce_Num() {
  assert(child.size() == 0);
  valuable = true;
}

void netlist::Operation::reduce_Con() {
  assert(child.size() == 0);
  SP_CAST(m, Concatenation, data);
  m->reduce();
  if(m->is_valuable()) {
    data.reset(new Number(m->get_value()));
    otype = oNum;
    valuable = true;
  }  
}

void netlist::Operation::reduce_Var() {
  assert(child.size() == 0);
  SP_CAST(m, VIdentifier, data);
  if(m->is_valuable()) {
    data.reset(new Number(m->get_value()));
    otype = oNum;
    valuable = true;
  }  
}

// unary +
void netlist::Operation::reduce_UPos() {
  assert(child.size() == 1);
  assert(child[0].use_count() != 0);
  child[0]->reduce();
  *this = *(child[0]);
  return;
}

// unary -
void netlist::Operation::reduce_UNeg() {
  assert(child.size() == 1);
  assert(child[0].use_count() != 0);
  child[0]->reduce();
  if(child[0]->is_valuable()) {
    data.reset(new Number(-child[0]->get_num()));
    child.clear();
    otype = oNum;
    valuable = true;
  }
}

// unary !
void netlist::Operation::reduce_ULRev() {
  assert(child.size() == 1);
  assert(child[0].use_count() != 0);
  child[0]->reduce();
  if(child[0]->is_valuable()) {
    data.reset(new Number(!(child[0]->get_num())));
    child.clear();
    otype = oNum;
    valuable = true;
  }
}

// unary ~
void netlist::Operation::reduce_URev() {
  assert(child.size() == 1);
  assert(child[0].use_count() != 0);
  child[0]->reduce();
  if(child[0]->is_valuable()) {
    data.reset(new Number(~(child[0]->get_num())));
    child.clear();
    otype = oNum;
    valuable = true;
  }
}

// unary &
void netlist::Operation::reduce_UAnd() {
  assert(child.size() == 1);
  assert(child[0].use_count() != 0);
  child[0]->reduce();
  if(child[0]->is_valuable()) {
    data.reset(new Number(op_uand(child[0]->get_num())));
    child.clear();
    otype = oNum;
    valuable = true;
  }
}

// unary ~&
void netlist::Operation::reduce_UNand() {
  assert(child.size() == 1);
  assert(child[0].use_count() != 0);
  child[0]->reduce();
  if(child[0]->is_valuable()) {
    data.reset(new Number(~op_uand(child[0]->get_num())));
    child.clear();
    otype = oNum;
    valuable = true;
  }
}

// unary |
void netlist::Operation::reduce_UOr() {
  assert(child.size() == 1);
  assert(child[0].use_count() != 0);
  child[0]->reduce();
  if(child[0]->is_valuable()) {
    data.reset(new Number(op_uor(child[0]->get_num())));
    child.clear();
    otype = oNum;
    valuable = true;
  }
}

// unary ~|
void netlist::Operation::reduce_UNor() {
  assert(child.size() == 1);
  assert(child[0].use_count() != 0);
  child[0]->reduce();
  if(child[0]->is_valuable()) {
    data.reset(new Number(~op_uor(child[0]->get_num())));
    child.clear();
    otype = oNum;
    valuable = true;
  }
}

// unary ^
void netlist::Operation::reduce_UXor() {
  assert(child.size() == 1);
  assert(child[0].use_count() != 0);
  child[0]->reduce();
  if(child[0]->is_valuable()) {
    data.reset(new Number(op_uxor(child[0]->get_num())));
    child.clear();
    otype = oNum;
    valuable = true;
  }
}

// unary ~^
void netlist::Operation::reduce_UNxor() {
  assert(child.size() == 1);
  assert(child[0].use_count() != 0);
  child[0]->reduce();
  if(child[0]->is_valuable()) {
    data.reset(new Number(~op_uxor(child[0]->get_num())));
    child.clear();
    otype = oNum;
    valuable = true;
  }
}
  
// **
void netlist::Operation::reduce_Power() {
  assert(child.size() == 2);
  assert(child[0].use_count() != 0);
  assert(child[1].use_count() != 0);
  child[0]->reduce();
  child[1]->reduce();
  if(child[0]->is_valuable() && child[1]->is_valuable()) {
    mpz_class res;
    mpz_pow_ui(res.get_mpz_t(), 
               child[0]->get_num().get_value().get_mpz_t(), 
               child[1]->get_num().get_value().get_ui());
    data.reset(new Number(res));
    child.clear();
    otype = oNum;
    valuable = true;
  }
}

// *
void netlist::Operation::reduce_Time() {
  assert(child.size() == 2);
  assert(child[0].use_count() != 0);
  assert(child[1].use_count() != 0);
  child[0]->reduce();
  child[1]->reduce();
  if(child[0]->is_valuable() && child[1]->is_valuable()) {
    data.reset(new Number(child[0]->get_num() * child[1]->get_num()));
    child.clear();
    otype = oNum;
    valuable = true;
  }
}

// /
void netlist::Operation::reduce_Div() {
  assert(child.size() == 2);
  assert(child[0].use_count() != 0);
  assert(child[1].use_count() != 0);
  child[0]->reduce();
  child[1]->reduce();
  if(child[0]->is_valuable() && child[1]->is_valuable()) {
    data.reset(new Number(child[0]->get_num() / child[1]->get_num()));
    child.clear();
    otype = oNum;
    valuable = true;
  }
}

// %
void netlist::Operation::reduce_Mode() {
  assert(child.size() == 2);
  assert(child[0].use_count() != 0);
  assert(child[1].use_count() != 0);
  child[0]->reduce();
  child[1]->reduce();
  if(child[0]->is_valuable() && child[1]->is_valuable()) {
    data.reset(new Number(child[0]->get_num() % child[1]->get_num()));
    child.clear();
    otype = oNum;
    valuable = true;
  }
}


// +
void netlist::Operation::reduce_Add() {
  assert(child.size() == 2);
  assert(child[0].use_count() != 0);
  assert(child[1].use_count() != 0);
  child[0]->reduce();
  child[1]->reduce();
  if(child[0]->is_valuable() && child[1]->is_valuable()) {
    data.reset(new Number(child[0]->get_num() + child[1]->get_num()));
    child.clear();
    otype = oNum;
    valuable = true;
  }
}

// -
void netlist::Operation::reduce_Minus() {
  assert(child.size() == 2);
  assert(child[0].use_count() != 0);
  assert(child[1].use_count() != 0);
  child[0]->reduce();
  child[1]->reduce();
  if(child[0]->is_valuable() && child[1]->is_valuable()) {
    data.reset(new Number(child[0]->get_num() - child[1]->get_num()));
    child.clear();
    otype = oNum;
    valuable = true;
  }
}

// >>
void netlist::Operation::reduce_RS() {
  assert(child.size() == 2);
  assert(child[0].use_count() != 0);
  assert(child[1].use_count() != 0);
  child[0]->reduce();
  child[1]->reduce();
  if(child[0]->is_valuable() && child[1]->is_valuable()) {
    data.reset(new Number(child[0]->get_num() >> child[1]->get_num()));
    child.clear();
    otype = oNum;
    valuable = true;
  }
}

// <<
void netlist::Operation::reduce_LS() {
  assert(child.size() == 2);
  assert(child[0].use_count() != 0);
  assert(child[1].use_count() != 0);
  child[0]->reduce();
  child[1]->reduce();
  if(child[0]->is_valuable() && child[1]->is_valuable()) {
    data.reset(new Number(child[0]->get_num() << child[1]->get_num()));
    child.clear();
    otype = oNum;
    valuable = true;
  }
}

// >>>
void netlist::Operation::reduce_LRS() {
  assert(child.size() == 2);
  assert(child[0].use_count() != 0);
  assert(child[1].use_count() != 0);
  child[0]->reduce();
  child[1]->reduce();
  if(child[0]->is_valuable() && child[1]->is_valuable()) {
    data.reset(new Number(op_sign_rsh(child[0]->get_num(), child[1]->get_num())));
    child.clear();
    otype = oNum;
    valuable = true;
  }
}

// <
void netlist::Operation::reduce_Less() {
  assert(child.size() == 2);
  assert(child[0].use_count() != 0);
  assert(child[1].use_count() != 0);
  child[0]->reduce();
  child[1]->reduce();
  if(child[0]->is_valuable() && child[1]->is_valuable()) {
    data.reset(new Number(child[0]->get_num() < child[1]->get_num()));
    child.clear();
    otype = oNum;
    valuable = true;
  }
}

// <=
void netlist::Operation::reduce_Le() {
  assert(child.size() == 2);
  assert(child[0].use_count() != 0);
  assert(child[1].use_count() != 0);
  child[0]->reduce();
  child[1]->reduce();
  if(child[0]->is_valuable() && child[1]->is_valuable()) {
    data.reset(new Number(child[0]->get_num() <= child[1]->get_num()));
    child.clear();
    otype = oNum;
    valuable = true;
  }
}

// >
void netlist::Operation::reduce_Great() {
  assert(child.size() == 2);
  assert(child[0].use_count() != 0);
  assert(child[1].use_count() != 0);
  child[0]->reduce();
  child[1]->reduce();
  if(child[0]->is_valuable() && child[1]->is_valuable()) {
    data.reset(new Number(child[0]->get_num() > child[1]->get_num()));
    child.clear();
    otype = oNum;
    valuable = true;
  }
}

// >=
void netlist::Operation::reduce_Ge() {
  assert(child.size() == 2);
  assert(child[0].use_count() != 0);
  assert(child[1].use_count() != 0);
  child[0]->reduce();
  child[1]->reduce();
  if(child[0]->is_valuable() && child[1]->is_valuable()) {
    data.reset(new Number(child[0]->get_num() >= child[1]->get_num()));
    child.clear();
    otype = oNum;
    valuable = true;
  }
}

// ==
void netlist::Operation::reduce_Eq() {
  assert(child.size() == 2);
  assert(child[0].use_count() != 0);
  assert(child[1].use_count() != 0);
  child[0]->reduce();
  child[1]->reduce();
  if(child[0]->is_valuable() && child[1]->is_valuable()) {
    data.reset(new Number(child[0]->get_num() == child[1]->get_num()));
    child.clear();
    otype = oNum;
    valuable = true;
  }
}


// !=
void netlist::Operation::reduce_Neq() {
  assert(child.size() == 2);
  assert(child[0].use_count() != 0);
  assert(child[1].use_count() != 0);
  child[0]->reduce();
  child[1]->reduce();
  if(child[0]->is_valuable() && child[1]->is_valuable()) {
    data.reset(new Number(child[0]->get_num() != child[1]->get_num()));
    child.clear();
    otype = oNum;
    valuable = true;
  }
}

// !==
void netlist::Operation::reduce_CNeq() {
  assert(child.size() == 2);
  assert(child[0].use_count() != 0);
  assert(child[1].use_count() != 0);
  child[0]->reduce();
  child[1]->reduce();
  if(child[0]->is_valuable() && child[1]->is_valuable()) {
    data.reset(new Number(!op_case_equal(child[0]->get_num(), child[1]->get_num())));
    child.clear();
    otype = oNum;
    valuable = true;
  }
}

// &
void netlist::Operation::reduce_And() {
  assert(child.size() == 2);
  assert(child[0].use_count() != 0);
  assert(child[1].use_count() != 0);
  child[0]->reduce();
  child[1]->reduce();
  if(child[0]->is_valuable() && child[1]->is_valuable()) {
    data.reset(new Number(child[0]->get_num() & child[1]->get_num()));
    child.clear();
    otype = oNum;
    valuable = true;
  }
}

// ^
void netlist::Operation::reduce_Xor() {
  assert(child.size() == 2);
  assert(child[0].use_count() != 0);
  assert(child[1].use_count() != 0);
  child[0]->reduce();
  child[1]->reduce();
  if(child[0]->is_valuable() && child[1]->is_valuable()) {
    data.reset(new Number(child[0]->get_num() ^ child[1]->get_num()));
    child.clear();
    otype = oNum;
    valuable = true;
  }
}

// ~^
void netlist::Operation::reduce_Nxor() {
  assert(child.size() == 2);
  assert(child[0].use_count() != 0);
  assert(child[1].use_count() != 0);
  child[0]->reduce();
  child[1]->reduce();
  if(child[0]->is_valuable() && child[1]->is_valuable()) {
    data.reset(new Number(~(child[0]->get_num() ^ child[1]->get_num())));
    child.clear();
    otype = oNum;
    valuable = true;
  }
}

// |
void netlist::Operation::reduce_Or() {
  assert(child.size() == 2);
  assert(child[0].use_count() != 0);
  assert(child[1].use_count() != 0);
  child[0]->reduce();
  child[1]->reduce();
  if(child[0]->is_valuable() && child[1]->is_valuable()) {
    data.reset(new Number(~(child[0]->get_num() | child[1]->get_num())));
    child.clear();
    otype = oNum;
    valuable = true;
  }
}

// &&
void netlist::Operation::reduce_LAnd() {
  assert(child.size() == 2);
  assert(child[0].use_count() != 0);
  assert(child[1].use_count() != 0);
  child[0]->reduce();
  child[1]->reduce();
  if(child[0]->is_valuable() && child[1]->is_valuable()) {
    data.reset(new Number(~(child[0]->get_num() && child[1]->get_num())));
    child.clear();
    otype = oNum;
    valuable = true;
  } else if(child[0]->is_valuable()) {
    Number& m1 = child[0]->get_num();
    if(m1.is_false()) {           // 0 && x => 0
      data.reset(new Number(0));
      child.clear();
      otype = oNum;
      valuable = true;
    } else if (m1.is_true()) {    // 1 && x => x 
      *this = *(child[1]);
    } 
  } else if(child[1]->is_valuable()) {
    Number& m2 = child[1]->get_num();
    if(m2.is_false()) {           // x && 0 => 0
      data.reset(new Number(0));
      child.clear();
      otype = oNum;
      valuable = true;
    } else if (m2.is_true()) {    // x && 1 => x 
      *this = *(child[0]);
    } 
  }
}

// ||
void netlist::Operation::reduce_LOr() {
  assert(child.size() == 2);
  assert(child[0].use_count() != 0);
  assert(child[1].use_count() != 0);
  child[0]->reduce();
  child[1]->reduce();
  if(child[0]->is_valuable() && child[1]->is_valuable()) {
    data.reset(new Number(~(child[0]->get_num() || child[1]->get_num())));
    child.clear();
    otype = oNum;
    valuable = true;
  } else if(child[0]->is_valuable()) {
    Number& m1 = child[0]->get_num();
    if(m1.is_true()) {           // 1 || x => 1
      data.reset(new Number(1));
      child.clear();
      otype = oNum;
      valuable = true;
    } else if (m1.is_false()) {    // 0 || x => x 
      *this = *(child[1]);
    } 
  } else if(child[1]->is_valuable()) {
    Number& m2 = child[1]->get_num();
    if(m2.is_true()) {           // x || 1 => 1
      data.reset(new Number(1));
      child.clear();
      otype = oNum;
      valuable = true;
    } else if (m2.is_false()) {    // x || 0 => x 
      *this = *(child[0]);
    } 
  }
}

// ?:
void netlist::Operation::reduce_Question() {
  assert(child.size() == 3);
  assert(child[0].use_count() != 0);
  assert(child[1].use_count() != 0);
  assert(child[2].use_count() != 0);
  child[0]->reduce();
  child[1]->reduce();
  child[2]->reduce();
  if(child[0]->is_valuable()) {
    if(child[0]->get_num().is_true()) {
      *this = *(child[1]);
    } else if(child[0]->get_num().is_false()){
      *this = *(child[2]);
    }
  }
}

