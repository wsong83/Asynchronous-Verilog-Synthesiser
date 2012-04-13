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
 * Concatenation
 * 20/02/2012   Wei Song
 *
 *
 */

#ifndef _H_CONCATENATION_
#define _H_CONCATENATION_

namespace netlist {

  class ConElem {               /* element of a concatenation */
    /* A concatenation is a list<ConElem>
     * while every element is <exprssion, concatenation>,
     * representing something like {3{a,b}},
     * or an expression.
     * So the whole structure is recursive.
     */
  public:
    ConElem() {}
    ConElem(const location& lloc) : loc(lloc) {}
    ConElem(const shared_ptr<Expression>& expr, const list<shared_ptr<ConElem> >& elems)
      : exp(expr), con(elems) {}
    ConElem(const location& lloc, const shared_ptr<Expression>& expr, const list<shared_ptr<ConElem> >& elems)
      : exp(expr), con(elems), loc(lloc) {}
    ConElem(const shared_ptr<Expression>& expr)
      : exp(expr) {}
    ConElem(const location& lloc, const shared_ptr<Expression>& expr)
      : exp(expr), loc(lloc) {}
    
    void reduce();
    void db_register(int iod = 1);
    void db_expunge();
    NETLIST_STREAMOUT_FUN_DECL;
    virtual ConElem* deep_copy() const;

    shared_ptr<Expression> exp;
    list<shared_ptr<ConElem> > con;
    location loc;

  };
  NETLIST_STREAMOUT(ConElem);

  class Concatenation : public NetComp {
  public:
    // constructors
    NETLIST_DEFAULT_CON(Concatenation, tConcatenation);
    NETLIST_DEFAULT_CON_WL(Concatenation, tConcatenation);
    
    // helpers
    Concatenation& operator+ (shared_ptr<Concatenation>& rhs);
    Concatenation& operator+ (shared_ptr<ConElem>& rhs);
    void reduce();
    void db_register(int iod = 1);
    void db_expunge();

    // inherit from NetComp
    NETLIST_STREAMOUT_FUN_DECL;
    virtual Concatenation* deep_copy() const;

    // data
    list<shared_ptr<ConElem> > data;

  };
  NETLIST_STREAMOUT(Concatenation);
}

#endif
