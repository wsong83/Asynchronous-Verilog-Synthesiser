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
 * 09/05/2014   Wei Song
 *
 *
 */

#ifndef _CPP_RANGE_H_
#define _CPP_RANGE_H_

#include <list>
#include <vector>
#include <utility>
#include <string>
#include <algorithm>
#include <ostream>
#include <boost/tuple/tuple.hpp>
#include <boost/foreach.hpp>
#include <assert.h>
#include <iostream>

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
    bool compressed;            // whether to compress single bit range when streamout
    std::pair<T, T> r_pair;     // specific range expression
  public:
    //////////////////////////////////////////////
    // data accessers
    T& first() { return r_pair.first;}
    const T& first() const { return r_pair.first;}
    T& second() { return r_pair.second;}
    const T& second() const { return r_pair.second;}

    //////////////////////////////////////////////
    // constructors

    // default to construct an range with undefined value
    RangeElement()
      : compressed(false) {}

    // single bit range
    RangeElement(const T& r, bool compress = true)
      : compressed(compress), r_pair(r, r) {}

    // bit range
    template<class Y>
    RangeElement(const T& rh, const Y& rl, bool compress = true)
      : compressed(compress), r_pair(rh, rl) {}

    // type conversion
    template <class Y>
    RangeElement(const RangeElement<Y>& r)
      : compressed(r.compressed), r_pair(r.r_pair) {}

    //////////////////////////////////////////////
    // Helpers
    RangeElement& operator= (const RangeElement& r) {
      compressed = r.compressed;
      r_pair.first = r.r_pair.first;
      r_pair.second = r.r_pair.second;
      return *this;
    }

    // set compressed
    void set_compress(bool b) { compressed = b; }

    // size of bit
    T size() const {
      return r_pair.first - r_pair.second + 1;
    } 

    // valid range expression
    bool is_valid() const {
      return !(r_pair.first < r_pair.second);
    }

    // check whether range r is enclosed in this range 
    template <class Y>
    bool is_enclosed(const RangeElement<Y>& r) const {
        return (
                !(r_pair.first < r.r_pair.first) && 
                !(r.r_pair.second < r_pair.second)
                );      
    }

    // check whether range r is equal with this range
    template <class Y>
    bool is_same(const RangeElement<Y>& r) const {
      return (
              r_pair.first == r.r_pair.first &&
              r_pair.second == r.r_pair.second
              );
    }

    // check whether r is adjacent to this range
    template <class Y>
    bool is_adjacent(const RangeElement<Y>& r) const {
      return (!(r_pair.first + 1 < r.r_pair.second) && !(r.r_pair.first + 1 < r_pair.second));
    }
      
    // weak order
    template <class Y>
    bool less(const RangeElement<Y>& r) const {
      if(r_pair.first < r.r_pair.first)
        return true;
      else if(r_pair.first > r.r_pair.first)
        return false;
      else if(r_pair.second > r.r_pair.second)
        return true;
      else if(r_pair.second < r.r_pair.second)
        return false;
      else
        return false;
    }

    // check whether r has shared range with this range
    template <class Y>
    bool is_overlapped(const RangeElement<Y>& r) const {
      return (!(r_pair.first < r.r_pair.second) && !(r.r_pair.first < r_pair.second));
    }

    // simple combine without check
    template <class Y>
    RangeElement combine(const RangeElement<Y>& r) const {
      return RangeElement(std::max(r_pair.first, r.r_pair.first),
                          std::min(r_pair.second, r.r_pair.second),
                          compressed);
    }

    // simple overlap without check
    template <class Y>
    RangeElement overlap(const RangeElement<Y>& r) const {
      return RangeElement(std::min(r_pair.first, r.r_pair.first),
                          std::max(r_pair.second, r.r_pair.second),
                          compressed);
    }
    
    // simple reduce without check
    template <class Y>
    RangeElement reduce(const RangeElement<Y>& r) const {
      RangeElement rv(*this);
      if(!(r.r_pair.second > r_pair.first) && (r_pair.second < r.r_pair.second))
        rv.r_pair.first = r.r_pair.second - 1;
      if(!(r.r_pair.first < r_pair.second) && (r_pair.first > r.r_pair.first))
        rv.r_pair.second = r.r_pair.first + 1;
       return rv;
    }

    // normalize divide
    template <class Y>
    boost::tuple<RangeElement, RangeElement, RangeElement> 
    divideBy(const RangeElement<Y>& r) const {
      RangeElement RAnd = overlap(r);
      RangeElement ROr = combine(r);
      RangeElement RMinus = ROr - RAnd;
      boost::tuple<RangeElement, RangeElement, RangeElement> rv;
      boost::get<0>(rv) = RangeElement(ROr.first(), RMinus.second(), compressed);
      boost::get<1>(rv) = RAnd;
      boost::get<2>(rv) = RangeElement(RMinus.first(), ROr.second(), compressed);
      return rv;
    }

    // stream out function
    std::ostream& streamout(std::ostream& os) const{
      os << "[";
      if(is_valid()) {
        if(compressed && r_pair.first == r_pair.second)
          os << r_pair.first;
        else 
          os << r_pair.first << ":" << r_pair.second;
      }
      os << "]";
      return os;
    }

  };

  /////////////////////////////////////////////
  // overload operators

  // rhs range is enclosed in lhs (not equal)
  template <class T, class Y>
  bool operator> (const RangeElement<T>& lhs, const RangeElement<Y>& rhs) {
    return lhs.is_enclosed(rhs) && !lhs.is_same(rhs);
  }

  // rhs range is enclosed in lhs
  template <class T, class Y>
  bool operator>= (const RangeElement<T>& lhs, const RangeElement<Y>& rhs) {
    return lhs.is_enclosed(rhs);
  }

  // lhs range is enclosed in rhs (not equal)
  template <class T, class Y>
  bool operator< (const RangeElement<T>& lhs, const RangeElement<Y>& rhs) {
    return rhs.is_enclosed(lhs) && !rhs.is_same(lhs);
  }

  // lhs range is enclosed in rhs
  template <class T, class Y>
  bool operator<= (const RangeElement<T>& lhs, const RangeElement<Y>& rhs) {
    return rhs.is_enclosed(lhs);
  }
  
  // two ranges are equal
  template <class T, class Y>
  inline bool operator== (const RangeElement<T>& lhs, const RangeElement<Y>& rhs) {
    return rhs.is_same(lhs);
  }

  // two ranges are not equal
  template <class T, class Y>
  inline bool operator!= (const RangeElement<T>& lhs, const RangeElement<Y>& rhs) {
    return !rhs.is_same(lhs);
  }

  // return the overlapped range
  // function does not check the result's validation
  template <class T, class Y>  
  RangeElement<T> operator& (const RangeElement<T>& lhs, const RangeElement<Y>& rhs) {
    return lhs.overlap(rhs);
  }

  // return the combined range
  // function does not check the result's validation
  template <class T, class Y>  
  RangeElement<T> operator| (const RangeElement<T>& lhs, const RangeElement<Y>& rhs) {
    return lhs.combine(rhs);
  }

  // return the reduced range
  // function does not check the result's validation
  template <class T, class Y>  
  RangeElement<T> operator- (const RangeElement<T>& lhs, const RangeElement<Y>& rhs) {
    return lhs.reduce(rhs);
  }

  // return the standard division (high, overlapped, low)
  template <class T, class Y>
  boost::tuple<RangeElement<T>, RangeElement<T>, RangeElement<T> > 
  operator^ (const RangeElement<T>& lhs, const RangeElement<Y>& rhs) {
    return lhs.divideBy(rhs);
  }

  // standard out stream
  template<class T>
  std::ostream& operator<< (std::ostream& os, const RangeElement<T>& r) {
    return r.streamout(os);
  }


  // ----------------------------------------------------------------- //


  ////////////////////////////////////////////////// 
  // Range
  // 
  // multi-dimensional range expression
  //
  //////////////////////////////////////////////////

  template <class T>
  class Range {
  private:
    bool compressed;                       // whether to compress single bit range when streamout
    std::vector<RangeElement<T> > r_array; // the range array

  public:
    //////////////////////////////////////////////
    // constructors
    
    // default constructor
    Range() : compressed(true) {}

    // Range with configuration
    Range(bool compress) : compressed(compress) {}

    // type convert from different ranges
    template <class Y>
    Range(const Range<Y>& r)
      : compressed(r.compressed) {
      r_array.resize(r.r_array.size());
      for(unsigned int i=0; i<r_array.size(); i++)
        r_array[i] = r.r_array[i];
    }

    // construct from a list of RangeElements
    template <class Y>
    Range(const std::list<RangeElement<Y> >& l, bool compress = true) 
      : compressed(compress) {
      r_array.resize(l.size());
      unsigned int i = 0;
      BOOST_FOREACH(const RangeElement<Y>& r, l)
	r_array[i++]=r;
    }
    // construct from a list of RangeElements
    template <class Y>
    Range(const std::vector<RangeElement<Y> >& l, bool compress = true)
      : compressed(compress), r_array(l) {}

    // construct from a list of value pairs
    template <class Y>
    Range(const std::list<std::pair<Y,Y> >& l, bool compress = true) 
      : compressed(compress) {
      r_array.resize(l.size());
      unsigned int i = 0;
      typedef std::pair<Y,Y> local_range_pair;
      BOOST_FOREACH(const local_range_pair& r, l)
	r_array[i++]=RangeElement<T>(r.first, r.second, compress);
    }

    // construct from a list of RangeElements
    template <class Y>
    Range(const std::vector<std::pair<Y,Y> >& l, bool compress = true)
      : compressed(compress) {
       r_array.resize(l.size());
      unsigned int i = 0;
      typedef std::pair<Y,Y> local_range_pair;
      BOOST_FOREACH(const local_range_pair& r, l)
	r_array[i++]=RangeElement<T>(r.first, r.second, compress);   
    }

    //////////////////////////////////////////////
    // accesser
    RangeElement<T>& operator[] (unsigned int index) {
      return r_array.at(index);
    }

    const RangeElement<T>& operator[] (unsigned int index) const {
      return r_array.at(index);
    }

    //////////////////////////////////////////////
    // Helpers

    // set compressed
    void set_compress(bool b) { compressed = b; }

    // size of dimension
    unsigned int size_dimension() const {
      return r_array.size();
    }

    // size of bits
    T size_bit() const {
      T rv(1);
      for(unsigned int i=0; i<r_array.size(); i++)
        rv = rv * r_array[i].size();
      return rv;
    }

    // add a higher dimension
    // this method is very expernsive, do not use if possible
    template <class Y>
    void push_front(const RangeElement<Y>& r) {
      r_array.insert(0, r);
      r_array[0].set_compress(compressed);
    }

    // add a lower dimension
    template <class Y>
    void push_back(const RangeElement<Y>& r) {
      r_array.push_back(r);
      r_array[r_array.size()-1].set_compress(compressed);
    }

    void pop_back() {
      r_array.pop_back();
    }

    // check whether the range expression is valid
    bool is_valid() const {
      if(r_array.empty()) return false;
      for(unsigned int i=0; i<r_array.size(); i++) 
        if(!r_array[i].is_valid()) return false;
      return true;
    }

    // check whether range r is enclosed in this range 
    template <class Y>
    bool is_enclosed(const Range<Y>& r) const {
      assert(r_array.size() == r.r_array.size());
      for(unsigned int i=0; i<r_array.size(); i++)
        if(!(r_array[i] >= r.r_array[i])) return false;
      return true;
    }

    // check whether r is equal with this range
    template <class Y>
    bool is_same(const Range<Y>& r) const {
      assert(r_array.size() == r.r_array.size());
       for(unsigned int i=0; i<r_array.size(); i++)
        if(r_array[i] != r.r_array[i]) return false;
      return true;
    }

    // simple combine without check
    template <class Y>
    Range combine(const Range<Y>& r) const {
      assert(r_array.size() == r.r_array.size());
      Range rv(compressed);
      rv.r_array.resize(r_array.size());
      for(unsigned int i=0; i<r_array.size(); i++)
        rv.r_array[i] = r_array[i] | r.r_array[i];
      return rv;
    }

    // simple overlap without check
    template <class Y>
    Range overlap(const Range<Y>& r) const {
      assert(r_array.size() == r.r_array.size());
      Range rv(compressed);
      rv.r_array.resize(r_array.size());
      for(unsigned int i=0; i<r_array.size(); i++) 
        rv.r_array[i] = r_array[i] & r.r_array[i];
      return rv;
    }

    // simple reduce without check
    template <class Y>
    Range reduce(const Range<Y>& r) const {
      assert(r_array.size() == r.r_array.size());
      Range rv(compressed);
      rv.r_array.resize(r_array.size());
      for(unsigned int i=0; i<r_array.size(); i++) {
        if(r_array[i] != r.r_array[i])
          rv.r_array[i] = r_array[i] - r.r_array[i];
        else
          rv.r_array[i] = r_array[i];
      }

      // if same, means this and r are equal, the reduced one is none
      if(rv == *this) 
        return Range();
      else 
        return rv;
    }

    // normalize divide
    template <class Y>
    boost::tuple<Range, Range, Range> 
    divideBy(const Range<Y>& r) const {
      Range rH(compressed), rM(compressed), rL(compressed);
      rH.r_array.resize(r_array.size());
      rM.r_array.resize(r_array.size());
      rL.r_array.resize(r_array.size());
      for(unsigned int i=0; i<r_array.size(); i++) {
        if(r_array[i] != r.r_array[i])
          boost::tie(rH.r_array[i], rM.r_array[i], rL.r_array[i]) = 
            r_array[i].divideBy(r.r_array[i]);
        else {
          rH.r_array[i] = r_array[i];
          rM.r_array[i] = r_array[i];
          rL.r_array[i] = r_array[i];
        }
      }
      return boost::tuple<Range, Range, Range>(rH, rM, rL);
    }

    // weak order
    template <class Y>
    bool less(const Range<Y>& r) const {
      assert(r_array.size() == r.r_array.size());
      for(unsigned int i=0; i<r_array.size(); i++) {
        if(r_array[i] != r.r_array[i])
          return r_array[i].less(r.r_array[i]);
      }
      return false;
    }

    // check whether the ranges can be compared
    // indicating they have the same number of dimensions
    template <class Y>
    bool is_comparable(const Range<Y>& r) const {
      return r_array.size() == r.r_array.size();
    }
    
    // check whether the range and this range satify the operable condition:
    // only one dimension is not equal
    template <class Y>
    bool is_operable(const Range<Y>& r) const {
      if(r_array.size() != r.r_array.size()) return false;
      unsigned int cnt = 0;     // counting the different dimensions
      for(unsigned int i=0; i<r_array.size(); i++)
        if(r_array[i] != r.r_array[i]) cnt++;
      if(cnt > 1) return false;
      else return true;
    }

    // check whether the range and this range are adjacent
    template <class Y>
    bool is_adjacent(const Range<Y>& r) const {
      if(!is_operable(r)) return false;
      for(unsigned int i=0; i<r_array.size(); i++) {
        if(r_array[i] != r.r_array[i]) {
          if(r_array[i].is_adjacent(r.r_array[i]))
            return true;
          else
            return false;
        }
      }
      
      return true;
    }    

    // stream out function
    std::ostream& streamout(std::ostream& os) const{
      if(is_valid()) {
        for(unsigned int i=0; i<r_array.size(); i++)
          os << r_array[i];
      } else {
        os << "[]";
      }
      return os;
    }

  };

  /////////////////////////////////////////////
  // overload operators

  // rhs range is enclosed in lhs (not equal)
  template <class T, class Y>
  bool operator> (const Range<T>& lhs, const Range<Y>& rhs) {
    if(lhs.is_comparable(rhs))
      return lhs.is_enclosed(rhs) && !lhs.is_same(rhs);
    else 
      return false;
  }

  // rhs range is enclosed in lhs
  template <class T, class Y>
  bool operator>= (const Range<T>& lhs, const Range<Y>& rhs) {
    if(lhs.is_comparable(rhs))
      return lhs.is_enclosed(rhs);
    else 
      return false;
  }

  // lhs range is enclosed in rhs (not equal)
  template <class T, class Y>
  bool operator< (const Range<T>& lhs, const Range<Y>& rhs) {
    if(lhs.is_comparable(rhs)) 
      return rhs.is_enclosed(lhs) && !rhs.is_same(lhs);
    else 
      return false;
  }

  // lhs range is enclosed in rhs
  template <class T, class Y>
  inline bool operator<= (const Range<T>& lhs, const Range<Y>& rhs) {
    if(lhs.is_comparable(rhs)) 
      return rhs.is_enclosed(lhs);
    else 
      return false;
  }
  
  // two ranges are equal
  template <class T, class Y>  
  bool operator== (const Range<T>& lhs, const Range<Y>& rhs) {
    if(lhs.is_comparable(rhs)) 
      return rhs.is_same(lhs);
    else 
      return false;
  }

  // two ranges are not equal
  template <class T, class Y>  
  bool operator!= (const Range<T>& lhs, const Range<Y>& rhs) {
    return !(rhs == lhs);
  }

  // return the overlapped range
  template <class T, class Y>  
  Range<T> operator& (const Range<T>& lhs, const Range<Y>& rhs) {
    if(lhs.is_comparable(rhs)) 
      return lhs.overlap(rhs);
    else 
      return Range<T>();
  }

  // return the combined range
  template <class T, class Y>
  Range<T> operator| (const Range<T>& lhs, const Range<Y>& rhs) {
    if(lhs.is_adjacent(rhs))
      return lhs.combine(rhs);
    else
      return Range<T>();
  }

  // return the reduced range
  template <class T, class Y>
  Range<T> operator- (const Range<T>& lhs, const Range<Y>& rhs) {
    if(lhs.is_operable(rhs))
      return lhs.reduce(rhs);
    else
      return Range<T>();
  }

  // return the standard division (high, overlapped, low)
  template <class T, class Y>
  boost::tuple<Range<T>, Range<T>, Range<T> > 
  operator^ (const Range<T>& lhs, const Range<Y>& rhs) {
    if(lhs.is_adjacent(rhs))
      return lhs.divideBy(rhs);
    else
      return boost::tuple<Range<T>, Range<T>, Range<T> >();
  }

  // standard out stream
  template<class T>
  std::ostream& operator<< (std::ostream& os, const Range<T>& r) {
    return r.streamout(os);
  }

}

#endif
