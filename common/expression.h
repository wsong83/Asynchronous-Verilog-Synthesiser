/*
 * Copyright (c) 2011 Wei Song <songw@cs.man.ac.uk> 
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
 * 08/02/2011   Wei Song
 *
 *
 */

#ifndef _H_EXPRESSION_
#define _H_EXPRESSION_

#include "component.h"

namespace netlist {

  class Expression : public NetComp {
  public:
    // constructors
    Expression(unsigned int);	/* a const integer is an expression */
    Expression(const Number&);	/* a number is an expression */

    // helpers
    bool is_valuable() const;
    int get_value() const;
    virtual std::ostream& streamout(std::ostream&) const;
    bool operator== (const Expression& rhs) const;

  private:
  };

  Expression operator+ (const Expression&, const Expression&);
  Expression operator- (const Expression&, const Expression&);

  NETLIST_STREAMOUT(Expression)

}

#endif
