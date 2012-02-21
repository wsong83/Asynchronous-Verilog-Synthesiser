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
     * or a variable.
     * So the whole structure is recursive.
     */
  public:
    ConElem(const shared_ptr<Expression>& exp, const list<ConElem>& elems)
      : con(exp, elems) {}
    ConElem(const shared_ptr<Identifier>& var)
      : var(var) {}
    ConElem(const ConElem& rhs)
      : con(rhs.con), var(rhs.var) {}
    
    ConElem& operator= (const ConElem& rhs) {
      con = rhs.con;
      var = rhs.var;
      return *this;
    }

    ConElem& operator= (const pair<shared_ptr<Expression>, list<ConElem> >& rhs) {
      con = rhs;
      var.reset();
      return *this;
    }

    ConElem& operator= (const shared_ptr<Identifier>& rhs) {
      con.first.reset();
      con.second.clear();
      var = rhs;
      return *this;
    }

    pair<shared_ptr<Expression>, list<ConElem> > con;
    shared_ptr<Identifier> var;

  private:
    ConElem();                  /* no default constructor */
  };

  class Concatenation : public NetComp {
  public:
    // constructors
    NETLIST_DEFAULT_CON(Concatenation, tConcatenation);
    
    // helpers
    ostream& streamout(ostream&);


    // data
    list<ConElem> data;

  };

}

#endif
