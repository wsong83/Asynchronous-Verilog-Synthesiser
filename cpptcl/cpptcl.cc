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
 * Small modification to resolve name conflicts between boost and std
 * 02/07/2012 ~ 07/07/2012   Wei Song
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

#include "cpptcl.h"
#include <map>
#include <sstream>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

using namespace Tcl;
using namespace Tcl::details;
using std::string;
using boost::shared_ptr;
using boost::tuple;
using std::map;
using std::pair;
using std::vector;
using std::ostringstream;
using std::istringstream;
using std::istreambuf_iterator;
using std::istream;
using std::exception;

result::result(Tcl_Interp *interp) : interp_(interp) {}

result::operator bool() const
{
     Tcl_Obj *obj = Tcl_GetObjResult(interp_);
     
     int val, cc;
     cc = Tcl_GetBooleanFromObj(interp_, obj, &val);
     if (cc != TCL_OK)
     {
          throw tcl_error(interp_);
     }
     
     return val != 0;
}

result::operator double() const
{
     Tcl_Obj *obj = Tcl_GetObjResult(interp_);
     
     double val;
     int cc = Tcl_GetDoubleFromObj(interp_, obj, &val);
     if (cc != TCL_OK)
     {
          throw tcl_error(interp_);
     }
     
     return val;
}

result::operator int() const
{
     Tcl_Obj *obj = Tcl_GetObjResult(interp_);
     
     int val, cc;
     cc = Tcl_GetIntFromObj(interp_, obj, &val);
     if (cc != TCL_OK)
     {
          throw tcl_error(interp_);
     }
     
     return val;
}

result::operator long() const
{
     Tcl_Obj *obj = Tcl_GetObjResult(interp_);
     
     long val;
     int cc;
     cc = Tcl_GetLongFromObj(interp_, obj, &val);
     if (cc != TCL_OK)
     {
          throw tcl_error(interp_);
     }
     
     return val;
}

result::operator long long() const
{
     Tcl_Obj *obj = Tcl_GetObjResult(interp_);
     
     long long val;
     int cc;
     cc = Tcl_GetWideIntFromObj(interp_, obj, &val);
     if (cc != TCL_OK)
     {
          throw tcl_error(interp_);
     }
     
     return val;
}

result::operator string() const
{
     Tcl_Obj *obj = Tcl_GetObjResult(interp_);
     return Tcl_GetString(obj);
}

result::operator object() const
{
     return object(Tcl_GetObjResult(interp_));
}


void details::set_result(Tcl_Interp *interp, bool b)
{
     Tcl_SetObjResult(interp, Tcl_NewBooleanObj(b));
}

void details::set_result(Tcl_Interp *interp, int i)
{
     Tcl_SetObjResult(interp, Tcl_NewIntObj(i));
}

void details::set_result(Tcl_Interp *interp, long i)
{
     Tcl_SetObjResult(interp, Tcl_NewLongObj(i));
}

void details::set_result(Tcl_Interp *interp, long long i)
{
     Tcl_SetObjResult(interp, Tcl_NewWideIntObj(i));
}

void details::set_result(Tcl_Interp *interp, double d)
{
     Tcl_SetObjResult(interp, Tcl_NewDoubleObj(d));
}

void details::set_result(Tcl_Interp *interp, string const &s)
{
     Tcl_SetObjResult(interp,
          Tcl_NewStringObj(s.data(), static_cast<int>(s.size())));
}

void details::set_result(Tcl_Interp *interp, void *p)
{
     ostringstream ss;
     ss << 'p' << p;
     string s(ss.str());

     Tcl_SetObjResult(interp,
          Tcl_NewStringObj(s.data(), static_cast<int>(s.size())));
}

void details::set_result(Tcl_Interp *interp, object const &o)
{
     Tcl_SetObjResult(interp, o.get_object());
}


void details::check_params_no(int objc, int required)
{
     if (objc < required)
     {
          throw tcl_error("Too few arguments.");
     }
}

object details::get_var_params(Tcl_Interp *interp,
     int objc, Tcl_Obj * CONST objv[],
     int from, policies const &pol)
{
     object o;
     if (pol.variadic_)
     {
          check_params_no(objc, from);
          o.assign(objv + from, objv + objc);
     }
     else
     {
          check_params_no(objc, from + 1);
          o.assign(objv[from]);
     }

     o.set_interp(interp);

     return o;
}


namespace // unnamed
{

// map of polymorphic callbacks
typedef map<string, shared_ptr<callback_base> > callback_interp_map;
typedef map<void *, callback_interp_map> callback_map;

  //callback_map callbacks;
  //callback_map constructors;

  // map of polymorphic variable traces
  typedef pair<const string, tuple<shared_ptr<trace_base>, int, void *> > trace_record;
  typedef map<const string, tuple<shared_ptr<trace_base>, int, void *> > trace_record_map;
  typedef map<pair<string, string>, trace_record_map> trace_interp_map;
  typedef map<void *, trace_interp_map> trace_map;

  //trace_map traces;

// map of call policies
typedef map<string, policies> policies_interp_map;
typedef map<void *, policies_interp_map> policies_map;

  //policies_map call_policies;

// map of object handlers
typedef map<string, shared_ptr<class_handler_base> > class_interp_map;
typedef map<void *, class_interp_map> class_handlers_map;

  //class_handlers_map class_handlers;
}

namespace Tcl {
  // the global data structure to save all tcl names
  class tcl_name_database {
  public:
    callback_map callbacks;     // free function map
    callback_map constructors;  // object map
    trace_map traces;           // variable trace map
    policies_map call_policies; // object control
    class_handlers_map class_handlers; // class handler
  };
}

namespace { // unnamed

  shared_ptr<tcl_name_database> gdb_(new tcl_name_database());

// helper for finding call policies - returns true when found
bool find_policies(Tcl_Interp *interp, string const &cmdName,
     policies_interp_map::iterator &piti)
{
     policies_map::iterator pit = gdb_->call_policies.find(interp);
     if (pit == gdb_->call_policies.end())
     {
          return false;
     }
     
     piti = pit->second.find(cmdName);
     return piti != pit->second.end();
}

extern "C"
int object_handler(ClientData cd, Tcl_Interp *interp,
     int objc, Tcl_Obj * CONST objv[]);

// helper function for post-processing call policies
// for both free functions (isMethod == false)
// and class methods (isMethod == true)
void post_process_policies(Tcl_Interp *interp, policies &pol,
     Tcl_Obj * CONST objv[], bool isMethod)
{
     // check if it is a factory
     if (pol.factory_.empty() == false)
     {
          class_handlers_map::iterator it = gdb_->class_handlers.find(interp);
          if (it == gdb_->class_handlers.end())
          {
               throw tcl_error(
                    "Factory was registered for unknown class.");
          }

          class_interp_map::iterator oit = it->second.find(pol.factory_);
          if (oit == it->second.end())
          {
               throw tcl_error(
                    "Factory was registered for unknown class.");
          }

          class_handler_base *chb = oit->second.get();

          // register a new command for the object returned
          // by this factory function
          // if everything went OK, the result is the address of the
          // new object in the 'pXXX' form
          // - the new command will be created with this name

          Tcl_CreateObjCommand(interp,
               Tcl_GetString(Tcl_GetObjResult(interp)),
               object_handler, static_cast<ClientData>(chb), 0);
     }

     // process all declared sinks
     // - unregister all object commands that envelopes the pointers
     for (vector<int>::iterator s = pol.sinks_.begin();
          s != pol.sinks_.end(); ++s)
     {
          if (isMethod == false)
          {
               // example: if there is a declared sink at parameter 3,
               // and the Tcl command was:
               // % fun par1 par2 PAR3 par4
               // then the index 3 correctly points into the objv array

               int index = *s;
               Tcl_DeleteCommand(interp, Tcl_GetString(objv[index]));
          }
          else
          {
               // example: if there is a declared sink at parameter 3,
               // and the Tcl command was:
               // % $p method par1 par2 PAR3 par4
               // then the index 3 needs to be incremented
               // in order correctly point into the 4th index of objv array

               int index = *s + 1;
               Tcl_DeleteCommand(interp, Tcl_GetString(objv[index]));
          }
     }
}

// actual functions handling various callbacks

// generic callback handler
extern "C"
int callback_handler(ClientData cData, Tcl_Interp *interp,
     int objc, Tcl_Obj * CONST objv[])
{
     callback_map::iterator it = gdb_->callbacks.find(interp);
     if (it == gdb_->callbacks.end())
     {
          char msg[] = "Trying to invoke non-existent callback (wrong interpreter?)";
          Tcl_SetResult(interp,
               msg,
               TCL_STATIC);
          return TCL_ERROR;
     }
     
     string cmdName(Tcl_GetString(objv[0]));
     callback_interp_map::iterator iti = it->second.find(cmdName);
     if (iti == it->second.end())
     {
          char msg[] = "Trying to invoke non-existent callback (wrong cmd name?)";
          Tcl_SetResult(interp,
               msg,
               TCL_STATIC);
          return TCL_ERROR;
     }
     
     policies_map::iterator pit = gdb_->call_policies.find(interp);
     if (pit == gdb_->call_policies.end())
     {
          char msg[] = "Trying to invoke callback with no known policies";
          Tcl_SetResult(interp,
               msg,
               TCL_STATIC);
          return TCL_ERROR;
     }
     
     policies_interp_map::iterator piti;
     if (find_policies(interp, cmdName, piti) == false)
     {
          char msg[] = "Trying to invoke callback with no known policies";
          Tcl_SetResult(interp,
               msg,
               TCL_STATIC);
          return TCL_ERROR;
     }

     policies &pol = piti->second;
     
     try
     {
       iti->second->invoke(interp, objc, objv, pol, cData);

          post_process_policies(interp, pol, objv, false);
     }
     catch (std::exception const &e)
     {
          Tcl_SetResult(interp, const_cast<char*>(e.what()), TCL_VOLATILE);
          return TCL_ERROR;
     }
     catch (...)
     {
          char msg[] = "Unknown error.";
          Tcl_SetResult(interp, msg, TCL_STATIC);
          return TCL_ERROR;
     }
     
     return TCL_OK;
}

// generic trace handler
static char err_msg_trace_no_interp[] = "Trying to invoke non-existent trace (wrong interpreter?)";
static char err_msg_trace_no_var[] = "Trying to invoke non-existent trace (wrong variable name?)";
static char err_msg_trace_unknown[] = "Unknown error.";

extern "C"
char * trace_handler(ClientData cData, Tcl_Interp *interp,
                     const char * VarName, const char* index, int flags) {
  if(!gdb_->traces.count(interp)) {   // interp not found
    return err_msg_trace_no_interp;
  }
     
  pair<string, string> map_name(VarName, "");
  if(index != NULL) map_name.second = boost::lexical_cast<string>(*index);

  if(!gdb_->traces[interp].count(map_name)) {
    return err_msg_trace_no_var;
  }
  
  BOOST_FOREACH(trace_record& m, gdb_->traces[interp][map_name]) {
    if((static_cast<void **>(cData) == &(m.second.get<2>())) && (m.second.get<1>() & flags)) {
      try {
        // covert the pointer
        m.second.get<0>()->invoke(interp, m.second.get<2>(), VarName, index, flags);
      } catch (...) {           // the return type of char * is painfully difficult to cope with
        return err_msg_trace_unknown;
      }
    }
  }

  // OK
  return NULL;
}

// generic "object" command handler
extern "C"
int object_handler(ClientData cd, Tcl_Interp *interp,
     int objc, Tcl_Obj * CONST objv[])
{
     // here, client data points to the singleton object
     // which is responsible for managing commands for
     // objects of a given type

     class_handler_base *chb = reinterpret_cast<class_handler_base*>(cd);

     // the command name has the form 'pXXX' where XXX is the address
     // of the "this" object

     string const str(Tcl_GetString(objv[0]));
     istringstream ss(str);
     char dummy;
     void *p;
     ss >> dummy >> p;

     try
     {
          string methodName(Tcl_GetString(objv[1]));
          policies &pol = chb->get_policies(methodName);

          chb->invoke(p, interp, objc, objv, pol);

          post_process_policies(interp, pol, objv, true);
     }
     catch (std::exception const &e)
     {
          Tcl_SetResult(interp, const_cast<char*>(e.what()), TCL_VOLATILE);
          return TCL_ERROR;
     }
     catch (...)
     {
          char msg[] = "Unknown error.";
          Tcl_SetResult(interp, msg, TCL_STATIC);
          return TCL_ERROR;
     }

     return TCL_OK;
}

// generic "constructor" command
extern "C"
int constructor_handler(ClientData cd, Tcl_Interp *interp,
     int objc, Tcl_Obj * CONST objv[])
{
     // here, client data points to the singleton object
     // which is responsible for managing commands for
     // objects of a given type

     class_handler_base *chb = reinterpret_cast<class_handler_base*>(cd);

     callback_map::iterator it = gdb_->constructors.find(interp);
     if (it == gdb_->constructors.end())
     {
          char msg[] = "Trying to invoke non-existent callback (wrong interpreter?)";
          Tcl_SetResult(interp,
               msg,
               TCL_STATIC);
          return TCL_ERROR;
     }
     
     string className(Tcl_GetString(objv[0]));
     callback_interp_map::iterator iti = it->second.find(className);
     if (iti == it->second.end())
     {
          char msg[] = "Trying to invoke non-existent callback (wrong class name?)";
          Tcl_SetResult(interp,
               msg,
               TCL_STATIC);
          return TCL_ERROR;
     }
     
     policies_interp_map::iterator piti;
     if (find_policies(interp, className, piti) == false)
     {
          char msg[] = "Trying to invoke callback with no known policies";
          Tcl_SetResult(interp,
               msg,
               TCL_STATIC);
          return TCL_ERROR;
     }

     policies &pol = piti->second;

     try
     {
       iti->second->invoke(interp, objc, objv, pol, cd);

          // if everything went OK, the result is the address of the
          // new object in the 'pXXX' form
          // - we can create a new command with this name

          Tcl_CreateObjCommand(interp,
               Tcl_GetString(Tcl_GetObjResult(interp)),
               object_handler, static_cast<ClientData>(chb), 0);
     }
     catch (std::exception const &e)
     {
          Tcl_SetResult(interp, const_cast<char*>(e.what()), TCL_VOLATILE);
          return TCL_ERROR;
     }
     catch (...)
     {
          char msg[] = "Unknown error.";
          Tcl_SetResult(interp, msg, TCL_STATIC);
          return TCL_ERROR;
     }

     return TCL_OK;
}

} // unnamed namespace

Tcl::details::no_init_type Tcl::no_init;


policies & policies::factory(string const &name)
{
     factory_ = name;
     return *this;
}

policies & policies::sink(int index)
{
     sinks_.push_back(index);
     return *this;
}

policies & policies::variadic()
{
     variadic_ = true;
     return *this;
}

policies Tcl::factory(string const &name)
{
     return policies().factory(name);
}

policies Tcl::sink(int index)
{
     return policies().sink(index);
}

policies Tcl::variadic()
{
     return policies().variadic();
}


class_handler_base::class_handler_base()
{
     // default policies for the -delete command
     policies_["-delete"] = policies();
}

void class_handler_base::register_method(string const &name,
     shared_ptr<object_cmd_base> ocb, policies const &p)
{
     methods_[name] = ocb;
     policies_[name] = p;
}

policies & class_handler_base::get_policies(string const &name)
{
     policies_map_type::iterator it = policies_.find(name);
     if (it == policies_.end())
     {
          throw tcl_error("Trying to use non-existent policy: " + name);
     }

     return it->second;
}


object::object()
     : interp_(0)
{
     obj_ = Tcl_NewObj();
     Tcl_IncrRefCount(obj_);
}

object::object(bool b)
     : interp_(0)
{
     obj_ = Tcl_NewBooleanObj(b);
     Tcl_IncrRefCount(obj_);
}

object::object(char const *buf, size_t size)
     : interp_(0)
{
     obj_ = Tcl_NewByteArrayObj(
          reinterpret_cast<unsigned char const *>(buf),
          static_cast<int>(size));
     Tcl_IncrRefCount(obj_);
}

object::object(double d)
     : interp_(0)
{
     obj_ = Tcl_NewDoubleObj(d);
     Tcl_IncrRefCount(obj_);
}

object::object(int i)
     : interp_(0)
{
     obj_ = Tcl_NewIntObj(i);
     Tcl_IncrRefCount(obj_);
}

object::object(long i)
     : interp_(0)
{
     obj_ = Tcl_NewLongObj(i);
     Tcl_IncrRefCount(obj_);
}

object::object(long long i)
     : interp_(0)
{
     obj_ = Tcl_NewWideIntObj(i);
     Tcl_IncrRefCount(obj_);
}

object::object(char const *s)
     : interp_(0)
{
     obj_ = Tcl_NewStringObj(s, -1);
     Tcl_IncrRefCount(obj_);
}

object::object(string const &s)
     : interp_(0)
{
     obj_ = Tcl_NewStringObj(s.data(), static_cast<int>(s.size()));
     Tcl_IncrRefCount(obj_);
}

object::object(std::vector<std::string> const &v)
     : interp_(0)
{
  std::vector<Tcl_Obj*> tv(v.size());
  for(unsigned int i=0; i<v.size(); i++)
    tv[i] = Tcl_NewStringObj(v[i].c_str(), -1);
  obj_ = Tcl_NewListObj(v.size(), v.empty() ? NULL : &tv[0]);
  Tcl_IncrRefCount(obj_);
}

object::object(Tcl_Obj *o, bool shared)
     : interp_(0)
{
     init(o, shared);
}

object::object(object const &other, bool shared)
     : interp_(other.get_interp())
{
     init(other.obj_, shared);
}

void object::init(Tcl_Obj *o, bool shared)
{
     if (shared)
     {
          obj_ = o;
     }
     else
     {
          obj_ = Tcl_DuplicateObj(o);
     }
     Tcl_IncrRefCount(obj_);
}

object::~object()
{
     Tcl_DecrRefCount(obj_);
}

object & object::assign(bool b)
{
     Tcl_SetBooleanObj(obj_, b);
     return *this;
}

object & object::resize(size_t size)
{
     Tcl_SetByteArrayLength(obj_, static_cast<int>(size));
     return *this;
}

object & object::assign(char const *buf, size_t size)
{
     Tcl_SetByteArrayObj(obj_,
          reinterpret_cast<unsigned char const *>(buf),
          static_cast<int>(size));
     return *this;
}

object & object::assign(double d)
{
     Tcl_SetDoubleObj(obj_, d);
     return *this;
}

object & object::assign(int i)
{
     Tcl_SetIntObj(obj_, i);
     return *this;
}

object & object::assign(long i)
{
     Tcl_SetLongObj(obj_, i);
     return *this;
}

object & object::assign(long long i)
{
     Tcl_SetWideIntObj(obj_, i);
     return *this;
}

object & object::assign(char const *s)
{
     Tcl_SetStringObj(obj_, s, -1);
     return *this;
}

object & object::assign(string const &s)
{
     Tcl_SetStringObj(obj_, s.data(), static_cast<int>(s.size()));
     return *this;
}

object & object::assign(std::vector<std::string> const &s) {
  object(s).swap(*this);
  return *this;
}

object & object::assign(object const &other)
{
     object(other).swap(*this);
     return *this;
}

object & object::assign(Tcl_Obj *o)
{
     object(o).swap(*this);
     return *this;
}

object & object::swap(object &other)
{
     std::swap(obj_, other.obj_);
     std::swap(interp_, other.interp_);
     return *this;
}

template <>
bool object::get<bool>(interpreter &i) const
{
     int retVal;
     int res = Tcl_GetBooleanFromObj(i.get(), obj_, &retVal);
     if (res != TCL_OK)
     {
          throw tcl_error(i.get());
     }

     return retVal != 0;
}

template <>
vector<char> object::get<vector<char> >(interpreter &) const
{
     size_t size;
     char const *buf = get(size);
     return vector<char>(buf, buf + size);
}

template <>
double object::get<double>(interpreter &i) const
{
     double retVal;
     int res = Tcl_GetDoubleFromObj(i.get(), obj_, &retVal);
     if (res != TCL_OK)
     {
          throw tcl_error(i.get());
     }

     return retVal;
}

template <>
int object::get<int>(interpreter &i) const
{
     int retVal;

     int res = Tcl_GetIntFromObj(i.get(), obj_, &retVal);
     if (res != TCL_OK)
     {
          throw tcl_error(i.get());
     }

     return retVal;
}

template <>
long object::get<long>(interpreter &i) const
{
     long retVal;
     int res = Tcl_GetLongFromObj(i.get(), obj_, &retVal);
     if (res != TCL_OK)
     {
          throw tcl_error(i.get());
     }

     return retVal;
}

template <>
long long object::get<long long>(interpreter &i) const
{
     long long retVal;
     int res = Tcl_GetWideIntFromObj(i.get(), obj_, &retVal);
     if (res != TCL_OK)
     {
          throw tcl_error(i.get());
     }

     return retVal;
}

template <>
char const * object::get<char const *>(interpreter &) const
{
     return get();
}

template <>
string object::get<string>(interpreter &) const {
  return get_string();
}

template <>
vector<string> object::get<vector<string> >(interpreter &i) const {
  int len;
  int res = Tcl_ListObjLength(i.get(), obj_, &len);
  
  if (res != TCL_OK) throw tcl_error(i.get());

  vector<string> rv(len);
  for(int index = 0; index < len; index++) {
    Tcl_Obj *o;
    Tcl_ListObjIndex(i.get(), obj_, index, &o);
    rv[index] = string(Tcl_GetString(o));
  }

  return rv;

}

string object::get_string() const {
  int len;
  char const *buf = Tcl_GetStringFromObj(obj_, &len);
  return string(buf, buf + len);
}

char const * object::get() const
{
     return Tcl_GetString(obj_);
}

char const * object::get(size_t &size) const
{
     int len;
     unsigned char *buf = Tcl_GetByteArrayFromObj(obj_, &len);
     size = len;
     return const_cast<char const *>(reinterpret_cast<char *>(buf));
}

size_t object::length(interpreter &i) const
{
     int len;
     int res = Tcl_ListObjLength(i.get(), obj_, &len);

     if (res != TCL_OK)
     {
          throw tcl_error(i.get());
     }

     return static_cast<size_t>(len);
}

object object::at(interpreter &i, size_t index) const
{
     Tcl_Obj *o;
     int res = Tcl_ListObjIndex(i.get(), obj_, static_cast<int>(index), &o);
     if (res != TCL_OK)
     {
          throw tcl_error(i.get());
     }
     if (o == NULL)
     {
          throw tcl_error("Index out of range.");
     }

     return object(o);
}

object & object::append(interpreter &i, object const &o)
{
     int res = Tcl_ListObjAppendElement(i.get(), obj_, o.obj_);
     if (res != TCL_OK)
     {
          throw tcl_error(i.get());
     }

     return *this;
}

object & object::append_list(interpreter &i, object const &o)
{
     int res = Tcl_ListObjAppendList(i.get(), obj_, o.obj_);
     if (res != TCL_OK)
     {
          throw tcl_error(i.get());
     }

     return *this;
}

object & object::replace(interpreter &i, size_t index, size_t count,
     object const &o)
{
     int res = Tcl_ListObjReplace(i.get(), obj_,
          static_cast<int>(index), static_cast<int>(count),
          1, &(o.obj_));
     if (res != TCL_OK)
     {
          throw tcl_error(i.get());
     }

     return *this;
}

object & object::replace_list(interpreter &i, size_t index, size_t count,
     object const &o)
{
     int objc;
     Tcl_Obj **objv;

     int res = Tcl_ListObjGetElements(i.get(), o.obj_, &objc, &objv);
     if (res != TCL_OK)
     {
          throw tcl_error(i.get());
     }

     res = Tcl_ListObjReplace(i.get(), obj_,
          static_cast<int>(index), static_cast<int>(count),
          objc, objv);
     if (res != TCL_OK)
     {
          throw tcl_error(i.get());
     }

     return *this;
}

void object::set_interp(Tcl_Interp *interp)
{
     interp_ = interp;
}

Tcl_Interp * object::get_interp() const
{
     return interp_;
}


interpreter::interpreter()
{
     interp_ =  Tcl_CreateInterp();
     owner_ = true;
}

interpreter::interpreter(Tcl_Interp *interp, bool owner)
{
     interp_ =  interp;
     owner_ = owner;
}

interpreter::~interpreter()
{
     if (owner_)
     {
          // clear all callback info belonging to this interpreter
          clear_definitions(interp_);

          Tcl_DeleteInterp(interp_);
     }
}

void interpreter::make_safe()
{
     int cc = Tcl_MakeSafe(interp_);
     if (cc != TCL_OK)
     {
          throw tcl_error(interp_);
     }
}

result interpreter::eval(string const &script)
{
     int cc = Tcl_Eval(interp_, script.c_str());
     if (cc != TCL_OK)
     {
          throw tcl_error(interp_);
     }

     return result(interp_);
}

result interpreter::eval(istream &s)
{
     string str(
          istreambuf_iterator<char>(s.rdbuf()),
          istreambuf_iterator<char>()
     );
     return eval(str);
}

result interpreter::eval(object const &o)
{
     int cc = Tcl_EvalObjEx(interp_, o.get_object(), 0);
     if (cc != TCL_OK)
     {
          throw tcl_error(interp_);
     }

     return result(interp_);
}

void interpreter::pkg_provide(string const &name, string const &version)
{
     int cc = Tcl_PkgProvide(interp_, name.c_str(), version.c_str());
     if (cc != TCL_OK)
     {
          throw tcl_error(interp_);
     }
}

void interpreter::create_alias(string const &cmd,
     interpreter &targetInterp, string const &targetCmd)
{
     int cc = Tcl_CreateAlias(interp_, cmd.c_str(),
          targetInterp.interp_, targetCmd.c_str(), 0, 0);
     if (cc != TCL_OK)
     {
          throw tcl_error(interp_);
     }
}

void interpreter::clear_definitions(Tcl_Interp *interp)
{
  // delete all callbacks that were registered for given interpreter
  if(gdb_->callbacks.count(interp)) {
    callback_interp_map &imap = gdb_->callbacks[interp];
    for (callback_interp_map::iterator it2 = imap.begin();
         it2 != imap.end(); ++it2) {
      Tcl_DeleteCommand(interp, it2->first.c_str());
    }
    gdb_->callbacks.erase(interp);
  }

  // delete all trace functions
  if(gdb_->traces.count(interp)) {
    trace_interp_map &tmap = gdb_->traces[interp];
    trace_interp_map::iterator it, end;
    for(it=tmap.begin(), end=tmap.end(); it!=end; it++) {
      if(it->first.second == "") {
        string VarName = it->first.first;
        BOOST_FOREACH(trace_record& m, it->second) {
          if(m.second.get<1>() & TCL_TRACE_READS)
            Tcl_UntraceVar(interp, VarName.c_str(), TCL_TRACE_READS, 
                           trace_handler, &(m.second.get<2>()));
          if(m.second.get<1>() &TCL_TRACE_WRITES)
            Tcl_UntraceVar(interp, VarName.c_str(), TCL_TRACE_WRITES, 
                           trace_handler, &(m.second.get<2>()));
        }
      } else {
        string VarName = it->first.first;
        string index = it->first.second;
        BOOST_FOREACH(trace_record& m, it->second) {
          if(m.second.get<1>() & TCL_TRACE_READS)
            Tcl_UntraceVar2(interp, VarName.c_str(), index.c_str(), TCL_TRACE_READS, 
                           trace_handler, &(m.second.get<2>()));
          if(m.second.get<1>() &TCL_TRACE_WRITES)
            Tcl_UntraceVar2(interp, VarName.c_str(), index.c_str(), TCL_TRACE_WRITES, 
                           trace_handler, &(m.second.get<2>()));
        }
      }
    }
    gdb_->traces.erase(interp);
  }

  // delete all constructors
  if(gdb_->constructors.count(interp)) {
    callback_interp_map &imap = gdb_->constructors[interp];
    for (callback_interp_map::iterator it2 = imap.begin();
         it2 != imap.end(); ++it2) {
      Tcl_DeleteCommand(interp, it2->first.c_str());
    }
    gdb_->constructors.erase(interp);
  }

  // delete all call policies
  gdb_->call_policies.erase(interp);

  // delete all object handlers
  // (we have to assume that all living objects were destroyed,
  // otherwise Bad Things will happen)
  gdb_->class_handlers.erase(interp);
}

void interpreter::add_function(string const &name,
                               shared_ptr<callback_base> cb, policies const &p, 
                               ClientData cData)
{
  // add gdb ref
  if(db_.use_count() == 0) db_ = gdb_;

     Tcl_CreateObjCommand(interp_, name.c_str(),
          callback_handler, cData, 0);

     gdb_->callbacks[interp_][name] = cb;
     gdb_->call_policies[interp_][name] = p;
}

void interpreter::add_trace(const string& VarName, unsigned int *index,  
                            const std::string& FunName,
                            shared_ptr<trace_base> proc,
                            void * cData, int flag) {
  // add gdb ref
  if(db_.use_count() == 0) db_ = gdb_;

  // empty function name is not allowed. 
  // Due to the standard result, no return or error is indicated. 
  // This is at user's risk.
  if(FunName.empty()) return;   
  
  // record the variable name and index into the map name
  pair<string, string> map_name(VarName, "");
  if(index != NULL) map_name.second = boost::lexical_cast<string>(*index);
  
  trace_record_map& trecord = gdb_->traces[interp_][map_name];
  
  // record it in our own maps
  if(trecord.count(FunName)) { // already recorded
    if(cData != trecord[FunName].get<2>()) {
      // user error, define the same trace function with the same variable 
      // but with different cData is not supported
      assert(0 == "Fail to add a trace on the same variable using the same function name but with different client data!");
      return;
    }
    trecord[FunName].get<1>() |= flag;
  } else {                        // new
    trecord[FunName].get<0>() = proc;
    trecord[FunName].get<1>() = flag;
    trecord[FunName].get<2>() = cData;
  }
  if(index == NULL)
    Tcl_TraceVar(interp_, VarName.c_str(), flag, 
                 trace_handler, &(trecord[FunName].get<2>()));
  else
    Tcl_TraceVar2(interp_, VarName.c_str(), map_name.second.c_str(), flag, 
                  trace_handler, &(trecord[FunName].get<2>()));
}

void interpreter::remove_trace(const string& VarName, unsigned int *index, 
                               const std::string& FunName, int flag) {
  if(!gdb_->traces.count(interp_)) return; // interpreter not found
  
  // get variable name
  pair<string, string> map_name(VarName, "");
  if(index != NULL) map_name.second = boost::lexical_cast<string>(*index);
  
  if(!gdb_->traces[interp_].count(map_name)) return; // variable not found
  trace_record_map& trecord = gdb_->traces[interp_][map_name];

  if(FunName == "") {            // all
    BOOST_FOREACH(trace_record& m, trecord) {
      if(m.second.get<1>() & flag) { // need a untrace operation
        // stupid tcl cannot untrace read and write traces at the same time if they are defined separately
        if(index == NULL) {
          if(m.second.get<1>() & flag & TCL_TRACE_READS)
            Tcl_UntraceVar(interp_, VarName.c_str(), TCL_TRACE_READS, 
                           trace_handler, &(m.second.get<2>()));
          if(m.second.get<1>() & flag &TCL_TRACE_WRITES)
            Tcl_UntraceVar(interp_, VarName.c_str(), TCL_TRACE_WRITES, 
                           trace_handler, &(m.second.get<2>()));
        }
        else { 
          if(m.second.get<1>() & flag & TCL_TRACE_READS)
            Tcl_UntraceVar2(interp_, VarName.c_str(), map_name.second.c_str(), TCL_TRACE_READS, 
                            trace_handler, &(m.second.get<2>()));
          if(m.second.get<1>() & flag & TCL_TRACE_WRITES)
            Tcl_UntraceVar2(interp_, VarName.c_str(), map_name.second.c_str(), TCL_TRACE_WRITES, 
                            trace_handler, &(m.second.get<2>()));
        }
      }
      m.second.get<1>() &= (~flag);
    }
    trace_record_map::iterator it, end;
    for(it = trecord.begin(), end = trecord.end();
        it != end; ) {
      if(it->second.get<1>() == 0) trecord.erase(it++);
      else it++;
    }
  } else {                       // a specific trace
    if(!trecord.count(FunName)) return; // function not found
    if(trecord[FunName].get<1>() & flag) { // need a untrace operation
        if(index == NULL) {
          if(trecord[FunName].get<1>() & flag & TCL_TRACE_READS)
            Tcl_UntraceVar(interp_, VarName.c_str(), TCL_TRACE_READS, 
                           trace_handler, &(trecord[FunName].get<2>()));
          if(trecord[FunName].get<1>() & flag & TCL_TRACE_WRITES)
            Tcl_UntraceVar(interp_, VarName.c_str(), TCL_TRACE_WRITES, 
                           trace_handler, &(trecord[FunName].get<2>()));
        }
        else { 
          if(trecord[FunName].get<1>() & flag & TCL_TRACE_READS)
            Tcl_UntraceVar2(interp_, VarName.c_str(), map_name.second.c_str(), TCL_TRACE_READS, 
                            trace_handler, &(trecord[FunName].get<2>()));
          if(trecord[FunName].get<1>() & flag & TCL_TRACE_WRITES)
            Tcl_UntraceVar2(interp_, VarName.c_str(), map_name.second.c_str(), TCL_TRACE_WRITES, 
                            trace_handler, &(trecord[FunName].get<2>()));
        }
    }
    trecord[FunName].get<1>() &= (~flag);
    if(trecord[FunName].get<1>() == 0)
      trecord.erase(FunName);
  }

  if(trecord.empty()) gdb_->traces[interp_].erase(map_name);
}

void interpreter::add_class(string const &name,
     shared_ptr<class_handler_base> chb)
{
  // add gdb ref
  if(db_.use_count() == 0) db_ = gdb_;

     gdb_->class_handlers[interp_][name] = chb;
}

void interpreter::add_constructor(string const &name,
     shared_ptr<class_handler_base> chb, shared_ptr<callback_base> cb,
     policies const &p)
{
  // add gdb ref
  if(db_.use_count() == 0) db_ = gdb_;

     Tcl_CreateObjCommand(interp_, name.c_str(),
          constructor_handler, static_cast<ClientData>(chb.get()), 0);

     gdb_->constructors[interp_][name] = cb;
     gdb_->call_policies[interp_][name] = p;
}


int tcl_cast<int>::from(Tcl_Interp *interp, Tcl_Obj *obj)
{
     int res;
     int cc = Tcl_GetIntFromObj(interp, obj, &res);
     if (cc != TCL_OK)
     {
          throw tcl_error(interp);
     }
     
     return res;
}

Tcl_Obj * tcl_cast<int>::to(Tcl_Interp *, int const & v) {
  return Tcl_NewIntObj(v);
}

long tcl_cast<long>::from(Tcl_Interp *interp, Tcl_Obj *obj)
{
     long res;
     int cc = Tcl_GetLongFromObj(interp, obj, &res);
     if (cc != TCL_OK)
     {
          throw tcl_error(interp);
     }
     
     return res;
}

Tcl_Obj * tcl_cast<long>::to(Tcl_Interp *, long const & v) {
  return Tcl_NewLongObj(v);
}

long long tcl_cast<long long>::from(Tcl_Interp *interp, Tcl_Obj *obj)
{
     long long res;
     int cc = Tcl_GetWideIntFromObj(interp, obj, &res);
     if (cc != TCL_OK)
     {
          throw tcl_error(interp);
     }
     
     return res;
}

Tcl_Obj * tcl_cast<long long>::to(Tcl_Interp *, long long const & v) {
  return Tcl_NewWideIntObj(v);
}

bool tcl_cast<bool>::from(Tcl_Interp *interp, Tcl_Obj *obj)
{
     int res;
     int cc = Tcl_GetBooleanFromObj(interp, obj, &res);
     if (cc != TCL_OK)
     {
          throw tcl_error(interp);
     }
     
     return res != 0;
}

Tcl_Obj * tcl_cast<bool>::to(Tcl_Interp *, bool const & v) {
  return Tcl_NewBooleanObj(v);
}

double tcl_cast<double>::from(Tcl_Interp *interp, Tcl_Obj *obj)
{
     double res;
     int cc = Tcl_GetDoubleFromObj(interp, obj, &res);
     if (cc != TCL_OK)
     {
          throw tcl_error(interp);
     }
     
     return res;
}

Tcl_Obj * tcl_cast<double>::to(Tcl_Interp *, double const & v) {
  return Tcl_NewDoubleObj(v);
}

string tcl_cast<string>::from(Tcl_Interp *, Tcl_Obj *obj)
{
     return Tcl_GetString(obj);
}

Tcl_Obj * tcl_cast<string>::to(Tcl_Interp *, string const & v) {
  return Tcl_NewStringObj(v.c_str(), -1);
}

char const * tcl_cast<char const *>::from(Tcl_Interp *, Tcl_Obj *obj)
{
     return Tcl_GetString(obj);
}

Tcl_Obj * tcl_cast<char const *>::to(Tcl_Interp *, char const * const & v) {
  return Tcl_NewStringObj(v, -1);
}

vector<string> tcl_cast<vector<string> >::from(Tcl_Interp * interp, Tcl_Obj *obj)
{
  int len;
  int res = Tcl_ListObjLength(interp, obj, &len);
  
  if (res != TCL_OK) throw tcl_error(interp);

  vector<string> rv(len);
  for(int index = 0; index < len; index++) {
    Tcl_Obj *o;
    Tcl_ListObjIndex(interp, obj, index, &o);
    rv[index] = string(Tcl_GetString(o));
  }

  return rv;
}

Tcl_Obj * tcl_cast<vector<string> >::to(Tcl_Interp *, vector<string> const & v) {
  std::vector<Tcl_Obj*> tv(v.size());
  for(unsigned int i=0; i<v.size(); i++)
    tv[i] = Tcl_NewStringObj(v[i].c_str(), -1);
  return Tcl_NewListObj(v.size(), v.empty() ? NULL : &tv[0]);
}

object tcl_cast<object>::from(Tcl_Interp *interp, Tcl_Obj *obj)
{
     object o(obj);
     o.set_interp(interp);

     return o;
}

Tcl_Obj * tcl_cast<object>::to(Tcl_Interp *, object const & v) {
  return Tcl_DuplicateObj(v.get_object());
}
