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
 * Variable
 * 27/02/2012   Wei Song
 *
 *
 */

#ifndef _H_VA_REGISTER_
#define _H_VA_REGISTER_

namespace netlist {
  
  class Variable : public NetComp {
  public:
    Variable() : NetComp(tVariable), uid({0,0}) {}
    Variable(const VIdentifier& id): NetComp(tVariable), name(id), uid({0,0}) {}
    Variable(const VIdentifier& id, const Expression& expp)
      : NetComp(tVariable), name(id), uid({0,0}), exp(expp) {}

    void set_value(const Number&); /* reset the value of this variable */
    void update();  /* recalculate the value and update all fanouts */
    ostream& streamout(ostream& os) const;

    VIdentifier name;
    map<unsigned int, VIdentifier *> fan[2]; /* fan[0] for fanin, fan[1] for fanout */
    unsigned int get_id(int);

  private:
    unsigned int uid[2];
    Expression exp;

  };

  NETLIST_STREAMOUT(Variable);


}

#endif
