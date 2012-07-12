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

template <typename R>
class callback0 : public callback_base
{
     typedef R (*functor_type)();
     
public:
     callback0(functor_type f) : f_(f) {}
     
     virtual void invoke(Tcl_Interp *interp,
          int, Tcl_Obj * CONST [],
                         policies const &, ClientData)
     {
          dispatch<R>::do_dispatch(interp, f_);
     }

private:
     functor_type f_;
};

#define BOOST_PP_ITERATION_LIMITS (1, 9)
#define BOOST_PP_FILENAME_1       "preproc/callbacks.hpp"
#include BOOST_PP_ITERATE()
#undef BOOST_PP_ITERATION_LIMITS
#undef BOOST_PP_FILENAME_1
