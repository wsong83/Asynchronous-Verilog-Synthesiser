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
 * Using boost preprocessor to reduce the burden of writing templates
 * 06/07/2012   Wei Song
 *
 */

#define n BOOST_PP_ITERATION()

#define dispatch_print(z, n, data) \
tcl_cast<BOOST_PP_CAT(T,n)>::from(interp, objv[BOOST_PP_ADD(n,1)])

template <typename R, BOOST_PP_ENUM_PARAMS(BOOST_PP_DEC(n), typename T)>
class BOOST_PP_CAT(callback,n)<R, BOOST_PP_ENUM_PARAMS(BOOST_PP_DEC(n), T), object const &> : public callback_base
{
     typedef object const & BOOST_PP_CAT(T,BOOST_PP_DEC(n));
     typedef R (*functor_type)(BOOST_PP_ENUM_PARAMS(n,T));
     enum { var_start = n };

public:
     BOOST_PP_CAT(callback,n)(functor_type f) : f_(f) {}
     
     virtual void invoke(Tcl_Interp *interp,
          int objc, Tcl_Obj * CONST objv[],
          policies const &pol, ClientData)
     {
          object mm = get_var_params(interp, objc, objv, var_start, pol);
          
          dispatch<R>::template do_dispatch<BOOST_PP_ENUM_PARAMS(n,T)>(interp, f_,
               BOOST_PP_ENUM(BOOST_PP_DEC(n), dispatch_print, ~), mm);
     }

private:
     functor_type f_;
};

#undef dispatch_print
#undef n
