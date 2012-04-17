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

#include "averilog/location.hh"
using averilog::location;

// function macro for stream out operator <<
#ifndef NETLIST_STREAMOUT
#define NETLIST_STREAMOUT(COMP)                                \
  inline ostream& operator<< ( ostream& os, const COMP& rhs) { \
    return rhs.streamout(os, (unsigned int)(0));               \
  }
#endif

#ifndef NETLIST_DEFAULT_CON
#define NETLIST_DEFAULT_CON(COMP, CT) COMP() : NetComp(NetComp::CT) { }
#endif

#ifndef NETLIST_DEFAULT_CON_WL
#define NETLIST_DEFAULT_CON_WL(COMP, CT) COMP(const location& lloc) : NetComp(NetComp::CT, lloc) { }
#endif

#ifndef NETLIST_STREAMOUT_DECL
#define NETLIST_STREAMOUT_DECL                    \
  virtual ostream& streamout (ostream&, unsigned int) const
#endif

#ifndef SP_CAST
#define SP_CAST(m, T, d)                      \
  shared_ptr<T > m = static_pointer_cast<T >(d)
#endif

#ifndef NETLIST_CHECK_INPARSE_DECL
#define NETLIST_CHECK_INPARSE_DECL           \
  virtual bool check_inparse( );
#endif

#ifndef NETLIST_SET_FATHER_DECL
#define NETLIST_SET_FATHER_DECL           \
  virtual void set_father(Block* pf);    
#endif

namespace netlist{
  
  // the base class of all netlist components
  class NetComp {
  public:
#include "comp_type.h"
    // no one should directly use this class
    NetComp() : ctype(tUnkown), father(NULL) {}
    NetComp(ctype_t tt) : ctype(tt), father(NULL) {}
    NetComp(ctype_t tt, const averilog::location& lloc) : ctype(tt), loc(lloc), father(NULL) {}
    
    ctype_t get_type() const { return ctype; }
    ctype_t ctype;
    location loc;
    
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

    // syntax check during parsing
    virtual bool check_inparse() {
      cout << "ERROR!!, the check_inparse() of NetComp is used!!!" << endl;
      assert(0 == "the check_inparse() of NetComp is used");
      return false;
    }

    // set the father block pointer
    virtual void set_father(Block* pf) {
      // here a naked pointer is used because tranfer this to shared_ptr is too complicated to be employed
      father = pf;
    }

  protected:
    Block* father;              /* the pointer pointed to the father block */
      
  };
  NETLIST_STREAMOUT(NetComp);
  
}

#endif
