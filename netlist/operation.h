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
 * 10/02/2012   Wei Song (29's birth day :-) )
 *
 *
 */

#ifndef AV_H_OPERATION_
#define AV_H_OPERATION_

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

    // constructors
    Operation();
    Operation(operation_t);
    Operation(const Number&);
    Operation(const VIdentifier&);
    Operation(const boost::shared_ptr<Concatenation>&);
    Operation(const boost::shared_ptr<LConcatenation>&);

    // helpers
    bool is_valuable() const { return valuable; }
    operation_t get_type() const { return otype; }
    Number& get_num();
    Number get_num() const;
    Concatenation& get_con();
    VIdentifier& get_var();
    void reduce();
    
    // inherit from NetComp
    NETLIST_SET_FATHER_DECL;
    NETLIST_CHECK_INPARSE_DECL;
    NETLIST_STREAMOUT_DECL;
    virtual Operation* deep_copy() const; 
    virtual void db_register(int iod = 1);
    virtual void db_expunge();

  private:
    operation_t otype;
    bool valuable;
    boost::shared_ptr<NetComp> data;
    Block* father;
  };

  NETLIST_STREAMOUT(Operation);

  void execute_operation( Operation::operation_t op,
                          std::list<boost::shared_ptr<Operation> >& d1,
                          std::list<boost::shared_ptr<Operation> >& d2,
                          std::list<boost::shared_ptr<Operation> >& d3
                          );

  void execute_UPos(std::list<boost::shared_ptr<Operation> >&);
  void execute_UNeg(std::list<boost::shared_ptr<Operation> >&);
  void execute_ULRev(std::list<boost::shared_ptr<Operation> >&); /* ! */
  void execute_URev(std::list<boost::shared_ptr<Operation> >&);  /* ~ */
  void execute_UAnd(std::list<boost::shared_ptr<Operation> >&);  /* unary & */
  void execute_UNand(std::list<boost::shared_ptr<Operation> >&); /* unary ~& */
  void execute_UOr(std::list<boost::shared_ptr<Operation> >&);   /* unary | */
  void execute_UNor(std::list<boost::shared_ptr<Operation> >&);  /* unary ~| */
  void execute_UXor(std::list<boost::shared_ptr<Operation> >&);  /* unary ^ */
  void execute_UNxor(std::list<boost::shared_ptr<Operation> >&); /* unary ~^ */
  void execute_Power(std::list<boost::shared_ptr<Operation> >&, std::list<boost::shared_ptr<Operation> >&); /* ** */
  void execute_Time(std::list<boost::shared_ptr<Operation> >&, std::list<boost::shared_ptr<Operation> >&);  /* * */
  void execute_Div(std::list<boost::shared_ptr<Operation> >&, std::list<boost::shared_ptr<Operation> >&);   /* / */
  void execute_Mode(std::list<boost::shared_ptr<Operation> >&, std::list<boost::shared_ptr<Operation> >&);  /* % */
  void execute_Add(std::list<boost::shared_ptr<Operation> >&, std::list<boost::shared_ptr<Operation> >&);   /* + */
  void execute_Minus(std::list<boost::shared_ptr<Operation> >&, std::list<boost::shared_ptr<Operation> >&); /* - */
  void execute_RS(std::list<boost::shared_ptr<Operation> >&, std::list<boost::shared_ptr<Operation> >&);    /* >> */
  void execute_LS(std::list<boost::shared_ptr<Operation> >&, std::list<boost::shared_ptr<Operation> >&);    /* << */
  void execute_LRS(std::list<boost::shared_ptr<Operation> >&, std::list<boost::shared_ptr<Operation> >&);   /* >>> */
  void execute_Less(std::list<boost::shared_ptr<Operation> >&, std::list<boost::shared_ptr<Operation> >&);  /* < */
  void execute_Le(std::list<boost::shared_ptr<Operation> >&, std::list<boost::shared_ptr<Operation> >&);    /* <= */
  void execute_Great(std::list<boost::shared_ptr<Operation> >&, std::list<boost::shared_ptr<Operation> >&); /* > */
  void execute_Ge(std::list<boost::shared_ptr<Operation> >&, std::list<boost::shared_ptr<Operation> >&);    /* >= */
  void execute_Eq(std::list<boost::shared_ptr<Operation> >&, std::list<boost::shared_ptr<Operation> >&);    /* == */
  void execute_Neq(std::list<boost::shared_ptr<Operation> >&, std::list<boost::shared_ptr<Operation> >&);   /* != */
  void execute_CEq(std::list<boost::shared_ptr<Operation> >&, std::list<boost::shared_ptr<Operation> >&);   /* === */
  void execute_CNeq(std::list<boost::shared_ptr<Operation> >&, std::list<boost::shared_ptr<Operation> >&);  /* !== */
  void execute_And(std::list<boost::shared_ptr<Operation> >&, std::list<boost::shared_ptr<Operation> >&);   /* & */
  void execute_Xor(std::list<boost::shared_ptr<Operation> >&, std::list<boost::shared_ptr<Operation> >&);   /* ^ */
  void execute_Nxor(std::list<boost::shared_ptr<Operation> >&, std::list<boost::shared_ptr<Operation> >&);  /* ~^ */
  void execute_Or(std::list<boost::shared_ptr<Operation> >&, std::list<boost::shared_ptr<Operation> >&);    /* | */
  void execute_LAnd(std::list<boost::shared_ptr<Operation> >&, std::list<boost::shared_ptr<Operation> >&);  /* && */
  void execute_LOr(std::list<boost::shared_ptr<Operation> >&, std::list<boost::shared_ptr<Operation> >&);   /* || */
  void execute_Question(std::list<boost::shared_ptr<Operation> >&, std::list<boost::shared_ptr<Operation> >&, std::list<boost::shared_ptr<Operation> >&); /* ? : */

}

#endif
