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

template <typename R, typename T1>
class callback1 : public callback_base
{
     typedef R (*functor_type)(T1);
     
public:
     callback1(functor_type f) : f_(f) {}
     
     virtual void invoke(Tcl_Interp *interp,
          int objc, Tcl_Obj * CONST objv[],
          policies const &, ClientData)
     {
          check_params_no(objc, 2);
          
          dispatch<R>::template do_dispatch<T1>(interp, f_,
               tcl_cast<T1>::from(interp, objv[1]));
     }

private:
     functor_type f_;
};

template <typename R, typename T1, typename T2>
class callback2 : public callback_base
{
     typedef R (*functor_type)(T1, T2);
     
public:
     callback2(functor_type f) : f_(f) {}
     
     virtual void invoke(Tcl_Interp *interp,
          int objc, Tcl_Obj * CONST objv[],
          policies const &, ClientData)
     {
          check_params_no(objc, 3);
          
          dispatch<R>::template do_dispatch<T1, T2>(interp, f_,
               tcl_cast<T1>::from(interp, objv[1]),
               tcl_cast<T2>::from(interp, objv[2]));
     }

private:
     functor_type f_;
};

template <typename R, typename T1, typename T2, typename T3>
class callback3 : public callback_base
{
     typedef R (*functor_type)(T1, T2, T3);
     
public:
     callback3(functor_type f) : f_(f) {}
     
     virtual void invoke(Tcl_Interp *interp,
          int objc, Tcl_Obj * CONST objv[],
          policies const &, ClientData)
     {
          check_params_no(objc, 4);
          
          dispatch<R>::template do_dispatch<T1, T2, T3>(interp, f_,
               tcl_cast<T1>::from(interp, objv[1]),
               tcl_cast<T2>::from(interp, objv[2]),
               tcl_cast<T3>::from(interp, objv[3]));
     }

private:
     functor_type f_;
};

template <typename R, typename T1, typename T2, typename T3, typename T4>
class callback4 : public callback_base
{
     typedef R (*functor_type)(T1, T2, T3, T4);
     
public:
     callback4(functor_type f) : f_(f) {}
     
     virtual void invoke(Tcl_Interp *interp,
          int objc, Tcl_Obj * CONST objv[],
          policies const &, ClientData)
     {
          check_params_no(objc, 5);
          
          dispatch<R>::template do_dispatch<T1, T2, T3, T4>(interp, f_,
               tcl_cast<T1>::from(interp, objv[1]),
               tcl_cast<T2>::from(interp, objv[2]),
               tcl_cast<T3>::from(interp, objv[3]),
               tcl_cast<T4>::from(interp, objv[4]));
     }

private:
     functor_type f_;
};

template <typename R, typename T1, typename T2, typename T3, typename T4,
     typename T5>
class callback5 : public callback_base
{
     typedef R (*functor_type)(T1, T2, T3, T4, T5);
     
public:
     callback5(functor_type f) : f_(f) {}
     
     virtual void invoke(Tcl_Interp *interp,
          int objc, Tcl_Obj * CONST objv[],
          policies const &, ClientData)
     {
          check_params_no(objc, 6);
          
          dispatch<R>::template do_dispatch<T1, T2, T3, T4, T5>(interp, f_,
               tcl_cast<T1>::from(interp, objv[1]),
               tcl_cast<T2>::from(interp, objv[2]),
               tcl_cast<T3>::from(interp, objv[3]),
               tcl_cast<T4>::from(interp, objv[4]),
               tcl_cast<T5>::from(interp, objv[5]));
     }

private:
     functor_type f_;
};

template <typename R, typename T1, typename T2, typename T3, typename T4,
     typename T5, typename T6>
class callback6 : public callback_base
{
     typedef R (*functor_type)(T1, T2, T3, T4, T5, T6);
     
public:
     callback6(functor_type f) : f_(f) {}
     
     virtual void invoke(Tcl_Interp *interp,
          int objc, Tcl_Obj * CONST objv[],
          policies const &, ClientData)
     {
          check_params_no(objc, 7);
          
          dispatch<R>::template do_dispatch<T1, T2, T3, T4, T5, T6>(
               interp, f_,
               tcl_cast<T1>::from(interp, objv[1]),
               tcl_cast<T2>::from(interp, objv[2]),
               tcl_cast<T3>::from(interp, objv[3]),
               tcl_cast<T4>::from(interp, objv[4]),
               tcl_cast<T5>::from(interp, objv[5]),
               tcl_cast<T6>::from(interp, objv[6]));
     }

private:
     functor_type f_;
};

template <typename R, typename T1, typename T2, typename T3, typename T4,
     typename T5, typename T6, typename T7>
class callback7 : public callback_base
{
     typedef R (*functor_type)(T1, T2, T3, T4, T5, T6, T7);
     
public:
     callback7(functor_type f) : f_(f) {}
     
     virtual void invoke(Tcl_Interp *interp,
          int objc, Tcl_Obj * CONST objv[],
          policies const &, ClientData)
     {
          check_params_no(objc, 8);
          
          dispatch<R>::template do_dispatch<T1, T2, T3, T4, T5, T6, T7>(
               interp, f_,
               tcl_cast<T1>::from(interp, objv[1]),
               tcl_cast<T2>::from(interp, objv[2]),
               tcl_cast<T3>::from(interp, objv[3]),
               tcl_cast<T4>::from(interp, objv[4]),
               tcl_cast<T5>::from(interp, objv[5]),
               tcl_cast<T6>::from(interp, objv[6]),
               tcl_cast<T7>::from(interp, objv[7]));
     }

private:
     functor_type f_;
};

template <typename R, typename T1, typename T2, typename T3, typename T4,
     typename T5, typename T6, typename T7, typename T8>
class callback8 : public callback_base
{
     typedef R (*functor_type)(T1, T2, T3, T4, T5, T6, T7, T8);
     
public:
     callback8(functor_type f) : f_(f) {}
     
     virtual void invoke(Tcl_Interp *interp,
          int objc, Tcl_Obj * CONST objv[],
          policies const &, ClientData)
     {
          check_params_no(objc, 9);
          
          dispatch<R>::template do_dispatch<T1, T2, T3, T4, T5, T6, T7, T8>(
               interp, f_,
               tcl_cast<T1>::from(interp, objv[1]),
               tcl_cast<T2>::from(interp, objv[2]),
               tcl_cast<T3>::from(interp, objv[3]),
               tcl_cast<T4>::from(interp, objv[4]),
               tcl_cast<T5>::from(interp, objv[5]),
               tcl_cast<T6>::from(interp, objv[6]),
               tcl_cast<T7>::from(interp, objv[7]),
               tcl_cast<T8>::from(interp, objv[8]));
     }

private:
     functor_type f_;
};

template <typename R, typename T1, typename T2, typename T3, typename T4,
     typename T5, typename T6, typename T7, typename T8, typename T9>
class callback9 : public callback_base
{
     typedef R (*functor_type)(T1, T2, T3, T4, T5, T6, T7, T8, T9);
     
public:
     callback9(functor_type f) : f_(f) {}
     
     virtual void invoke(Tcl_Interp *interp,
          int objc, Tcl_Obj * CONST objv[],
          policies const &, ClientData)
     {
          check_params_no(objc, 10);
          
          dispatch<R>::template do_dispatch<
               T1, T2, T3, T4, T5, T6, T7, T8, T9>(interp, f_,
               tcl_cast<T1>::from(interp, objv[1]),
               tcl_cast<T2>::from(interp, objv[2]),
               tcl_cast<T3>::from(interp, objv[3]),
               tcl_cast<T4>::from(interp, objv[4]),
               tcl_cast<T5>::from(interp, objv[5]),
               tcl_cast<T6>::from(interp, objv[6]),
               tcl_cast<T7>::from(interp, objv[7]),
               tcl_cast<T8>::from(interp, objv[8]),
               tcl_cast<T9>::from(interp, objv[9]));
     }

private:
     functor_type f_;
};
