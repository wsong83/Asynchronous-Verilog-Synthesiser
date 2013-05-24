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
 * the operation in expression
 * 11/02/2012   Wei Song
 *
 *
 */

#include "component.h"
#include "shell/env.h"
#include "sdfg/rtree.hpp"
#include <boost/foreach.hpp>
#include <algorithm>
#include <boost/tuple/tuple.hpp>

using namespace netlist;
using std::ostream;
using std::string;
using std::list;
using std::vector;
using boost::shared_ptr;
using boost::static_pointer_cast;
using shell::location;

netlist::Operation::Operation()
  : NetComp(tOperation), otype(oNULL), valuable(false)
{}

netlist::Operation::Operation(operation_t otype)
  : NetComp(tOperation), otype(otype), valuable(false)
{
  // only operators do not need an operand
  assert(otype > oFun);
}

netlist::Operation::Operation(const location& lloc, operation_t otype)
  : NetComp(tOperation, lloc), otype(otype), valuable(false)
{
  // only operators do not need an operand
  assert(otype > oFun);
}

netlist::Operation::Operation(const Number& num)
  : NetComp(tOperation, num.loc), otype(oNum), valuable(true), data(new Number(num))
{ }

netlist::Operation::Operation(const location& lloc, const Number& num)
  : NetComp(tOperation, lloc), otype(oNum), valuable(true), data(new Number(num))
{ }

netlist::Operation::Operation(const VIdentifier& id)
  : NetComp(tOperation, id.loc), otype(oVar), valuable(false), data(new VIdentifier(id))
{ }

netlist::Operation::Operation(const location& lloc, const VIdentifier& id)
  : NetComp(tOperation, lloc), otype(oVar), valuable(false), data(new VIdentifier(id))
{ }

netlist::Operation::Operation(const shared_ptr<Concatenation>& con)
  : NetComp(tOperation, con->loc), otype(oCon), valuable(false), data(static_pointer_cast<NetComp>(con))
{ }

netlist::Operation::Operation(const location& lloc, const shared_ptr<Concatenation>& con)
  : NetComp(tOperation, lloc), otype(oCon), valuable(false), data(static_pointer_cast<NetComp>(con))
{ }

netlist::Operation::Operation(const shared_ptr<LConcatenation>& con)
  : NetComp(tOperation, con->loc), otype(oCon), valuable(false)
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

netlist::Operation::Operation(const location& lloc, const shared_ptr<LConcatenation>& con)
  : NetComp(tOperation, lloc), otype(oCon), valuable(false)
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

netlist::Operation::Operation(const location& lloc, const shared_ptr<FuncCall>& fc)
  : NetComp(tOperation, lloc), otype(oFun), valuable(false), data(static_pointer_cast<NetComp>(fc))
{ }

netlist::Operation::Operation(operation_t op, const boost::shared_ptr<Operation>& exp)
  : NetComp(tOperation, exp->loc), otype(op), valuable(false)
{
  child.push_back(exp);
  reduce();
}

netlist::Operation::Operation(const location& lloc, operation_t op, const boost::shared_ptr<Operation>& exp)
  : NetComp(tOperation, lloc), otype(op), valuable(false)
{
  child.push_back(exp);
  reduce();
}

netlist::Operation::Operation(operation_t op, const boost::shared_ptr<Operation>& exp1,
                              const boost::shared_ptr<Operation>& exp2)
  : NetComp(tOperation, exp1->loc + exp2->loc), otype(op), valuable(false)
{
  child.push_back(exp1);
  child.push_back(exp2);
  reduce();
}

netlist::Operation::Operation(const location& lloc, operation_t op, 
                              const boost::shared_ptr<Operation>& exp1,
                              const boost::shared_ptr<Operation>& exp2)
  : NetComp(tOperation, lloc), otype(op), valuable(false)
{
  child.push_back(exp1);
  child.push_back(exp2);
  reduce();
}

netlist::Operation::Operation(operation_t op, 
                              const boost::shared_ptr<Operation>& exp1,
                              const boost::shared_ptr<Operation>& exp2,
                              const boost::shared_ptr<Operation>& exp3)
  : NetComp(tOperation, exp1->loc + exp3->loc), otype(op), valuable(false)
{
  child.push_back(exp1);
  child.push_back(exp2);
  child.push_back(exp3);
  reduce();
}

netlist::Operation::Operation(const location& lloc, operation_t op, 
                              const boost::shared_ptr<Operation>& exp1,
                              const boost::shared_ptr<Operation>& exp2,
                              const boost::shared_ptr<Operation>& exp3)
  : NetComp(tOperation, lloc), otype(op), valuable(false)
{
  child.push_back(exp1);
  child.push_back(exp2);
  child.push_back(exp3);
  reduce();
}

netlist::Operation::~Operation() {
  if(data) {
    data->db_expunge();
  }
}

Number& netlist::Operation::get_num(){
  assert(otype == oNum);
  return *(static_pointer_cast<Number>(data));
}

const Number& netlist::Operation::get_num() const{
  assert(otype == oNum);
  return *(static_pointer_cast<Number>(data));
}

Concatenation& netlist::Operation::get_con() {
  assert(otype == oCon);
  return *(static_pointer_cast<Concatenation>(data));
}

const Concatenation& netlist::Operation::get_con() const{
  assert(otype == oCon);
  return *(static_pointer_cast<Concatenation>(data));
}

FuncCall& netlist::Operation::get_fun() {
  assert(otype == oFun);
  return *(static_pointer_cast<FuncCall>(data));
}

const FuncCall& netlist::Operation::get_fun() const{
  assert(otype == oFun);
  return *(static_pointer_cast<FuncCall>(data));
}

VIdentifier& netlist::Operation::get_var(){
  assert(otype == oVar);
  return *(static_pointer_cast<VIdentifier>(data));
}

const VIdentifier& netlist::Operation::get_var() const {
  assert(otype == oVar);
  return *(static_pointer_cast<VIdentifier>(data));
}

string netlist::Operation::toString() const {
  string op;
  switch(otype) {
  case oNum: 
  case oVar: 
  case oCon:
  case oFun:    return ::toString(*data);
  case oUPos:   return ::toString(*(child[0]));
  case oUNeg:   op = "-";   goto UNARY0;
  case oULRev:  op = "!";   goto UNARY0;
  case oURev:   op = "~";   goto UNARY0;
  case oUAnd:   op = "&";   goto UNARY0;
  case oUNand:  op = "~&";  goto UNARY0;
  case oUOr:    op = "|";   goto UNARY0;
  case oUNor:   op = "~|";  goto UNARY0;
  case oUXor:   op = "^";   goto UNARY0;
  case oUNxor:  op = "~^";
  UNARY0:  return 
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
  case oTime:   op = "*";   goto BDIV;
  case oDiv:    op = "/";
  BDIV: return 
      (child[0]->otype > oDiv 
       ? "("+::toString(*(child[0]))+")" : ::toString(*(child[0])))
      + " " + op + " " + 
      (child[1]->otype > oDiv 
       ? "("+::toString(*(child[1]))+")" : ::toString(*(child[1])));    
  case oMode:   op = "%";   goto BMINUS;
  case oAdd:    op = "+";   goto BMINUS;
  case oMinus:  op = "-";
  BMINUS: return 
      (child[0]->otype > oMinus 
       ? "("+::toString(*(child[0]))+")" : ::toString(*(child[0])))
      + " " + op + " " + 
      (child[1]->otype > oMinus 
       ? "("+::toString(*(child[1]))+")" : ::toString(*(child[1])));    
  case oRS:     op = ">>";   goto BLRS;
  case oLS:     op = "<<";   goto BLRS;
  case oLRS:    op = ">>>";
  BLRS: return 
      (child[0]->otype >= oRS 
       ? "("+::toString(*(child[0]))+")" : ::toString(*(child[0])))
      + " " + op + " " + 
      (child[1]->otype >= oRS 
       ? "("+::toString(*(child[1]))+")" : ::toString(*(child[1])));    
  case oLess:   op = "<";   goto BGE;
  case oLe:     op = "<=";  goto BGE;
  case oGreat:  op = ">";   goto BGE;
  case oGe:     op = ">=";
  BGE:  return 
      (child[0]->otype >= oLess 
       ? "("+::toString(*(child[0]))+")" : ::toString(*(child[0])))
      + " " + op + " " + 
      (child[1]->otype >= oLess 
       ? "("+::toString(*(child[1]))+")" : ::toString(*(child[1])));    
  case oEq:     op = "==";  goto BCNEQ;
  case oNeq:    op = "!=";  goto BCNEQ;
  case oCEq:    op = "==="; goto BCNEQ;
  case oCNeq:   op = "!=="; goto BCNEQ;
  BCNEQ:  return 
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
  case oXor:    op = "^";   goto BNXOR;
  case oNxor:   op = "~^";
  BNXOR:  return 
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
  if(data) data->db_register(iod);
  if(child.size()) {
    BOOST_FOREACH(shared_ptr<Operation>& m, child) 
      m->db_register(iod);
  }
}

void netlist::Operation::db_expunge() {
  if(data) data->db_expunge();
  if(child.size()) {
    BOOST_FOREACH(shared_ptr<Operation>& m, child) 
      m->db_expunge();
  }
}

void netlist::Operation::set_father(Block *pf) {
  if(father == pf) return;
  father = pf;
  if(data) data->set_father(pf);
  if(child.size()) {
    BOOST_FOREACH(shared_ptr<Operation>& m, child) 
      m->set_father(pf);
  }
}

ostream& netlist::Operation::streamout(ostream& os, unsigned int indent) const {
  os << string(indent, ' ') << toString();
  return os;
}

Operation* netlist::Operation::deep_copy() const {
  Operation* rv = new Operation();
  rv->loc = loc;
  rv->otype = this->otype;
  rv->valuable = this->valuable;
  if(data) rv->data = shared_ptr<NetComp>(data->deep_copy());
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
  case oFun:      reduce_Fun();      break;
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

shared_ptr<SDFG::RTree> netlist::Operation::get_rtree() const {
  switch(otype) {
  case oVar: return get_var().get_rtree();
  case oCon: return get_con().get_rtree();
  case oNULL:
  case oNum: return shared_ptr<SDFG::RTree>(new SDFG::RTree());
  case oFun: return get_fun().get_rtree();
  case oUPos:
  case oUNeg:
  case oULRev:
  case oURev:
  case oUAnd:
  case oUNand:
  case oUOr:
  case oUNor:
  case oUXor:
  case oUNxor: 
    return shared_ptr<SDFG::RTree>(new SDFG::RTree(child[0]->get_rtree(), 
                                                   SDFG::dfgEdge::SDFG_DAT)
                                   );
  case oPower:
  case oTime:
  case oDiv:
  case oMode:
  case oRS:
  case oLS:
  case oLRS:
  case oAnd:
  case oXor:
  case oNxor:
  case oOr:
  case oLAnd:
  case oLOr: 
    return shared_ptr<SDFG::RTree>(new SDFG::RTree(child[0]->get_rtree(),
                                                   child[1]->get_rtree(),
                                                   SDFG::dfgEdge::SDFG_DAT)
                                   );
  case oAdd:
  case oMinus:
    return shared_ptr<SDFG::RTree>(new SDFG::RTree(child[0]->get_rtree(),
                                                   child[1]->get_rtree(),
                                                   SDFG::dfgEdge::SDFG_CAL)
                                   );
  case oLess:
  case oLe:
  case oGreat:
  case oGe:
    return shared_ptr<SDFG::RTree>(new SDFG::RTree(child[0]->get_rtree(),
                                                   child[1]->get_rtree(),
                                                   SDFG::dfgEdge::SDFG_CMP)
                                   );
  case oEq:
  case oNeq:
  case oCEq:
  case oCNeq:
    return shared_ptr<SDFG::RTree>(new SDFG::RTree(child[0]->get_rtree(),
                                                   child[1]->get_rtree(),
                                                   SDFG::dfgEdge::SDFG_EQU)
                                   );
  case oQuestion:
    return shared_ptr<SDFG::RTree>(new SDFG::RTree(child[0]->get_rtree(),
                                                   child[1]->get_rtree(),
                                                   child[2]->get_rtree())
                                   );
  default:
    assert(0 == "wrong operation type!");
  }  
}

void netlist::Operation::scan_vars(shared_ptr<SDFG::RForest> rf, bool ctl) const {
  switch(otype) {
  case oVar: {
    get_var().scan_vars(rf, ctl);
    break;
  }
  case oCon: {
    get_con().scan_vars(rf, ctl);
    break;
  }
  case oNULL:
  case oNum: break;
  case oFun: {
    get_fun().scan_vars(rf, ctl);
    break;
  }
  case oUPos:
  case oUNeg:
  case oULRev:
  case oURev:
  case oUAnd:
  case oUNand:
  case oUOr:
  case oUNor:
  case oUXor:
  case oUNxor: {
    child[0]->scan_vars(rf, ctl);
    break;
  }
  case oPower:
  case oTime:
  case oDiv:
  case oMode:
  case oAdd:
  case oMinus:
  case oRS:
  case oLS:
  case oLRS:
  case oLess:
  case oLe:
  case oGreat:
  case oGe:
  case oEq:
  case oNeq:
  case oCEq:
  case oCNeq:
  case oAnd:
  case oXor:
  case oNxor:
  case oOr:
  case oLAnd:
  case oLOr: {
    child[0]->scan_vars(rf, ctl);
    child[1]->scan_vars(rf, ctl);
    break;
  }
  case oQuestion: {
    child[0]->scan_vars(rf, true);
    child[1]->scan_vars(rf, ctl);
    child[2]->scan_vars(rf, ctl);
    break;    
  }
  default:
    assert(0 == "wrong operation type!");
  }
}

void netlist::Operation::replace_variable(const VIdentifier& var, const Number& num) {
  switch(otype) {
  case oVar: {
    if(get_var() == var) {      // found rand replace
      otype = oNum;
      data = shared_ptr<NetComp>(new Number(num));
      valuable = true;
    } else {
      get_var().replace_variable(var, num);
    }
    break;
  }
  case oCon: {
    get_con().replace_variable(var, num);
    break;
  }
  case oNULL:
  case oNum: break;
  case oFun: {
    get_fun().replace_variable(var, num);
    break;
  }
  case oUPos:
  case oUNeg:
  case oULRev:
  case oURev:
  case oUAnd:
  case oUNand:
  case oUOr:
  case oUNor:
  case oUXor:
  case oUNxor: {
    child[0]->replace_variable(var, num);
    break;
  }
  case oPower:
  case oTime:
  case oDiv:
  case oMode:
  case oAdd:
  case oMinus:
  case oRS:
  case oLS:
  case oLRS:
  case oLess:
  case oLe:
  case oGreat:
  case oGe:
  case oEq:
  case oNeq:
  case oCEq:
  case oCNeq:
  case oAnd:
  case oXor:
  case oNxor:
  case oOr:
  case oLAnd:
  case oLOr: {
    child[0]->replace_variable(var, num);
    child[1]->replace_variable(var, num);
    break;
  }
  case oQuestion: {
    child[0]->replace_variable(var, num);
    child[1]->replace_variable(var, num);
    child[2]->replace_variable(var, num);
    break;    
  }
  default:
    assert(0 == "wrong operation type!");
  }
}

void netlist::Operation::replace_variable(const VIdentifier& var, shared_ptr<Expression> reqn) {
  switch(otype) {
  case oVar: {
    if(get_var() == var) {      // found rand replace
      shared_ptr<Expression> tmp_exp(reqn->deep_copy());
      *this = tmp_exp->get_op();
    }
    break;
  }
  case oCon: {
    get_con().replace_variable(var, reqn);
    break;
  }
  case oNULL:
  case oNum: break;
  case oUPos:
  case oUNeg:
  case oULRev:
  case oURev:
  case oUAnd:
  case oUNand:
  case oUOr:
  case oUNor:
  case oUXor:
  case oUNxor: {
    child[0]->replace_variable(var, reqn);
    break;
  }
  case oPower:
  case oTime:
  case oDiv:
  case oMode:
  case oAdd:
  case oMinus:
  case oRS:
  case oLS:
  case oLRS:
  case oLess:
  case oLe:
  case oGreat:
  case oGe:
  case oEq:
  case oNeq:
  case oCEq:
  case oCNeq:
  case oAnd:
  case oXor:
  case oNxor:
  case oOr:
  case oLAnd:
  case oLOr: {
    child[0]->replace_variable(var, reqn);
    child[1]->replace_variable(var, reqn);
    break;
  }
  case oQuestion: {
    child[0]->replace_variable(var, reqn);
    child[1]->replace_variable(var, reqn);
    child[2]->replace_variable(var, reqn);
    break;    
  }
  default:
    assert(0 == "wrong operation type!");
  }
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

void netlist::Operation::reduce_Fun() {
  assert(child.size() == 0);
  SP_CAST(m, FuncCall, data);
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
  m->reduce();
  if(m->is_valuable()) {
    data.reset(new Number(m->get_value()));
    otype = oNum;
    valuable = true;
  }  
}

// unary +
void netlist::Operation::reduce_UPos() {
  assert(child.size() == 1);
  assert(child[0]);
  child[0]->reduce();
  *this = *(child[0]);
  return;
}

// unary -
void netlist::Operation::reduce_UNeg() {
  assert(child.size() == 1);
  assert(child[0]);
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
  assert(child[0]);
  child[0]->reduce();
  if(child[0]->is_valuable()) {
    data.reset(new Number(!(child[0]->get_num())));
    child.clear();
    otype = oNum;
    valuable = true;
  } else {                      // other type of reduction
    switch(child[0]->otype) {
    case oULRev:
      *this = *(child[0]->child[0]);
      break;
    default:
      /* nothing to do */;
    }
  }
}

// unary ~
void netlist::Operation::reduce_URev() {
  assert(child.size() == 1);
  assert(child[0]);
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
  assert(child[0]);
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
  assert(child[0]);
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
  assert(child[0]);
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
  assert(child[0]);
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
  assert(child[0]);
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
  assert(child[0]);
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
  assert(child[0]);
  assert(child[1]);
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
  assert(child[0]);
  assert(child[1]);
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
  assert(child[0]);
  assert(child[1]);
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
  assert(child[0]);
  assert(child[1]);
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
  assert(child[0]);
  assert(child[1]);
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
  assert(child[0]);
  assert(child[1]);
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
  assert(child[0]);
  assert(child[1]);
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
  assert(child[0]);
  assert(child[1]);
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
  assert(child[0]);
  assert(child[1]);
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
  assert(child[0]);
  assert(child[1]);
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
  assert(child[0]);
  assert(child[1]);
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
  assert(child[0]);
  assert(child[1]);
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
  assert(child[0]);
  assert(child[1]);
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
  assert(child[0]);
  assert(child[1]);
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
  assert(child[0]);
  assert(child[1]);
  child[0]->reduce();
  child[1]->reduce();
  if(child[0]->is_valuable() && child[1]->is_valuable()) {
    data.reset(new Number(child[0]->get_num() != child[1]->get_num()));
    child.clear();
    otype = oNum;
    valuable = true;
  }
}

// ===
void netlist::Operation::reduce_CEq() {
  assert(child.size() == 2);
  assert(child[0]);
  assert(child[1]);
  child[0]->reduce();
  child[1]->reduce();
  if(child[0]->is_valuable() && child[1]->is_valuable()) {
    data.reset(new Number(op_case_equal(child[0]->get_num(), child[1]->get_num())));
    child.clear();
    otype = oNum;
    valuable = true;
  }
}

// !==
void netlist::Operation::reduce_CNeq() {
  assert(child.size() == 2);
  assert(child[0]);
  assert(child[1]);
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
  assert(child[0]);
  assert(child[1]);
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
  assert(child[0]);
  assert(child[1]);
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
  assert(child[0]);
  assert(child[1]);
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
  assert(child[0]);
  assert(child[1]);
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
  assert(child[0]);
  assert(child[1]);
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
  assert(child[0]);
  assert(child[1]);
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
  assert(child[0]);
  assert(child[1]);
  assert(child[2]);
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


list<OpPair> netlist::Operation::breakToCases() const {
  //std::cout << "breakToCases: " << *this << std::endl;
  list<OpPair> rv;
  switch(otype) {
  case oNum:
  case oVar:
  case oCon:
  case oFun: 
    rv.push_back(OpPair(shared_ptr<Operation>(), shared_ptr<Operation>(deep_copy())));
    break;
  case oUPos: 
    rv = child[0]->breakToCases(); 
    break;
  case oUNeg: 
  case oULRev: 
  case oURev:
  case oUAnd:
  case oUNand:
  case oUOr:
  case oUNor:
  case oUXor:
  case oUNxor: {
    BOOST_FOREACH(OpPair p, child[0]->breakToCases()) {
      p.second.reset(new Operation(otype, p.second));
      p.second->reduce();
      rv.push_back(p);
    }
    break;
  }
  case oPower:   
  case oTime:   
  case oDiv:   
  case oMode:   
  case oAdd:   
  case oMinus: 
  case oRS:
  case oLS:
  case oLRS:
  case oLess:
  case oLe:
  case oGreat:
  case oGe:
  case oEq:
  case oNeq:
  case oCEq:
  case oCNeq:
  case oAnd:
  case oXor:
  case oNxor:
  case oOr:
  case oLAnd:
  case oLOr: {
    list<OpPair> lop1 = child[1]->breakToCases();
    BOOST_FOREACH(OpPair p0, child[0]->breakToCases()) {
      if(p0.first) {
        BOOST_FOREACH(OpPair p1, lop1) {
          if(p1.first) {
            shared_ptr<Operation> np0_1(p0.first->deep_copy());
            shared_ptr<Operation> np0_2(p0.second->deep_copy());
            shared_ptr<Operation> np1_1(p1.first->deep_copy());
            shared_ptr<Operation> np1_2(p1.second->deep_copy());
            shared_ptr<Operation> np_1(new Operation(oLAnd, np0_1, np1_1));
            shared_ptr<Operation> np_2(new Operation(otype, np0_2, np1_2));
            np_1->reduce();
            np_2->reduce();
            rv.push_back(OpPair(np_1, np_2));
          } else {
            shared_ptr<Operation> np0_1(p0.first->deep_copy());
            shared_ptr<Operation> np0_2(p0.second->deep_copy());
            shared_ptr<Operation> np1_2(p1.second->deep_copy());
            shared_ptr<Operation> np_2(new Operation(otype, np0_2, np1_2));
            np_2->reduce();
            rv.push_back(OpPair(np0_1, np_2));            
          }
        }
      } else {
        BOOST_FOREACH(OpPair p1, lop1) {
          if(p1.first) {
            shared_ptr<Operation> np0_2(p0.second->deep_copy());
            shared_ptr<Operation> np1_1(p1.first->deep_copy());
            shared_ptr<Operation> np1_2(p1.second->deep_copy());
            shared_ptr<Operation> np_2(new Operation(otype, np0_2, np1_2));
            np_2->reduce();
            rv.push_back(OpPair(np1_1, np_2));
          } else {
            shared_ptr<Operation> np0_2(p0.second->deep_copy());
            shared_ptr<Operation> np1_2(p1.second->deep_copy());
            shared_ptr<Operation> np_2(new Operation(otype, np0_2, np1_2));
            np_2->reduce();
            rv.push_back(OpPair(p1.first, np_2));            
          }
        }
      }
    }
    break;
  }
  case oQuestion: {
    list<OpPair> op0 = child[0]->breakToCases();
    list<OpPair> op1 = child[1]->breakToCases();
    list<OpPair> op2 = child[2]->breakToCases();
    
    BOOST_FOREACH(OpPair A, op0) {
      BOOST_FOREACH(OpPair B, op1) {
        shared_ptr<Operation> d(B.second->deep_copy());
        shared_ptr<Operation> bAs(A.second->deep_copy());
        shared_ptr<Operation> c;
        if(A.first) {
          shared_ptr<Operation> bAf(A.first->deep_copy());
          c.reset(new Operation(oLAnd, bAf, bAs));
          if(B.first) {  // A.f && A.s && B.f  -> B.s
            shared_ptr<Operation> bBf(B.first->deep_copy());
            c.reset(new Operation(oLAnd, c, bBf));
          } // else A.f && A.s  -> B.s     
        } else {
          if(B.first) {  // A.s && B.f -> B.s
            shared_ptr<Operation> bBf(B.first->deep_copy());
            c.reset(new Operation(oLAnd, bAs, bBf));
          } else {       // A.s -> B.s
            c = bAs;
          }
        }
        rv.push_back(OpPair(c,d));
      }
      
      BOOST_FOREACH(OpPair B, op2) {
        shared_ptr<Operation> d(B.second->deep_copy());
        shared_ptr<Operation> c;
        if(A.first) {
          shared_ptr<Operation> bAf(A.first->deep_copy());
          c = bAf;
          if(B.first) {  // A.f && B.f  -> B.s
            shared_ptr<Operation> bBf(B.first->deep_copy());
            c.reset(new Operation(oLAnd, c, bBf));
          } // else A.f -> B.s     
        } else {
          if(B.first) {  // B.f -> B.s
            shared_ptr<Operation> bBf(B.first->deep_copy());
            c = bBf;
          } // else B.s
        }
        rv.push_back(OpPair(c,d));
      }
    }
    break;
  }
  default:  // should not run to here
    assert(0 == "wrong operation type");
  }

  //std::cout << "result: " << std::endl;
  //BOOST_FOREACH(OpPair p, rv) {
  //  if(p.first)
  //    std::cout << "[" << *(p.first) << "; " << *(p.second) << "]" << std::endl;
  //  else
  //    std::cout << "[" << "; " << *(p.second) << "]" << std::endl;
  //}
  return rv;
}
