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
 * A function call in an expression
 * 28/02/2013   Wei Song
 *
 *
 */

#ifndef AV_H_FUNC_CALL
#define AV_H_FUNC_CALL

namespace netlist {

  class FuncCall : public NetComp {
  public:
    FuncCall();
    FuncCall(const shell::location&, const FIdentifier&, const std::list<boost::shared_ptr<Expression> >&);

    // inherit from NetComp
    NETLIST_SET_FATHER_DECL;
    NETLIST_STREAMOUT_DECL;
    virtual FuncCall* deep_copy() const;
    NETLIST_DB_DECL;
    NETLIST_REPLACE_VARIABLE;
    virtual boost::shared_ptr<SDFG::RTree> get_rtree() const;

    // helpers
    void reduce();
    Number get_value() const;
    bool is_valuable() const;

    // data
    FIdentifier fname;          // function name
    std::list<boost::shared_ptr<Expression> > args; // arguments

  private:
    bool valuable;

  };

  NETLIST_STREAMOUT(FuncCall);
  
}

#endif

// Local Variables:
// mode: c++
// End:
