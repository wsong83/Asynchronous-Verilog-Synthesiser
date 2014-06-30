/*
 * Copyright (c) 2014-2014 Wei Song <songw@cs.man.ac.uk> 
 *    Advanced Processor Technologies Group, School of Computer Science
 *    University of Manchester, Manchester M13 9PL UK
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *    
 *        http://www.apache.org/licenses/LICENSE-2.0
 *    
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 *
 */

/* 
 * An C++ STL static range calculation library
 * 16/06/2014   Wei Song
 *
 *
 */

#ifndef _CPP_RANGE_DEFINE_H_
#define _CPP_RANGE_DEFINE_H_

#include <exception>
#include <string>

namespace CppRange {

  //////////////////////////////////////////////////
  // Special definition for different ranges
  //////////////////////////////////////////////////

  // for all classes with default conversion from int
  template<class T>
  T min_unit() {
    return 1;
  }

  // specializations
  template<>
  inline double min_unit<double>() {
    return 0.0;
  }

  template<>
  inline float min_unit<float>() {
    return 0.0;
  }

  // error exceptions

  // To disable exception and return empty ranges silently, discomment the following MACRO
  // #define CPP_RANGE_NO_EXCEPTION


  // the base exception
  class RangeException_Base : public std::exception {
  public:
    virtual const char* what() const throw() {
      return "Exception: CppRange error";
    }

    virtual ~RangeException_Base() throw() {}
  };

  // Invalid range expression
  class RangeException_InvalidRange : public RangeException_Base {
  public:
    virtual const char* what() const throw() {
      return "Exception: Invalid range definition";
    }

    virtual ~RangeException_InvalidRange() throw() {}
  };

  // Result can not be represented
  class RangeException_NonPresentable : public RangeException_Base {
  public:
    const std::string lhs;
    const std::string rhs;
    const std::string op;

    RangeException_NonPresentable(const std::string& lhs_arg, const std::string& rhs_arg, const std::string& op_arg)
      : lhs(lhs_arg), rhs(rhs_arg), op(op_arg) {}

    virtual const char* what() const throw() {
      std::string msg( 
                      "Exception: The result of " 
                      + lhs + " " + op + " " + rhs + 
                      " cannot be represented using a Range. Use RangeMap instead."
                       );
      return msg.c_str();
    }

    virtual ~RangeException_NonPresentable() throw() {}
  };

  // range not operable
  class RangeException_NonOperable : public RangeException_Base {
  public:
    const std::string lhs;
    const std::string rhs;
    const std::string op;

    RangeException_NonOperable(const std::string& lhs_arg, const std::string& rhs_arg, const std::string& op_arg)
      : lhs(lhs_arg), rhs(rhs_arg), op(op_arg) {}

    virtual const char* what() const throw() {
      std::string msg( 
                      "Exception: " + lhs + " " + op + " " + rhs + 
                      " is not operable because more than one dimensions are different." +
                      "Use RangeMap instead.")
        ;
      return msg.c_str();
    }

    virtual ~RangeException_NonOperable() throw() {}
  };
  
  // range not comparable
  class RangeException_NonComparable : public RangeException_Base {
  public:
    const std::string lhs;
    const std::string rhs;
    const std::string op;

    RangeException_NonComparable(const std::string& lhs_arg, const std::string& rhs_arg, const std::string& op_arg)
      : lhs(lhs_arg), rhs(rhs_arg), op(op_arg) {}

    virtual const char* what() const throw() {
      std::string msg(
                      "Exception: " + lhs + " " + op + " " + rhs + 
                      " is not operable because they have different number of dimensions."
                      );
      return msg.c_str();
    }

    virtual ~RangeException_NonComparable() throw() {}
  };

}

#endif
