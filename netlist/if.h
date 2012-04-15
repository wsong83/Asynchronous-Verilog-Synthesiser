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
 * If statements
 * 27/03/2012   Wei Song
 *
 *
 */

#ifndef _H_AV_IF_
#define _H_AV_IF_

namespace netlist {

  class IfState : public NetComp {
  public:
    // constructors
    IfState(
            const shared_ptr<Expression>& exp, 
            const shared_ptr<Block>& m_ifcase, 
            const shared_ptr<Block>& m_elsecase
            );

    IfState(
            const location& lloc,
            const shared_ptr<Expression>& exp, 
            const shared_ptr<Block>& m_ifcase, 
            const shared_ptr<Block>& m_elsecase
            );

    IfState(
            const shared_ptr<Expression>& exp, 
            const shared_ptr<Block>& m_ifcase 
            );

    IfState(
            const location& lloc,
            const shared_ptr<Expression>& exp, 
            const shared_ptr<Block>& m_ifcase 
            );

    // helpers
    virtual ostream& streamout(ostream&, unsigned int, bool) const; /* the streamout with first line prefix control */
    void set_name(const BIdentifier& nm) { name = nm; named = true;}
    bool is_named() const { return named; }

    // inherit from NetComp
    NETLIST_STREAMOUT_FUN_DECL;

    //data
    shared_ptr<Expression> exp; /* the condition expression */
    shared_ptr<Block> ifcase; /* the block to run when expression is true */
    shared_ptr<Block> elsecase; /* the block to run when else is used */
    BIdentifier name;

  private:
    bool named;

  };

  NETLIST_STREAMOUT(IfState);

}

#endif
