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
 * 10/02/2012   Wei Song (29's birth day :-) )
 *
 *
 */

#ifndef _H_OPERATION_
#define _H_OPERATION_

namespace netlist {

  class Operation {
  public:
    enum operation_t {
      oNULL,                      /* no operation */
      oNum,                       /* number */
      oVar,                       /* variable */
      oCon,                       /* concatenation */
      oFun,                       /* function call */
      oUPos,                      /* unary + */
      oUNeg,                      /* unary - */
      oULRev,                     /* ! */
      oURev,                      /* ~ */
      oUAnd,                      /* unary & */
      oUNand,                     /* unary ~& */
      oUOr,                       /* unary | */
      oUNor,                      /* unary ~| */
      oUXor,                      /* unary ^ */
      oUNxor,                     /* unary ~^ */
      oPower   = oUNxor + 10,     /* ** */
      oTime    = oPower + 10,     /* * */
      oDiv,                       /* / */
      oMode,                      /* % */
      oAdd     = oMode + 10,      /* + */
      oMinus,                     /* - */
      oRS      = oMinus + 10,     /* >> */
      oLS,                        /* << */
      oLRS,                       /* >>> */
      oLess    = oLRS + 10,       /* < */
      oLe,                        /* <= */
      oGreat,                     /* > */
      oGe,                        /* >= */
      oEq      = oGe + 10,        /* == */
      oNeq,                       /* != */
      oCEq,                       /* === */
      oCNeq,                      /* !== */
      oAnd     = oCNeq + 10,      /* & */
      oXor     = oAnd + 10,       /* ^ */
      oNxor,                      /* ~^ */
      oOr      = oNxor + 10,      /* | */
      oLAnd    = oOr + 10,        /* && */
      oLOr     = oLAnd + 10,      /* || */
      oQuestion = oLOr + 10       /* ? : */
    };

    Operation();
    Operation(operation_t);
    Operation(shared_ptr<Number>);
    Operation(shared_ptr<Identifier>);
    //    Operation(shared_ptr<FuncCall>);
    //    Operation(shared_ptr<Concatenation>);

    // helpers
    bool is_valuable() const { return valuable; }
    operation_t get_type() const { return otype; }
    Number& get_num_ref();
    Number get_num() const;
    shared_ptr<Concatenation> get_con() const;
    Identifier get_var() const;
    ostream& streamout(ostream&) const;

  private:
    operation_t otype;
    bool valuable;
    shared_ptr<NetComp> data;
  };

  NETLIST_STREAMOUT(Operation);

  void execute_operation( Operation::operation_t op,
			  list<Operation>& d1,
			  list<Operation>& d2,
			  list<Operation>& d3
			  );

  void execute_UPos(list<Operation>&);
  void execute_UNeg(list<Operation>&);
  void execute_Add(list<Operation>&, list<Operation>&);
 

}

#endif
