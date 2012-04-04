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
 * The root class for all netlist component.
 * 04/04/2012   Wei Song
 *
 *
 */

#ifndef _H_AV_NETCOMP_
#define _H_AV_NETCOMP_

// function macro for stream out operator <<
#ifndef NETLIST_STREAMOUT
#define NETLIST_STREAMOUT(COMP)                                \
  inline ostream& operator<< ( ostream& os, const COMP& rhs) { \
    return rhs.streamout(os, 0);                               \
  }
#endif

#ifndef NETLIST_DEFAULT_CON
#define NETLIST_DEFAULT_CON(COMP, CT) COMP() : NetComp(NetComp::CT) { }
#endif

#ifndef NETLIST_STREAMOUT_FUN_DECL
#define NETLIST_STREAMOUT_FUN_DECL                    \
  virtual ostream& streamout (ostream&, unsigned int) const
#endif

#ifndef NETLIST_REF
#define NETLIST_REF(COMP)               \
  virtual COMP* ptr() { return this; }
#endif


namespace netlist{
  
  // the base class of all netlist components
  class NetComp {
  public:
#include "comp_type.h"
    // no one should directly use this class
    NetComp() : ctype(tUnkown) {}
    NetComp(ctype_t tt) : ctype(tt) {}
    
    ctype_t get_type() const { return ctype; }
    ctype_t ctype;
    
    virtual void reduce() {}	/* many netlist component need method to reduce itself */

    // the internal stream out method, to avoid friend declarations
    virtual ostream& streamout (ostream& os, unsigned int indent) const {
      os << "ERROR!!, the streamout() of NetComp is used!!!" << endl;
      assert(0 == "the streamout() of NetComp is used");
      return os;
    }
    
    // deep copy when the content of a shared_ptr must be duplicated
    virtual NetComp* deep_copy() const { /* deep copy a netlist component */
      cout << "ERROR!!, the deep_copy() of NetComp is used!!!" << endl;
      assert(0 == "the deep_copy() of NetComp is used");
      return(new NetComp());
    }

    // return a reference of the current object in the right class type
    virtual NetComp* ptr() { 
      cout << "ERROR!!, ptr() of NetComp is used!!!" << endl;
      assert(0 == "the ptr() of NetComp is used");
      return this; 
    }
  };
  NETLIST_STREAMOUT(NetComp);
  
}

#endif
