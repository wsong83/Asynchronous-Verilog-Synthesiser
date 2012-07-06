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
tcl_cast<BOOST_PP_CAT(T,n)>::from(interp, objv[BOOST_PP_INC(n)])

template <typename R BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename T)>
class BOOST_PP_CAT(callback,n) : public callback_base
{
     typedef R (*functor_type)(BOOST_PP_ENUM_PARAMS(n,T));
     
public:
     BOOST_PP_CAT(callback,n)(functor_type f) : f_(f) {}
     
     virtual void invoke(Tcl_Interp *interp,
          int objc, Tcl_Obj * CONST objv[],
          policies const &, ClientData)
     {
          check_params_no(objc, BOOST_PP_INC(n));
          
          dispatch<R>::template do_dispatch<BOOST_PP_ENUM_PARAMS(n,T)>(interp, f_,
               BOOST_PP_ENUM(n, dispatch_print, ~));
     }

private:
     functor_type f_;
};

#undef dispatch_print
#undef n
