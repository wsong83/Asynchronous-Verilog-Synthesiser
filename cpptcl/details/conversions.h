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
 * Add conversions from common types to Tcl objects
 * 07/07/2012   Wei Song
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


// helper functor for converting Tcl objects to the given type
// (it is a struct instead of function,
// because I need to partially specialize it)

template <typename T>
struct tcl_cast;

template <typename T>
struct tcl_cast<T*>
{
     static T * from(Tcl_Interp *, Tcl_Obj *obj)
     {
          std::string s(Tcl_GetString(obj));
          if (s.size() == 0)
          {
               throw tcl_error("Expected pointer value, got empty string.");
          }

          if (s[0] != 'p')
          {
               throw tcl_error("Expected pointer value.");
          }

          std::istringstream ss(s);
          char dummy;
          void *p;
          ss >> dummy >> p;

          return static_cast<T*>(p);
     }
     
     static Tcl_Obj * to(Tcl_Interp *, T* v) {
       std::ostringstream ss;
       ss << 'p' << static_cast<void *>(v);
       std::string s(ss.str());
       return Tcl_NewStringObj(s.data(), static_cast<int>(s.size()));
     }

};

// the following partial specialization is to strip reference
// (it returns a temporary object of the underlying type, which
// can be bound to the const-ref parameter of the actual function)

template <typename T>
struct tcl_cast<T const &>
{
     static T from(Tcl_Interp *interp, Tcl_Obj *obj)
     {
          return tcl_cast<T>::from(interp, obj);
     }

     static Tcl_Obj * to(Tcl_Interp *interp, T const & v) {
       return tcl_cast<T>::to(interp, v);
     }
};


// the following specializations are implemented

template <>
struct tcl_cast<int>
{
     static int from(Tcl_Interp *, Tcl_Obj *);
     static Tcl_Obj * to(Tcl_Interp *, int const &);
};

template <>
struct tcl_cast<long>
{
     static long from(Tcl_Interp *, Tcl_Obj *);
     static Tcl_Obj * to(Tcl_Interp *, long const &);
};

template <>
struct tcl_cast<long long>
{
     static long long from(Tcl_Interp *, Tcl_Obj *);
     static Tcl_Obj * to(Tcl_Interp *, long long const &);
};

template <>
struct tcl_cast<bool>
{
     static bool from(Tcl_Interp *, Tcl_Obj *);
     static Tcl_Obj * to(Tcl_Interp *, bool const &);
};

template <>
struct tcl_cast<double>
{
     static double from(Tcl_Interp *, Tcl_Obj *);
     static Tcl_Obj * to(Tcl_Interp *, double const &);
};

template <>
struct tcl_cast<std::string>
{
     static std::string from(Tcl_Interp *, Tcl_Obj *);
     static Tcl_Obj * to(Tcl_Interp *, std::string const &);
};

template <>
struct tcl_cast<char const *>
{
     static char const * from(Tcl_Interp *, Tcl_Obj *);
     static Tcl_Obj * to(Tcl_Interp *, char const * const &);
};

template <>
struct tcl_cast<std::vector<std::string> >
{
     static std::vector<std::string> from(Tcl_Interp *, Tcl_Obj *);
     static Tcl_Obj * to(Tcl_Interp *, std::vector<std::string> const &);
};

template <>
struct tcl_cast<object>
{
     static object from(Tcl_Interp *, Tcl_Obj *);
     static Tcl_Obj * to(Tcl_Interp *, object const &);
};
