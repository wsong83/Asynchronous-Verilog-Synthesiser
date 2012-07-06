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
 * Add extra argument to support client data
 * 04/07/2012   Wei Song
 *
 *
 */
//
// Copyright (C) 2004-2006, Maciej Sobczak
//
// Permission to copy, use, modify, sell and distribute this software
// is granted provided this copyright notice appears in all copies.
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//

// Note: this file is not supposed to be a stand-alone header

template <typename R, typename T1>
class callback1_cd : public callback_base
{
     typedef R (*functor_type)(T1 *);
     
public:
     callback1_cd(functor_type f) : f_(f) {}
     
     virtual void invoke(Tcl_Interp *interp,
                         int objc, Tcl_Obj * CONST objv[],
                         policies const &, ClientData cd)
     {
          check_params_no(objc, 1);
          
          dispatch<R>::
            template do_dispatch<T1 *>(interp, f_,
                                       static_cast<T1 *>(cd));
     }

private:
     functor_type f_;
};

template <typename R, typename T1, typename T2>
class callback2_cd : public callback_base
{
     typedef R (*functor_type)(T1, T2 *);
     
public:
     callback2_cd(functor_type f) : f_(f) {}
     
     virtual void invoke(Tcl_Interp *interp,
                         int objc, Tcl_Obj * CONST objv[],
                         policies const &, ClientData cd)
     {
          check_params_no(objc, 2);
          
          dispatch<R>::
            template do_dispatch<T1, T2 *>(interp, f_,
                                           tcl_cast<T1>::from(interp, objv[1]),
                                           static_cast<T2 *>(cd));
     }

private:
     functor_type f_;
};

template <typename R, typename T2>
  class callback2_cd<R, object const &, T2> : public callback_base
{
     typedef object const & T1;
     typedef R (*functor_type)(T1, T2 *);
     enum { var_start = 1 };
     
public:
     callback2_cd(functor_type f) : f_(f) {}
     
     virtual void invoke(Tcl_Interp *interp,
                         int objc, Tcl_Obj * CONST objv[],
                         policies const &pol, ClientData cd)
     {
          object t1 = get_var_params(interp, objc, objv, var_start, pol);
          dispatch<R>::
            template do_dispatch<T1, T2 *>(interp, f_,
                                           t1,
                                           static_cast<T2 *>(cd));
     }

private:
     functor_type f_;
};

#define BOOST_PP_ITERATION_LIMITS (3, 10)
#define BOOST_PP_FILENAME_1       "preproc/callbacks_cd.hpp"
#include BOOST_PP_ITERATE()
#undef BOOST_PP_ITERATION_LIMITS
#undef BOOST_PP_FILENAME_1

