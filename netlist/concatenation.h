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
    ConElem(const shared_ptr<Expression>& expr, const list<ConElem>& elems)
      : con(expr, elems) {}
    ConElem(const shared_ptr<Expression>& expr)
      : exp(expr) {}
    ConElem(const ConElem& rhs)
      : con(rhs.con), exp(rhs.exp) {}
    
    ConElem& operator= (const ConElem& rhs);
    ConElem& operator= (const pair<shared_ptr<Expression>, list<ConElem> >& rhs);
    ConElem& operator= (const shared_ptr<Expression>& rhs);
    ConElem deep_copy() const;

    void reduce();
    ostream& streamout(ostream&) const;

    pair<shared_ptr<Expression>, list<ConElem> > con;
    shared_ptr<Expression> exp;

  private:
    ConElem();                  /* no default constructor */
  };
  NETLIST_STREAMOUT(ConElem);

  class Concatenation : public NetComp {
  public:
    // constructors
    NETLIST_DEFAULT_CON(Concatenation, tConcatenation);
    
    // helpers
    ostream& streamout(ostream&) const;
    Concatenation& operator+ (const Concatenation& rhs);
    Concatenation& operator+ (const ConElem& rhs);
    void reduce();

    // data
    list<ConElem> data;

  };
  NETLIST_STREAMOUT(Concatenation);
}

#endif
