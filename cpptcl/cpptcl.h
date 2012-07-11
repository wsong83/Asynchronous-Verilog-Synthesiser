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
 * Add support for adding/deleting traces for a variable
 * Using boost preprocessor to reduce the burden of writing templates
 * Add extra argument to support client data
 * 04/07/2012 ~ 07/07/2012   Wei Song
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

#ifndef CPPTCL_INCLUDED
#define CPPTCL_INCLUDED

extern "C" {
  #include <tcl.h>
}
#include <string>
#include <stdexcept>
#include <sstream>
#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/preprocessor/repetition.hpp>
#include <boost/preprocessor/arithmetic/sub.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/iteration/iterate.hpp>
#include <boost/lexical_cast.hpp>

namespace Tcl
{

// exception class used for reporting all Tcl errors

class tcl_error : public std::runtime_error
{
public:
     explicit tcl_error(std::string const &msg)
          : std::runtime_error(msg) {}
     explicit tcl_error(Tcl_Interp *interp)
          : std::runtime_error(Tcl_GetString(Tcl_GetObjResult(interp))) {}
};

// call policies

struct policies
{
     policies() : variadic_(false) {}

     policies & factory(std::string const &name);

     // note: this is additive
     policies & sink(int index);

     policies & variadic();

     std::string factory_;
     std::vector<int> sinks_;
     bool variadic_;
};

// syntax short-cuts
policies factory(std::string const &name);
policies sink(int index);
policies variadic();


class interpreter;
class object;

namespace details
{

// wrapper for the evaluation result
class result
{
public:
     result(Tcl_Interp *interp);
     
     operator bool() const;
     operator double() const;
     operator int() const;
     operator long() const;
     operator long long() const;
     operator std::string() const;
     operator object() const;

private:
     Tcl_Interp *interp_;
};

// helper functions used to set the result value

void set_result(Tcl_Interp *interp, bool b);
void set_result(Tcl_Interp *interp, int i);
void set_result(Tcl_Interp *interp, long i);
void set_result(Tcl_Interp *interp, long long i);
void set_result(Tcl_Interp *interp, double d);
void set_result(Tcl_Interp *interp, std::string const &s);
void set_result(Tcl_Interp *interp, void *p);
void set_result(Tcl_Interp *interp, object const &o);

// helper functor for converting Tcl objects to the given type
#include "details/conversions.h"

// dispatchers able to capture (or ignore) the result
#include "details/dispatchers.h"

// helper for checking for required number of parameters
// (throws tcl_error when not met)
void check_params_no(int objc, int required);

// helper for gathering optional params in variadic functions
object get_var_params(Tcl_Interp *interp,
     int objc, Tcl_Obj * CONST objv[],
     int from, policies const &pol);

// the callback_base is used to store callback handlers in a polynorphic map
class callback_base
{
public:
     virtual ~callback_base() {}

     virtual void invoke(Tcl_Interp *interp,
                         int objc, Tcl_Obj * CONST objv[],
                         policies const &pol, ClientData) = 0;
};


// base class for object command handlers
// and for class handlers
class object_cmd_base
{
public:
     // destructor not needed, but exists to shut up the compiler warnings
     virtual ~object_cmd_base() {}

     virtual void invoke(void *p, Tcl_Interp *interp,
          int objc, Tcl_Obj * CONST objv[],
          policies const &pol) = 0;
};

// base class for all class handlers, still abstract
class class_handler_base : public object_cmd_base
{
public:
     typedef std::map<std::string, policies> policies_map_type;

     class_handler_base();

     void register_method(std::string const &name,
          boost::shared_ptr<object_cmd_base> ocb, policies const &p);

     policies & get_policies(std::string const &name);

protected:
     typedef std::map<
          std::string,
          boost::shared_ptr<object_cmd_base>
          > method_map_type;

     // a map of methods for the given class
     method_map_type methods_;

     policies_map_type policies_;
};

// class handler - responsible for executing class methods
template <class C>
class class_handler : public class_handler_base
{
public:
     virtual void invoke(void *pv, Tcl_Interp *interp,
          int objc, Tcl_Obj * CONST objv[], policies const &pol)
     {
          C * p = static_cast<C*>(pv);

          if (objc < 2)
          {
               throw tcl_error("Too few arguments.");
          }
          
          std::string methodName(Tcl_GetString(objv[1]));

          if (methodName == "-delete")
          {
               Tcl_DeleteCommand(interp, Tcl_GetString(objv[0]));
               delete p;
               return;
          }

          // dispatch on the method name

          method_map_type::iterator it = methods_.find(methodName);
          if (it == methods_.end())
          {
               throw tcl_error("Method " + methodName + " not found.");
          }

          it->second->invoke(pv, interp, objc, objv, pol);
     }
};


// factory functions for creating class objects
#include "details/constructors.h"

// actual callback envelopes
#include "details/callbacks.h"
#include "details/callbacks_cd.h"

// trace functions
#include "details/trace.h"

// actual method envelopes
#include "details/methods.h"

// helper meta function for figuring appropriate constructor callback
#include "details/metahelpers.h"


// this class is used to provide the "def" interface for defining
// class member functions

template <class C>
class class_definer
{
public:
     class_definer(boost::shared_ptr<class_handler<C> > ch) : ch_(ch) {}

#define BOOST_PP_ITERATION_LIMITS (0, 9)
#define BOOST_PP_FILENAME_1       "preproc/class_definer.hpp"
#include BOOST_PP_ITERATE()
#undef BOOST_PP_ITERATION_LIMITS
#undef BOOST_PP_FILENAME_1

private:
     boost::shared_ptr<class_handler<C> > ch_;
};

} // namespace details


// init type for defining class constructors
template <typename T1 = void, typename T2 = void, typename T3 = void,
          typename T4 = void, typename T5 = void, typename T6 = void,
          typename T7 = void, typename T8 = void, typename T9 = void>
class init {};

// no_init type and object - to define classes without constructors
namespace details
{
     struct no_init_type {};
} // namespace details
extern details::no_init_type no_init;


// interpreter wrapper
class interpreter
{
public:
     interpreter();
     interpreter(Tcl_Interp *, bool owner = true);
     virtual ~interpreter();
     
     void make_safe();
     
     Tcl_Interp * get() const { return interp_; }

     // free function definitions
     
#define BOOST_PP_ITERATION_LIMITS (0, 9)
#define BOOST_PP_FILENAME_1       "preproc/interp_def.hpp"
#include BOOST_PP_ITERATE()
#undef BOOST_PP_ITERATION_LIMITS
#undef BOOST_PP_FILENAME_1
    
     // class definitions

     template <class C>
     details::class_definer<C> class_(std::string const &name)
     {
          boost::shared_ptr<details::class_handler<C> > ch(
               new details::class_handler<C>());

          add_class(name, ch);

          add_constructor(name, ch,
               boost::shared_ptr<details::callback_base>(
                    new details::callback0<C*>(&details::construct<
                         C, void, void, void, void, void, void, void,
                         void, void>::doit)));

          return details::class_definer<C>(ch);
     }

     template <class C, typename T1, typename T2, typename T3, typename T4,
               typename T5, typename T6, typename T7, typename T8,
               typename T9>
     details::class_definer<C> class_(std::string const &name,
          init<T1, T2, T3, T4, T5, T6, T7, T8, T9> const &,
          policies const &p = policies())
     {
          typedef typename details::get_callback_type_for_construct<
               C, T1, T2, T3, T4, T5, T6, T7, T8, T9>::type
               callback_type;

          boost::shared_ptr<details::class_handler<C> > ch(
               new details::class_handler<C>());

          add_class(name, ch);

          add_constructor(name, ch,
               boost::shared_ptr<details::callback_base>(
                    new callback_type(&details::construct<
                         C, T1, T2, T3, T4, T5, T6, T7, T8, T9>::doit)), p);

          return details::class_definer<C>(ch);
     }

     template <class C>
     details::class_definer<C> class_(
          std::string const &name, details::no_init_type const &)
     {
          boost::shared_ptr<details::class_handler<C> > ch(
               new details::class_handler<C>());

          add_class(name, ch);

          return details::class_definer<C>(ch);
     }

     // free script evaluation
     details::result eval(std::string const &script);
     details::result eval(std::istream &s);

     details::result eval(object const &o);

     // the InputIterator should give object& or Tcl_Obj* when dereferenced
     template <class InputIterator>
     details::result eval(InputIterator first, InputIterator last);

     // create alias from the *this interpreter to the target interpreter
     void create_alias(std::string const &cmd,
          interpreter &targetInterp, std::string const &targetCmd);

     // register a package info (useful when defining packages)
     void pkg_provide(std::string const &name, std::string const &version);

     // helper for cleaning up callbacks in non-managed interpreters
  // the original static is removed as I need call member function
  // see no reason why this must be a static
     void clear_definitions(Tcl_Interp *);

  // create a trace to a variable
  // a read trace using variable name
  template<typename VT, typename CDT = void>
  void def_read_trace(const std::string& VarName, // variable name
                      const std::string& FunName, // function name
                      VT (*proc)(VT const &, CDT *), // callback function
                      CDT *cData = NULL) {  // the data passed to the callback function
    add_trace(VarName, NULL, FunName,
              boost::shared_ptr<details::trace_base>
              (new details::trace<VT, CDT>(proc)), 
              cData, TCL_TRACE_READS);
  }

  // a read trace using array name and its index
  template<typename VT, typename CDT = void>
  void def_read_trace(const std::string& VarName, // variable name
                      unsigned int index,         // the index of a array element
                      const std::string& FunName, // function name
                      VT (*proc)(VT const &, CDT *), // callback
                      CDT *cData = NULL) {  // the data passed to the callback function
    add_trace(VarName, &index, FunName,
              boost::shared_ptr<details::trace_base>
              (new details::trace<VT, CDT>(proc)), 
              cData, TCL_TRACE_READS);
  }
    
  // a read trace using variable name
  template<typename VT, typename CDT = void>
  void def_write_trace(const std::string& VarName, // variable name
                       const std::string& FunName, // function name
                       VT (*proc)(VT const &, CDT *), // callback function
                       CDT *cData = NULL) {  // the data passed to the callback function
    add_trace(VarName, NULL, FunName, 
              boost::shared_ptr<details::trace_base>
              (new details::trace<VT, CDT>(proc)), 
              cData, TCL_TRACE_WRITES);
  }

  // a read trace using array name and its index
  template<typename VT, typename CDT = void>
  void def_write_trace(const std::string& VarName, // variable name
                       unsigned int index,         // the index of a array element
                       const std::string& FunName, // function name
                       VT (*proc)(VT const &, CDT *), // callback
                       CDT *cData = NULL) {  // the data passed to the callback function
    add_trace(VarName, &index, FunName,
              boost::shared_ptr<details::trace_base>
              (new details::trace<VT, CDT>(proc)), 
              cData, TCL_TRACE_WRITES);
  }

  // delete a read trace using variable name
  void undef_read_trace(const std::string& VarName, // variable name
                        const std::string& FunName = "") { 
    remove_trace(VarName, NULL, FunName, TCL_TRACE_READS);
  }

  // delete a read trace using array name and its index
  void undef_read_trace(const std::string& VarName, // variable name
                        unsigned int index,         // the index of a array element
                        const std::string& FunName = "") { 
    remove_trace(VarName, &index, FunName, TCL_TRACE_READS);
  }

  // delete a write trace using variable name
  void undef_write_trace(const std::string& VarName, // variable name
                         const std::string& FunName = "") { 
    remove_trace(VarName, NULL, FunName, TCL_TRACE_WRITES);
  }

  // delete a write trace using array name and its index
  void undef_write_trace(const std::string& VarName, // variable name
                         unsigned int index,         // the index of a array element
                         const std::string& FunName = "") { 
    remove_trace(VarName, &index, FunName, TCL_TRACE_WRITES);
  }

  // delete all traces using variable name
  void undef_all_trace(const std::string& VarName) { // variable name
    remove_trace(VarName, NULL, "", TCL_TRACE_WRITES|TCL_TRACE_READS);
  }

  // delete all traces using array name and its index
  void undef_all_trace(const std::string& VarName, // variable name
                       unsigned int index) { // the index of a array element
    remove_trace(VarName, &index, "", TCL_TRACE_WRITES|TCL_TRACE_READS);
  }

  // add Tcl variables
  template<typename VT>
  void set_variable(const std::string& VarName, VT const & var) {
    Tcl_SetVar2Ex(interp_, VarName.c_str(), 
                  NULL, 
                  details::tcl_cast<VT>::to(interp_, var), 0);
  }

  template<typename VT>
  void set_variable(const std::string& VarName, unsigned int index, VT const & var) {
    Tcl_SetVar2Ex(interp_, VarName.c_str(), 
                  boost::lexical_cast<std::string>(index).c_str(), 
                  details::tcl_cast<VT>::to(interp_, var), 0);
  }

  // remove Tcl variables
  void remove_variable(const std::string& VarName) {
    Tcl_UnsetVar(interp_, VarName.c_str(), 0);
  }
  
  void remove_variable(const std::string& VarName, unsigned int index) {
    Tcl_UnsetVar2(interp_, VarName.c_str(), 
                  boost::lexical_cast<std::string>(index).c_str(), 0);
  }

  // read Tcl variables
  template<typename VT>
  VT read_variable(const std::string& VarName) {
    return 
      details::tcl_cast<VT>::
      from(interp_, Tcl_GetVar2Ex(interp_, VarName.c_str(), 
                                  NULL, 0));
  }
  
  template<typename VT>
  VT read_variable(const std::string& VarName, unsigned int index) {
    return 
      details::tcl_cast<VT>::
      from(interp_, Tcl_GetVar2Ex(interp_, VarName.c_str(),
                                  boost::lexical_cast<std::string>(index).c_str(), 0));
  }
  

private:

     // copy not supported
     interpreter(const interpreter &);
     void operator=(const interpreter &);

     void add_function(std::string const &name,
          boost::shared_ptr<details::callback_base> cb,
          policies const &p = policies(), ClientData cData = NULL);

  void add_trace(const std::string& VarName, unsigned int *index,  
                 const std::string& FunName,
                 boost::shared_ptr<details::trace_base> proc,
                 void * cData, int flag);

  void remove_trace(const std::string& VarName, unsigned int *index, 
                    const std::string& FunName, int flag);

     void add_class(std::string const &name,
          boost::shared_ptr<details::class_handler_base> chb);

     void add_constructor(std::string const &name,
          boost::shared_ptr<details::class_handler_base> chb,
          boost::shared_ptr<details::callback_base> cb,
          policies const &p = policies());

     Tcl_Interp *interp_;
     bool owner_;
};


// object wrapper
class object
{
public:

     // constructors

     object();

     explicit object(bool b);
     object(char const *buf, size_t size); // byte array
     explicit object(double b);
     explicit object(int i);

     // list creation
     // the InputIterator should give object& or Tcl_Obj* when dereferenced
     template <class InputIterator>
     object(InputIterator first, InputIterator last)
          : interp_(0)
     {
          std::vector<Tcl_Obj*> v;
          fill_vector(v, first, last);
          obj_ = Tcl_NewListObj(static_cast<int>(v.size()),
               v.empty() ? NULL : &v[0]);
          Tcl_IncrRefCount(obj_);
     }

     explicit object(long i);
     explicit object(long long i);
     explicit object(char const *s);        // string construction
     explicit object(std::string const &s); // string construction
     explicit object(std::vector<std::string> const &s);
     explicit object(Tcl_Obj *o, bool shared = false);

     object(object const &other, bool shared = false);
     ~object();

     // assignment

     object & assign(bool b);
     object & resize(size_t size);                  // byte array resize
     object & assign(char const *buf, size_t size); // byte array assignment
     object & assign(double d);
     object & assign(int i);

     // list assignment
     // the InputIterator should give Tcl_Obj* or object& when dereferenced
     template <class InputIterator>
     object & assign(InputIterator first, InputIterator last)
     {
          std::vector<Tcl_Obj*> v;
          fill_vector(v, first, last);
          Tcl_SetListObj(obj_, static_cast<int>(v.size()),
               v.empty() ? NULL : &v[0]);
          return *this;
     }

     object & assign(long i);
     object & assign(long long i);
     object & assign(char const *s);        // string assignment
     object & assign(std::string const &s); // string assignment
     object & assign(std::vector<std::string> const &s);
     object & assign(object const &o);
     object & assign(Tcl_Obj *o);

     object & operator=(bool b)               { return assign(b); }
     object & operator=(double d)             { return assign(d); }
     object & operator=(int i)                { return assign(i); }
     object & operator=(long i)               { return assign(i); }
     object & operator=(long long i)          { return assign(i); }
     object & operator=(char const *s)        { return assign(s); }
     object & operator=(std::string const &s) { return assign(s); }
     object & operator=(std::vector<std::string> const &s) { return assign(s); }

     object & operator=(object const &o)      { return assign(o); }
     object & swap(object &other);

     // (logically) non-modifying members

     template <typename T>
     T get(interpreter &i) const;

  // specialization for string without interpreter
  std::string get_string() const;

     char const * get() const;             // string get
     char const * get(size_t &size) const; // byte array get

     size_t length(interpreter &i) const;  // returns list length
     object at(interpreter &i, size_t index) const;

     Tcl_Obj * get_object() const { return obj_; }

     // modifying members

     object & append(interpreter &i, object const &o);
     object & append_list(interpreter &i, object const &o);

     // list replace
     // the InputIterator should give Tcl_Obj* or object& when dereferenced
     template <class InputIterator>
     object & replace(interpreter &i, size_t index, size_t count,
          InputIterator first, InputIterator last)
     {
          std::vector<Tcl_Obj*> v;
          fill_vector(v, first, last);
          int res = Tcl_ListObjReplace(i.get(), obj_,
               static_cast<int>(index), static_cast<int>(count),
               static_cast<int>(v.size()), v.empty() ? NULL : &v[0]);
          if (res != TCL_OK)
          {
               throw tcl_error(i.get());
          }

          return *this;
     }

     object & replace(interpreter &i, size_t index, size_t count,
          object const &o);
     object & replace_list(interpreter &i, size_t index, size_t count,
          object const &o);

     // helper functions for piggy-backing interpreter info
     void set_interp(Tcl_Interp *interp);
     Tcl_Interp * get_interp() const;

     // helper function, also used from interpreter::eval
     template <class InputIterator>
     static void fill_vector(std::vector<Tcl_Obj*> &v,
          InputIterator first, InputIterator last)
     {
          for (; first != last; ++first)
          {
               object o(*first, true);
               v.push_back(o.obj_);
          }
     }

private:

     // helper function used from copy constructors
     void init(Tcl_Obj *o, bool shared);

     Tcl_Obj *obj_;
     Tcl_Interp *interp_;
};

// available specializations for object::get
template <> bool         object::get<bool>(interpreter &i) const;
template <> double       object::get<double>(interpreter &i) const;
template <> int          object::get<int>(interpreter &i) const;
template <> long         object::get<long>(interpreter &i) const;
template <> long long    object::get<long long>(interpreter &i) const;
template <> char const * object::get<char const *>(interpreter &i) const;
template <> std::string  object::get<std::string>(interpreter &i) const;
template <>
std::vector<char>        object::get<std::vector<char> >(interpreter &i) const;
template <>
std::vector<std::string> object::get<std::vector<std::string> >(interpreter &i) const;

// the InputIterator should give object& or Tcl_Obj* when dereferenced
template <class InputIterator>
details::result interpreter::eval(InputIterator first, InputIterator last)
{
     std::vector<Tcl_Obj*> v;
     object::fill_vector(v, first, last);
     int cc = Tcl_EvalObjv(interp_,
          static_cast<int>(v.size()), v.empty() ? NULL : &v[0], 0);
     if (cc != TCL_OK)
     {
          throw tcl_error(interp_);
     }

     return details::result(interp_);
}

namespace details
{

// additional callback envelopes for variadic functions
#include "details/callbacks_v.h"

// additional method envelopes for variadic methods
#include "details/methods_v.h"

} // namespace details

} // namespace Tcl


// macro for defining loadable module entry point
// - used for extending Tcl interpreter

#define CPPTCL_MODULE(name, i) void name##_cpptcl_Init(Tcl::interpreter &i); \
     extern "C" int name##_Init(Tcl_Interp *interp) \
     { \
          Tcl::interpreter i(interp, false); \
          name##_cpptcl_Init(i); \
          return TCL_OK; \
     } \
     void name##_cpptcl_Init(Tcl::interpreter &i)


#endif // CPPTCL_INCLUDED

// Local Variables:
// mode: c++
// End:
