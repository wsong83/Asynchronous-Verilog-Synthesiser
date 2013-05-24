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
 * Concatenation
 * 20/02/2012   Wei Song
 *
 *
 */

#ifndef AV_H_CONCATENATION_
#define AV_H_CONCATENATION_

#include <set>

namespace netlist {

  class ConElem {               /* element of a concatenation */
    /* A concatenation is a list<ConElem>
     * while every element is <exprssion, concatenation>,
     * representing something like {3{a,b}},
     * or an expression.
     * So the whole structure is recursive.
     */
  public:
    ConElem();
    ConElem(const shell::location& lloc);
    ConElem(const boost::shared_ptr<Expression>& expr, const std::list<boost::shared_ptr<ConElem> >& elems);
    ConElem(const shell::location& lloc, const boost::shared_ptr<Expression>& expr, 
            const std::list<boost::shared_ptr<ConElem> >& elems);
    ConElem(const boost::shared_ptr<Expression>& expr);
    ConElem(const shell::location& lloc, const boost::shared_ptr<Expression>& expr);
    
    // helpers
    void reduce();
    bool is_valuable() const { return con.size() == 0 && exp->is_valuable(); }
    Number get_value() { return exp->get_value(); }

    // inherit from NetComp
    NETLIST_STREAMOUT_DECL;
    NETLIST_SET_FATHER_DECL;
    virtual ConElem* deep_copy() const;
    NETLIST_DB_DECL;
    NETLIST_REPLACE_VARIABLE;
    void replace_variable(const VIdentifier&, boost::shared_ptr<Expression>); // replace a vid with an expression     
    virtual boost::shared_ptr<SDFG::RTree> get_rtree() const;

    boost::shared_ptr<Expression> exp;
    std::list<boost::shared_ptr<ConElem> > con;
    shell::location loc;
    Block* father;
    unsigned int width;

  };
  NETLIST_STREAMOUT(ConElem);

  class Concatenation : public NetComp {
  public:
    // constructors
    NETLIST_DEFAULT_CON(Concatenation, tConcatenation);
    NETLIST_DEFAULT_CON_WL(Concatenation, tConcatenation);
    
    // helpers
    Concatenation& operator+ (const boost::shared_ptr<Concatenation>& rhs);
    Concatenation& operator+ (const boost::shared_ptr<ConElem>& rhs);
    void reduce();
    bool is_valuable() const { return (data.size() == 1 && data.front()->is_valuable()); }
    bool is_exp() const { return (data.size() == 1 && data.front()->con.size() == 0); }
    boost::shared_ptr<Expression>& get_exp() { return data.front()->exp; }
    Number get_value() { return data.front()->get_value(); }

    // inherit from NetComp
    NETLIST_STREAMOUT_DECL;
    NETLIST_SET_FATHER_DECL;
    virtual Concatenation* deep_copy() const;
    NETLIST_DB_DECL;
    NETLIST_REPLACE_VARIABLE;
    void replace_variable(const VIdentifier&, boost::shared_ptr<Expression>); // replace a vid with an expression 
    virtual boost::shared_ptr<SDFG::RTree> get_rtree() const;

    // data
    std::list<boost::shared_ptr<ConElem> > data;

  };
  NETLIST_STREAMOUT(Concatenation);
}

#endif

// Local Variables:
// mode: c++
// End:
