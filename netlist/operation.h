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
 * 10/02/2012   Wei Song (29's birth day :-) )
 *
 *
 */

#ifndef AV_H_OPERATION_
#define AV_H_OPERATION_

#include <set>

namespace netlist {

  class Operation : public NetComp {
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

    // constructors
    Operation();
    Operation(operation_t);
    Operation(const shell::location&, operation_t);
    Operation(const Number&);
    Operation(const shell::location&, const Number&);
    Operation(const VIdentifier&);
    Operation(const shell::location&, const VIdentifier&);
    Operation(const boost::shared_ptr<Concatenation>&);
    Operation(const shell::location&, const boost::shared_ptr<Concatenation>&);
    Operation(const boost::shared_ptr<LConcatenation>&);
    Operation(const shell::location&, const boost::shared_ptr<LConcatenation>&);
    Operation(const shell::location&, const boost::shared_ptr<FuncCall>&);
    Operation(const shell::location&, operation_t, 
              const boost::shared_ptr<Operation>&); // (op)(exp)
    Operation(operation_t, const boost::shared_ptr<Operation>&); // (op)(exp)
    Operation(operation_t, const boost::shared_ptr<Operation>&, 
              const boost::shared_ptr<Operation>&); // (exp1)(op)(exp2)
    Operation(const shell::location&, operation_t, const boost::shared_ptr<Operation>&, 
              const boost::shared_ptr<Operation>&); // (exp1)(op)(exp2)
    Operation(operation_t, const boost::shared_ptr<Operation>&, 
              const boost::shared_ptr<Operation>&,
              const boost::shared_ptr<Operation>&); // (exp1) ? (exp2) : (exp3)
    Operation(const shell::location&, operation_t, const boost::shared_ptr<Operation>&, 
              const boost::shared_ptr<Operation>&,
              const boost::shared_ptr<Operation>&); // (exp1) ? (exp2) : (exp3)
    
    virtual ~Operation();

    // helpers
    bool is_valuable() const { return valuable; }
    operation_t get_type() const { return otype; }
    Number& get_num();
    const Number& get_num() const;
    Concatenation& get_con();
    const Concatenation& get_con() const;
    VIdentifier& get_var();
    const VIdentifier& get_var() const;
    void reduce();
    std::string toString() const;
    
    // inherit from NetComp
    NETLIST_SET_FATHER_DECL;
    NETLIST_STREAMOUT_DECL;
    virtual Operation* deep_copy() const; 
    NETLIST_DB_DECL;
    NETLIST_SCAN_VARS;
    NETLIST_REPLACE_VARIABLE;

  private:
    operation_t otype;
    bool valuable;
    boost::shared_ptr<NetComp> data;
    std::vector<boost::shared_ptr<Operation> > child; // expression tree structure

    // helper function to reduce the expression
    void reduce_Num();
    void reduce_Var();
    void reduce_Con();
    void reduce_Fun();
    void reduce_UPos();
    void reduce_UNeg();
    void reduce_ULRev();
    void reduce_URev();
    void reduce_UAnd();
    void reduce_UNand();
    void reduce_UOr();
    void reduce_UNor();
    void reduce_UXor();
    void reduce_UNxor();
    void reduce_Power();
    void reduce_Time();
    void reduce_Div();
    void reduce_Mode();
    void reduce_Add();
    void reduce_Minus();
    void reduce_RS();
    void reduce_LS();
    void reduce_LRS();
    void reduce_Less();
    void reduce_Le();
    void reduce_Great();
    void reduce_Ge();
    void reduce_Eq();
    void reduce_Neq();
    void reduce_CEq();
    void reduce_CNeq();
    void reduce_And();
    void reduce_Xor();
    void reduce_Nxor();
    void reduce_Or();
    void reduce_LAnd();
    void reduce_LOr();
    void reduce_Question();
  };

  NETLIST_STREAMOUT(Operation);

}

#endif

// Local Variables:
// mode: c++
// End:
