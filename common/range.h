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
 * Range declaration and definition
 * 08/02/2011   Wei Song
 *
 *
 */

#ifndef _H_RANGE_
#define _H_RANGE_

#include <utility>
#include "component.h"

namespace netlist {

  typedef std::pair<Expression, Expression> Range_Exp;

  class Range : public NetComp {
  public:
    // constructors
    Range(int);			/* select by a fix number */
    Range(const Expression&);	/* select by an expression  */
    Range(const Range_Exp&);    /* declare or select by a range expression */
    Range(const Range_Exp&, int); /* select by a range expression using positive or negtive colon */
    ~Range();

    // helpers
    bool is_valuable();
    bool is_single();
    virtual std::ostream& streamout(std::ostream&) const;
    
  private:
    union {
      int c;			/* constant */
      Expression* v;		/* variable */
      Range_Exp* r;		/* range expression */
    };

    enum type_t {
      TConst, 
      TVar, 
      TRange} type;
    

  };

  NETLIST_STREAMOUT(Range)

}

#endif
