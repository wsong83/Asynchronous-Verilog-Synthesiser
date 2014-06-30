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
 * RangeElement
 * 20/05/2014   Wei Song
 *
 *
 */

#ifndef _CPP_RANGE_ELEMENT_H_
#define _CPP_RANGE_ELEMENT_H_

#include <utility>
#include <ostream>
#include <algorithm>
#include <boost/tuple/tuple.hpp>
#include <boost/lexical_cast.hpp>
#include "cpp_range_define.hpp"
#include "cpp_range_util_def.hpp"

namespace CppRange {

  ////////////////////////////////////////////////// 
  // RangeElement
  // 
  // a single dimension range expression
  // the basic element of a multi-dimensional range
  //
  //////////////////////////////////////////////////

  template <class T>
  class RangeElement {
  private:
    bool initialized;           // false if constructed without range value
    std::pair<T, T> r_pair;     // specific range expression

  public:
    //////////////////////////////////////////////
    // constructors

    RangeElement();
    RangeElement(const T&);                             // single bit range
    RangeElement(const T& upper_bound, const T& lower_bound);
    RangeElement(const std::string&);                   // build from parsing a string

    //////////////////////////////////////////////
    // Helpers

    // data accessers
    void upper(const T&);                               // set a new upper bound 
    const T& upper() const;                             // get the upper bound
    void lower(const T&);                               // set a new lower bound
    const T& lower() const;                             // get the lower bound

    T size() const;                                     // get the size of the range
    bool valid() const;                                 // check the range is valid
    bool empty() const;                                 // ? this is empty
    bool in(T num) const;                               // ? num belongs to this range
    bool subset(const RangeElement& r) const;           // ? this is a subset of r
    bool proper_subset(const RangeElement& r) const;    // ? this is a proper subset of r
    bool superset(const RangeElement& r) const;         // ? this is a superset of r
    bool proper_superset(const RangeElement& r) const;  // ? this is a proper superset of r
    bool singleton() const;                             // ? this is a singleton range 
    bool equal(const RangeElement& r) const;            // ? this == r 
    bool connected(const RangeElement& r) const;        // ? this and r are connected
    bool less(const RangeElement& r) const;             // weak order compare
    bool overlap(const RangeElement& r) const;          // this & r != []
    bool disjoint(const RangeElement& r) const;         // this & r == []
    RangeElement combine(const RangeElement& r) const;  // get the union of this and r
    RangeElement hull(const RangeElement& r) const;     // get the minimal superset of the union
                                                        // of this and r
    RangeElement intersection(const RangeElement& r) const;
                                                        // get the intersection of this and r
    RangeElement complement(const RangeElement& r) const;
                                                        // subtract r from this range
    boost::tuple<RangeElement, RangeElement, RangeElement>
    divide(const RangeElement& r) const;                // standard divide/partition this and r
    
    std::ostream& streamout(std::ostream& os) const;    // stream out the range
    std::string toString() const;                       // simple conversion to string 
  };

}

#include "cpp_range_util.hpp"

namespace CppRange {

  /////////////////////////////////////////////
  // implementation of class methods

  // constructors
  
  // default
  template<class T> inline
  RangeElement<T>::RangeElement()
    : initialized(false), r_pair(T(0), T(0)) {}

  // single bit range
  template<class T> inline
  RangeElement<T>::RangeElement(const T& r)
    : initialized(true), r_pair(r, r) {}

  // bit range
  template<class T> inline
  RangeElement<T>::RangeElement(const T& rh, const T& rl)
    : initialized(true), r_pair(rh, rl) {}

  // parse a range text
  template<class T> inline
  RangeElement<T>::RangeElement(const std::string& str) {
    *this = parse_range<T>(str);
  }

  // helpers

  // bound accessor
  template<class T> inline
  void RangeElement<T>::upper(const T& upper_bound) {
    initialized = true; 
    r_pair.first = upper_bound;
  }
  
  template<class T> inline
  const T& RangeElement<T>::upper() const { 
    return r_pair.first;
  }
  
  template<class T> inline
  void RangeElement<T>::lower(const T& lower_bound) { 
    initialized = true; 
    r_pair.second = lower_bound;
  }
  
  template<class T> inline
  const T& RangeElement<T>::lower() const { 
    return r_pair.second;
  }

  // get the size of the range
  template<class T> inline
  T RangeElement<T>::size() const {
    if(!valid() || !initialized) return T(0);
    return r_pair.first - r_pair.second + min_unit<T>();
  }

  // check the range has a valid expression
  template<class T> inline
  bool RangeElement<T>::valid() const {
    return !(upper() < lower());
  }
    
  // check whether the range is empty
  template<class T> inline
  bool RangeElement<T>::empty() const {
    return size() == T(0);
  }

  // check whether a number is in this range
  template<class T> inline
  bool RangeElement<T>::in(T num) const {
    return !empty() && !(num > upper()) && !(num < lower());
  }
  
  // check this is a subset of r 
  template<class T> inline
  bool RangeElement<T>::subset(const RangeElement& r) const {
    if(!valid() || !r.valid()) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_InvalidRange());
#endif
      return false;
    }
    if(empty()) return true;
    if(r.empty()) return false;
    return !(r.upper() < upper()) && !(lower() < r.lower());      
  }

  // check this is a proper subset of r 
  template<class T> inline
  bool RangeElement<T>::proper_subset(const RangeElement& r) const {
    return subset(r) && !equal(r);      
  }

  // check this is a superset of r
  template<class T> inline
  bool RangeElement<T>::superset(const RangeElement& r) const {
    if(!valid() || !r.valid()) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_InvalidRange());
#endif
      return false;
    }
    if(r.empty()) return true;
    if(empty()) return false;
    return !(upper() < r.upper()) && !(r.lower() < lower());      
  }

  // check this is a proper superset of r 
  template<class T> inline
  bool RangeElement<T>::proper_superset(const RangeElement& r) const {
    return superset(r) && !equal(r);
  }

  // check whether this range is a singleton range
  template<class T> inline
  bool RangeElement<T>::singleton() const {
    if(!valid()) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_InvalidRange());
#endif
      return false;
    }
    return empty() || size() == min_unit<T>();
  }  

  // check whether this is equal with r
  template<class T> inline
  bool RangeElement<T>::equal(const RangeElement& r) const {
    if(!valid() || !r.valid()) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_InvalidRange());
#endif
      return false;
    }
    if(empty()) return r.empty();
    if(r.empty()) return false;
    return (upper() == r.upper()) && (r.lower() == lower());
  }

  // check whether r is adjacent to this range
  template<class T> inline
  bool RangeElement<T>:: connected(const RangeElement& r) const {
    if(!valid() || !r.valid()) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_InvalidRange());
#endif
      return false;
    }
    if(empty() || r.empty()) return false;
    return 
      (!(upper() + min_unit<T>() < r.lower()) 
       && !(r.upper() + min_unit<T>() < lower()));
  }

  // weak order
  template<class T> inline
  bool RangeElement<T>::less(const RangeElement& r) const {
    if(!valid() || !r.valid()) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_InvalidRange());
#endif
      return false;
    }
    if(empty()) return !r.empty();
    if(r.empty()) return false;
    if(upper() < r.upper()) return true;
    if(upper() > r.upper()) return false;
    if(lower() > r.lower()) return true;
    if(lower() < r.lower()) return false;
    return false;
  }
  
  // check whether r has a shared range with this range
  template<class T> inline
  bool RangeElement<T>::overlap(const RangeElement& r) const {
    if(!valid() || !r.valid()) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_InvalidRange());
#endif
      return false;
    }
    if(empty() || r.empty()) return false;
    return !(upper() < r.lower()) && !(r.upper() < lower());
  }

  // check whether r does not have a shared range with this range
  template<class T> inline
  bool RangeElement<T>::disjoint(const RangeElement& r) const {
    if(!valid() || !r.valid()) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_InvalidRange());
#endif
      return false;
    }
    if(empty() || r.empty()) return true;
    return (upper() < r.lower()) || (r.upper() < lower());
  }
  
  // union to ranges
  template<class T> inline
  RangeElement<T> RangeElement<T>::combine(const RangeElement& r) const {
    if(!valid() || !r.valid()) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_InvalidRange());
#endif
      return RangeElement();
    }
    if(empty()) return r;
    if(r.empty()) return *this;
    if(connected(r)) return hull(r); 
#ifndef CPP_RANGE_NO_EXCEPTION
    throw(RangeException_NonPresentable(toString(), r.toString(), "|"));
#endif
    return RangeElement(); // or throw an exception
  }
  
  // the minimal range to contain both this and r
  template<class T> inline
  RangeElement<T> RangeElement<T>::hull(const RangeElement& r) const {
    if(!valid() || !r.valid()) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_InvalidRange());
#endif
      return RangeElement();
    }
    if(empty()) return r;
    if(r.empty()) return *this;
    return RangeElement(std::max(upper(), r.upper()),
                        std::min(lower(), r.lower()));
  }
  
  // intersection
  template<class T> inline
  RangeElement<T> RangeElement<T>::intersection(const RangeElement& r) const {
    if(!valid() || !r.valid()) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_InvalidRange());
#endif
      return RangeElement();
    }
    if(empty() || r.empty()) return RangeElement();
    return RangeElement(std::min(upper(), r.upper()),
                        std::max(lower(), r.lower()));
  }

  // subtraction
  template<class T> inline
  RangeElement<T> RangeElement<T>::complement(const RangeElement& r) const {
    if(!valid() || !r.valid()) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_InvalidRange());
#endif
      return RangeElement();
    }
    if(empty()) return RangeElement();
    if(r.empty()) return *this;
    
    RangeElement RAnd = intersection(r);
    if(RAnd.empty()) return *this;
    if(equal(RAnd)) return RangeElement();
    
    RangeElement rv(*this);
    if(upper() != RAnd.upper())
      rv.lower(RAnd.upper() + min_unit<T>());
    if(lower() != RAnd.lower())
      rv.upper(RAnd.lower() - min_unit<T>());
#ifndef CPP_RANGE_NO_EXCEPTION
    if(!rv.valid()) 
      throw(RangeException_NonPresentable(toString(), r.toString(), "complement()"));
#endif    
    return rv;
  }

  // normalize divide
  template<class T> inline
  boost::tuple<RangeElement<T>, RangeElement<T>, RangeElement<T> > 
  RangeElement<T>::divide(const RangeElement& r) const {
    boost::tuple<RangeElement, RangeElement, RangeElement > rv;
    if(!valid() || !r.valid()) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_InvalidRange());
#endif
      return rv;
    }
    if(empty() || r.empty()) {
      boost::get<1>(rv) = hull(r);
      return rv;
    }

    RangeElement RAnd = intersection(r);
    if(RAnd.empty()) {
      if(less(r)) {
        boost::get<0>(rv) = r;
        boost::get<2>(rv) = *this;
      } else {
        boost::get<2>(rv) = r;
        boost::get<0>(rv) = *this;          
      }
    } else {
      RangeElement ROr = hull(r);
      boost::get<0>(rv) = RangeElement(ROr.upper(), RAnd.upper() +  min_unit<T>());
      boost::get<1>(rv) = RAnd;
      boost::get<2>(rv) = RangeElement(RAnd.lower() -  min_unit<T>(), ROr.lower());
    }
    return rv;
  }

  // stream out the range
  template<class T> inline
  std::ostream& RangeElement<T>::streamout(std::ostream& os) const {
    os << "[";
    if(!empty()) {
      os << upper();
      if(upper() != lower()) 
    os << ":" << lower();
    }
    os << "]";
    return os;
  }

  // convert to string
  template<class T> inline
  std::string RangeElement<T>::toString() const {
    return 
      "[" +
      boost::lexical_cast<std::string>(upper()) +
      ":" +
      boost::lexical_cast<std::string>(lower()) +
      "]";
  }

  /////////////////////////////////////////////
  // overload operators

  // rhs range is less than lhs
  template <class T>
  bool operator> (const RangeElement<T>& lhs, const RangeElement<T>& rhs) {
    return rhs.less(lhs);
  }

  // rhs range is less than or equal to lhs
  template <class T>
  bool operator>= (const RangeElement<T>& lhs, const RangeElement<T>& rhs) {
    if(!lhs.valid() || !rhs.valid()) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_InvalidRange());
#endif
      return false;
    }
    return rhs.less(lhs) || lhs.equal(rhs);
  }

  // lhs range is larger than rhs
  template <class T>
  bool operator< (const RangeElement<T>& lhs, const RangeElement<T>& rhs) {
    return lhs.less(rhs);
  }

  // lhs range is larger than or equal to rhs
  template <class T>
  bool operator<= (const RangeElement<T>& lhs, const RangeElement<T>& rhs) {
    if(!lhs.valid() || !rhs.valid()) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_InvalidRange());
#endif
      return false;
    }
    return lhs.less(rhs) || lhs.equal(rhs);
  }
  
  // two ranges are equal
  template <class T>
  inline bool operator== (const RangeElement<T>& lhs, const RangeElement<T>& rhs) {
    return rhs.equal(lhs);
  }

  // two ranges are not equal
  template <class T>
  inline bool operator!= (const RangeElement<T>& lhs, const RangeElement<T>& rhs) {
    if(!lhs.valid() || !rhs.valid()) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_InvalidRange());
#endif
      return false;
    }
    return !rhs.equal(lhs);
  }

  // return the overlapped range
  // function does not check the result's validation
  template <class T>  
  RangeElement<T> operator& (const RangeElement<T>& lhs, const RangeElement<T>& rhs) {
    return lhs.intersection(rhs);
  }

  // return the combined range
  // function does not check the result's validation
  template <class T>  
  RangeElement<T> operator| (const RangeElement<T>& lhs, const RangeElement<T>& rhs) {
    return lhs.combine(rhs);
  }

  // standard out stream
  template<class T>
  std::ostream& operator<< (std::ostream& os, const RangeElement<T>& r) {
    return r.streamout(os);
  }

}


#endif
