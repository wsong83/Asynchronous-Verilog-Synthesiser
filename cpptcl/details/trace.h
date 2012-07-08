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
  virtual void * get_functor() const = 0;
  virtual void * get_client_data() const = 0;
};

template <typename VT, typename CDT>
class trace : public trace_base {
  typedef VT (*functor_type) (VT const &, CDT *);
public:
  trace(functor_type f, CDT * cd) : f_(f), cd_(cd) {}
  virtual ~trace() {}
  
  virtual void * get_functor() const {
    return f_;
  }

  virtual void * get_client_data() const {
    return cd_;
  }

  virtual void invoke(Tcl_Interp *interp, ClientData cData,
                      const char * VarName, const char *index, int flag) {
    interpreter i(interp, false);
    // fetch the variable
    Tcl_Obj *var = Tcl_GetVar2Ex(interp, VarName, index, flag);
    // run the trace
    VT rv = f_(tcl_cast<VT>::from(interp, var), static_cast<CDT *>(cData));
    // reset the variable
    var = tcl_cast<VT>::to(interp, rv);
    Tcl_Obj *prv = Tcl_SetVar2Ex(interp, VarName, index, var, flag);
    assert(prv != var);
    delete var;                 // delete the tmp var
  }

private:
  functor_type f_;
  CDT * cd_;
};

// Local Variables:
// mode: c++
// End:
