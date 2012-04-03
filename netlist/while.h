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
 * while statements
 * 27/03/2012   Wei Song
 *
 *
 */

#ifndef _H_AV_WHILE_
#define _H_AV_WHILE_

namespace netlist {

  class WhileState : public NetComp {
  public:
    // constructors
    NETLIST_DEFAULT_CON(WhileState, tWhile);
    WhileState(const shared_ptr<Expression>& exp, const shared_ptr<SeqBlock>& body);

    // inherit from NetComp
    NETLIST_STREAMOUT_FUN_DECL;

    //data
    shared_ptr<Expression> exp;
    list<shared_ptr<NetComp> > statements;

  };
  
  NETLIST_STREAMOUT(WhileState);

}

#endif
