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
    Expression();
    Expression(const shell::location& lloc);
    Expression(const Number&);	// a number is an expression
    Expression(const shell::location& lloc, const Number&);	// a number is an expression
    Expression(const VIdentifier&); // a variable/parameter is an expression
    Expression(const shell::location& lloc, const VIdentifier&); // a variable/parameter is an expression
    Expression(const boost::shared_ptr<Concatenation>&); // a concatenation is an expression
    Expression(const shell::location& lloc, const boost::shared_ptr<Concatenation>&); // a concatenation is an expression
    Expression(const boost::shared_ptr<LConcatenation>&); // some times need to convert a lvalue back to expression
    Expression(const shell::location& lloc, const boost::shared_ptr<LConcatenation>&); // some times need to convert a lvalue back to expression
    Expression(const shell::location& lloc, const boost::shared_ptr<FuncCall>&); // a function call
    virtual ~Expression();

    // helpers
    bool is_valuable() const;    // check valuable
    Number get_value() const;    // fetch the value if valuable
    bool is_variable() const;    // check whether is variable
    VIdentifier get_variable() const; // father the variable
    bool is_singular() const;    // the expression is a number, concatenation, variable or function call
    Operation& get_op();             // get the private operation for read/write
    const Operation& get_op() const; // read the private operation
    void reduce();               // try to reduce the equation
    
    // develope the equation
    void append(Operation::operation_t);
    void append(Operation::operation_t, Expression&);
    void append(Operation::operation_t, Expression&, Expression&);
    void concatenate(const Expression&); // concatenate the number in two expressions

    // inherit from NetComp
    NETLIST_STREAMOUT_DECL;
    NETLIST_SET_FATHER_DECL;
    virtual Expression* deep_copy() const;
    NETLIST_DB_DECL;
    NETLIST_SCAN_VARS;
    NETLIST_REPLACE_VARIABLE;
    
  private:
    // data
    boost::shared_ptr<Operation> eqn;
    
  };

  Expression& operator+ (Expression&, Expression&);
  Expression& operator- (Expression&, Expression&);
  bool operator== (const Expression&, const Expression&);

  NETLIST_STREAMOUT(Expression);

}

#endif

// Local Variables:
// mode: c++
// End:
