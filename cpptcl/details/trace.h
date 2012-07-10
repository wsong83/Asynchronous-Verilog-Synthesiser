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
 * definition of trace functions
 * 07/07/2012   Wei Song
 *
 */

// this is not a stand alone header file

class trace_base {
public:
  virtual ~trace_base() {}
  
  virtual void invoke(Tcl_Interp *interp,
                      ClientData, const char *, const char *, int) = 0;
};

template <typename VT, typename CDT>
class trace : public trace_base {
  typedef VT (*functor_type) (VT const &, CDT *);
public:
  trace(functor_type f) : f_(f){}
  virtual ~trace() {}
  
  virtual void invoke(Tcl_Interp *interp, ClientData cData,
                      const char * VarName, const char *index, int flag) {
    // fetch the variable
    Tcl_Obj *var = Tcl_GetVar2Ex(interp, VarName, index, flag);
    VT orig = tcl_cast<VT>::from(interp, var);
    // run the trace
    VT rv = f_(orig, static_cast<CDT *>(cData));
    if(rv != orig) {
      // reset the variable
      var = tcl_cast<VT>::to(interp, rv);
      Tcl_Obj *prv = Tcl_SetVar2Ex(interp, VarName, index, var, flag);
      assert(prv == var);
    }
  }

private:
  functor_type f_;
};

// Local Variables:
// mode: c++
// End:
