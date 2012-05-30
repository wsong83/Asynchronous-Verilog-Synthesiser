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
 * Expressions
 * 08/02/2012   Wei Song
 *
 *
 */

#ifndef AV_H_EXPRESSION_
#define AV_H_EXPRESSION_

namespace netlist {
  
  class Expression : public NetComp {
  public:
    // constructors
    Expression() : NetComp(tExp), valuable(false) {}
    Expression(const shell::location& lloc) : NetComp(tExp, lloc), valuable(false) {}
    Expression(const Number&);	/* a number is an expression */
    Expression(const shell::location& lloc, const Number&);	/* a number is an expression */
    Expression(const VIdentifier&); /* a variable/parameter is an expression */
    Expression(const shell::location& lloc, const VIdentifier&); /* a variable/parameter is an expression */
    Expression(const boost::shared_ptr<Concatenation>&); /* a concatenation is an expression */
    Expression(const shell::location& lloc, const boost::shared_ptr<Concatenation>&); /* a concatenation is an expression */
    Expression(const boost::shared_ptr<LConcatenation>&); /* some times need to convert a lvalue back to expression */
    Expression(const shell::location& lloc, const boost::shared_ptr<LConcatenation>&); /* some times need to convert a lvalue back to expression */

    // helpers
    bool is_valuable() const;    /* check valuable */
    Number get_value() const;    /* fetch the value if valuable */
    void reduce();               /* try to reduce the equation */
    // return the size of equation
    int size() const { return eqn.size(); }
    bool empty() const { return eqn.empty(); }
    boost::shared_ptr<Operation>& front() { return eqn.front(); }
    
    // develope the equation
    void append(Operation::operation_t);
    void append(Operation::operation_t, Expression&);
    void append(Operation::operation_t, Expression&, Expression&);
    void concatenate(const Expression&); /* concatenate the number in two expressions */

    // inherit from NetComp
    NETLIST_STREAMOUT_DECL;
    NETLIST_SET_FATHER_DECL;
    NETLIST_CHECK_INPARSE_DECL;
    virtual Expression* deep_copy() const;
    virtual void db_register(int iod = 1);
    virtual void db_expunge();
    
    // data
    std::list<boost::shared_ptr<Operation> > eqn;

  private:
    bool valuable;
    
  };

  Expression& operator+ (Expression&, Expression&);
  Expression& operator- (Expression&, Expression&);
  bool operator== (const Expression&, const Expression&);

  NETLIST_STREAMOUT(Expression);

  // helper class
  class expression_state {
  public:
    boost::shared_ptr<Operation> op;               // operator
    int ops;                                       // number of operands needed
    int opp;                                       // current number of operands
    std::list<boost::shared_ptr<Operation> > d[3]; // oprands
  expression_state() : opp(0) {}
  };


}

#endif
