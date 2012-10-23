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
 * for statements
 * 27/03/2012   Wei Song
 *
 *
 */

#ifndef AV_H_AV_FOR_
#define AV_H_AV_FOR_

namespace netlist {

  class ForState : public NetComp {
  public:
    // constructors
    ForState() : NetComp(tFor), named(false) {}
    ForState(const shell::location& lloc) : NetComp(tFor, lloc), named(false) {}
    ForState(
             const boost::shared_ptr<Assign>& init, 
             const boost::shared_ptr<Expression>& cond, 
             const boost::shared_ptr<Assign>& incr, 
             const boost::shared_ptr<Block>& body
             );
    ForState(
             const shell::location& lloc,
             const boost::shared_ptr<Assign>& init, 
             const boost::shared_ptr<Expression>& cond, 
             const boost::shared_ptr<Assign>& incr, 
             const boost::shared_ptr<Block>& body
             );

    // inherit from NetComp
    NETLIST_STREAMOUT_DECL;
    NETLIST_SET_FATHER_DECL;
    NETLIST_CHECK_INPARSE_DECL;
    virtual ForState* deep_copy() const;
    virtual void db_register(int iod = 1);
    virtual void db_expunge();
    NETLIST_SET_ALWAYS_POINTER_DECL;
    NETLIST_REPLACE_VARIABLE;

    // helpers
    void set_name(const BIdentifier& nm) { name = nm; named = true;}
    bool is_named() const { return named; }

    //data
    boost::shared_ptr<Assign> init;
    boost::shared_ptr<Expression> cond;
    boost::shared_ptr<Assign> incr;
    boost::shared_ptr<Block> body;

    BIdentifier name;           /* dummy name for index search */

  private:
    bool named;

  };

  NETLIST_STREAMOUT(ForState);

}

#endif
