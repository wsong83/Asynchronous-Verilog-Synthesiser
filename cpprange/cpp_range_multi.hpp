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
 * Simple multi-dimensional Range
 * 20/05/2014   Wei Song
 *
 *
 */

#ifndef _CPP_RANGE_MULTI_H_
#define _CPP_RANGE_MULTI_H_

#include <list>
#include <vector>
#include <utility>
#include <ostream>
#include <boost/tuple/tuple.hpp>
#include <boost/foreach.hpp>

#include "cpp_range_element.hpp"

namespace CppRange {

  ////////////////////////////////////////////////// 
  // Range
  // 
  // multi-dimensional range expression
  //
  //////////////////////////////////////////////////

  template <class T>
  class Range {
  private:
    std::vector<RangeElement<T> > r_array;              // the range array

  public:
    //////////////////////////////////////////////
    // constructors
    
    Range() {}
    explicit Range(const std::list<RangeElement<T> >&); // construct from a list of 
                                                        // RangeElements
    explicit Range(const std::vector<RangeElement<T> >&);
                                                        // construct from a vector of 
                                                        // RangeElements
    explicit Range(const std::list<std::pair<T,T> >&);  // construct from a list of
                                                        // raw range pairs
    explicit Range(const std::vector<std::pair<T,T> >&);
                                                        // construct from a vector of 
                                                        // raw range pairs
    Range(const std::string&);                          // build from parsing a range text

    //////////////////////////////////////////////
    // helpers

    // data accesser
    RangeElement<T>& operator[] (unsigned int);         // access a certain dimension
                                                        // start from the highest (left)
    const RangeElement<T>& operator[] (unsigned int) const;
    
    // get iterators
    typename std::vector<RangeElement<T> >::iterator 
    begin() { return r_array.begin(); }
    typename std::vector<RangeElement<T> >::const_iterator 
    begin() const { return r_array.begin(); }      
    typename std::vector<RangeElement<T> >::iterator 
    end() { return r_array.end(); }      
    typename std::vector<RangeElement<T> >::const_iterator 
    end() const { return r_array.end(); }      
    

    unsigned int dimension() const;             // the number of dimensions
    T size() const;                             // the size of the range
    void add_upper(const RangeElement<T>&);             // add a higher dimension, expensive
                                                        // do not use whenever possible
    void add_lower(const RangeElement<T>&);             // add a lower dimension
    void add_dimension(const RangeElement<T>&, unsigned int pos);
                                                        // add a dimension at position 'pos'
    void remove_upper();                                // remove the highest dimension
    void remove_lower();                                // remove the lowest dimension
    void remove_dimension(unsigned int pos);            // remove a dimension at position 'pos'
    bool valid() const;                                 // ? this is a valid range
    bool empty() const;                                 // ? this is an empty range 
    bool subset(const Range&) const;                    // ? this is a subset of r
    bool proper_subset( const Range&) const;            // ? this is a proper subset of r
    bool superset(const Range&) const;                  // ? this is a superset of r
    bool proper_superset( const Range&) const;          // ? this is a proper superset of r
    bool singleton() const;                             // ? this is a singleton range 
    bool equal(const Range& r) const;                   // ? this == r 
    bool connected(const Range& r) const;               // ? this and r are connected
    bool less(const Range& r) const;                    // weak order compare
    bool overlap(const Range& r) const;                 // this & r != []
    bool disjoint(const Range& r) const;                // this & r == []
    Range combine(const Range& r) const;                // get the union of this and r
    Range hull(const Range& r) const;                   // get the minimal superset of the union
                                                        // of this and r
    Range intersection(const Range& r) const;           // get the intersection of this and r
    Range complement(const Range& r) const;             // subtract r from this range
    boost::tuple<Range, Range, Range>
    divide(const Range& r) const;                       // standard divide/partition this and r
    
    std::ostream& streamout(std::ostream& os) const;    // stream out the range
    std::string toString() const;                       // simple conversion to string 

  private:
    bool comparable(const Range& r) const;              // ? this and r can be compared 

  protected:
    bool operable(const Range& r) const;                // ? this and r are operable
                                                        // only one dimension is not equal
  };

  /////////////////////////////////////////////
  // implementation of class methods

  // constructors

  // construct from a list of RangeElements
  template<class T> inline
  Range<T>::Range(const std::list<RangeElement<T> >& l)
    : r_array(std::vector<RangeElement<T> >(l.begin(), l.end())) {} 

  // construct from a vector of RangeElements
  template<class T> inline
  Range<T>::Range(const std::vector<RangeElement<T> >& l)
    : r_array(l) {}

  // construct from a list of raw range pairs
  template<class T> inline
  Range<T>::Range(const std::list<std::pair<T,T> >& l) {
    r_array.resize(l.size());
    unsigned int i = 0;
    typedef std::pair<T,T> local_range_pair;
    BOOST_FOREACH(const local_range_pair& r, l)
      r_array[i++]=RangeElement<T>(r.first, r.second);
  }

  // construct from a vector of raw range pairs
  template<class T> inline
  Range<T>::Range(const std::vector<std::pair<T,T> >& l) {
    r_array.resize(l.size());
    unsigned int i = 0;
    typedef std::pair<T,T> local_range_pair;
    BOOST_FOREACH(const local_range_pair& r, l)
      r_array[i++]=RangeElement<T>(r.first, r.second);   
  }

  // parse a range text
  template<class T> inline
  Range<T>::Range(const std::string& str) {
    *this = Range(parse_range_list<T>(str));
  }

  //////////////////////////////////////////////
  // data accesser
  template<class T> inline
  RangeElement<T>& Range<T>::operator[] (unsigned int index) {
    return r_array.at(index);
  }
  
  template<class T> inline
  const RangeElement<T>& Range<T>::operator[] (unsigned int index) const {
    return r_array.at(index);
  }
  
  //////////////////////////////////////////////
  // Helpers
  
  // size of dimension
  template<class T> inline
  unsigned int Range<T>::dimension() const {
    return r_array.size();
  }

  // size of bits
  template<class T> inline
  T Range<T>::size() const {
    if(empty()) return T(0);
    
    T rv(1);
    for(unsigned int i=0; i<r_array.size(); i++)
      rv = rv * r_array[i].size();
    return rv;
  }
  
  // add a higher dimension
  // this method is very expernsive, do not use if possible
  template<class T> inline
  void Range<T>::add_upper(const RangeElement<T>& r) {
    r_array.insert(r_array.begin(), r);
  }

  // add a lower dimension
  template<class T> inline
  void Range<T>::add_lower(const RangeElement<T>& r) {
    r_array.push_back(r);
  }

  // add a dimension
  // this method is very expernsive, do not use if possible
  template<class T> inline
  void Range<T>::add_dimension(const RangeElement<T>& r, unsigned int pos) {
    // ** need to check pos < size ?
    r_array.insert(r_array.begin() + pos, r);
  }
   
  // remove the highest dimension
  // this method is very expernsive, do not use if possible
  template<class T> inline
  void Range<T>::remove_upper() {
    r_array.erase(r_array.begin());
  }

  // remove the lowest dimension
  template<class T> inline
  void Range<T>::remove_lower() {
    r_array.pop_back();
  }

  // remove a dimension
  // this method is very expernsive, do not use if possible
  template<class T> inline
  void Range<T>::remove_dimension(unsigned int pos) {
    // ** need to check pos < size ?
    r_array.erase(r_array.begin() + pos);
  }

  // check whether the range expression is valid
  template<class T> inline
  bool Range<T>::valid() const {
    for(unsigned int i=0; i<r_array.size(); i++) 
      if(!r_array[i].valid()) return false;
    return true;
  }

  // check whether the range expression is empty
  template<class T> inline
  bool Range<T>::empty() const {
    if(!valid()) return true;
    if(r_array.empty()) return true;
    for(unsigned int i=0; i<r_array.size(); i++) 
      if(r_array[i].empty()) return true;
    return false;
  }

  // check whether this is a subset of r
  template<class T> inline
  bool Range<T>::subset(const Range& r) const {
    if(!valid() || !r.valid()) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_InvalidRange());
#endif
      return false;
    }
    if(empty()) return true;
    if(r.empty()) return false;
    if(!comparable(r)) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_NonComparable(toString(), r.toString(), "subset()"));
#endif
      return false; // or throw an exception
    }
    for(unsigned int i=0; i<r_array.size(); i++) {
      if(!r_array[i].subset(r[i])) 
        return false;
    }
    return true;
  }

  // check whether this is a proper subset of r
  template<class T> inline
  bool Range<T>::proper_subset(const Range& r) const {
    if(!valid() || !r.valid()) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_InvalidRange());
#endif
      return false;
    }
    if(empty()) return !r.empty();
    if(r.empty()) return false;
    if(!comparable(r)) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_NonComparable(toString(), r.toString(), "proper_subset()"));
#endif
      return false; // or throw an exception
    }
    
    bool proper = false;
    for(unsigned int i=0; i<r_array.size(); i++) {
      if(r_array[i].equal(r[i])) 
        continue;
      else if(r_array[i].proper_subset(r[i]))
        proper = true;
      else
        return false;
    }
    return proper;
  }

  // check whether this is a superset of r
  template<class T> inline
  bool Range<T>::superset(const Range& r) const {
    if(!valid() || !r.valid()) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_InvalidRange());
#endif
      return false;
    }
    if(r.empty()) return true;
    if(empty()) return false;
    if(!comparable(r)) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_NonComparable(toString(), r.toString(), "superset()"));
#endif
      return false; // or throw an exception
    }

    for(unsigned int i=0; i<r_array.size(); i++) {
      if(!r_array[i].superset(r[i])) 
        return false;
    }
    return true;
  }

  // check whether this is a proper superset of r
  template<class T> inline
  bool Range<T>::proper_superset(const Range& r) const {
    if(!valid() || !r.valid()) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_InvalidRange());
#endif
      return false;
    }
    if(r.empty()) return !empty();
    if(empty()) return false;
    if(!comparable(r)) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_NonComparable(toString(), r.toString(), "proper_superset()"));
#endif
      return false; // or throw an exception
    }

    bool proper = false;
    for(unsigned int i=0; i<r_array.size(); i++) {
      if(r_array[i].equal(r[i])) 
        continue;
      else if(r_array[i].proper_superset(r[i]))
        proper = true;
      else
        return false;
    }
    return proper;
  }

  // check whether this range is a singleton range
  template<class T> inline
  bool Range<T>::singleton() const {
    if(!valid()) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_InvalidRange());
#endif
      return false;
    }
    return empty() || size() == min_unit<T>();
  }  
  
  // check whether r is equal with this range
  template<class T> inline
  bool Range<T>::equal(const Range& r) const {
    if(!valid() || !r.valid()) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_InvalidRange());
#endif
      return false;
    }
    if(empty()) return r.empty();
    if(r.empty()) return false;
    if(!comparable(r)) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_NonComparable(toString(), r.toString(), "=="));
#endif
      return false; // or throw an exception
    }

    for(unsigned int i=0; i<r_array.size(); i++) {
      if(!r_array[i].equal(r.r_array[i]))
        return false;
    }
    return true;
  }

  // check whether the range and this range are adjacent
  template<class T> inline
  bool Range<T>::connected(const Range& r) const {
    if(!valid() || !r.valid()) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_InvalidRange());
#endif
      return false;
    }
    if(empty() || r.empty()) return false;
    if(!comparable(r)) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_NonComparable(toString(), r.toString(), "connected()"));
#endif
      return false; // or throw an exception
    }
    
    for(unsigned int i=0; i<r_array.size(); i++) {
      if(!r_array[i].equal(r.r_array[i])) {
        if(!r_array[i].connected(r.r_array[i]))
          return false;
      }
    }
    return true;
  }    

  // weak order
  template<class T> inline
  bool Range<T>::less(const Range& r) const {
    if(!valid() || !r.valid()) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_InvalidRange());
#endif
      return false;
    }
    if(empty()) return !r.empty();
    if(r.empty()) return false;
    if(!comparable(r)) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_NonComparable(toString(), r.toString(), "<"));
#endif
      return false; // or throw an exception
    }
    
    for(unsigned int i=0; i<r_array.size(); i++) {
      if(!r_array[i].equal(r.r_array[i]))
        return r_array[i].less(r.r_array[i]);
    }
    return false;               // the two ranges are equal
  }

  // whether this and r has non-empty intersection
  template<class T> inline
  bool Range<T>::overlap(const Range& r) const {
    if(!valid() || !r.valid()) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_InvalidRange());
#endif
      return false;
    }
    if(empty() || r.empty())  return false;
    if(!comparable(r)) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_NonComparable(toString(), r.toString(), "overlap()"));
#endif
      return false; // or throw an exception
    }

    for(unsigned int i=0; i<r_array.size(); i++) {
      if(!r_array[i].overlap(r.r_array[i]))
        return false;
    }
    return true;
  }

  // whether this and r have no shared range
  template<class T> inline
  bool Range<T>::disjoint(const Range& r) const {
    if(!valid() || !r.valid()) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_InvalidRange());
#endif
      return false;
    }
    if(empty() || r.empty())  return true;
    if(!comparable(r)) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_NonComparable(toString(), r.toString(), "disjoint()"));
#endif
      return false; // or throw an exception
    }

    for(unsigned int i=0; i<r_array.size(); i++) {
      if(!r_array[i].overlap(r.r_array[i]))
        return true;
    }
    return false;
  }

  // combine two ranges
  template<class T> inline
  Range<T> Range<T>::combine(const Range& r) const {
    if(!valid() || !r.valid()) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_InvalidRange());
#endif
      return Range();
    }
    if(empty()) return r;
    if(r.empty()) return *this;
    if(!comparable(r)) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_NonComparable(toString(), r.toString(), "|"));
#endif
      return Range(); // or throw an exception
    }
    if(!operable(r)) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_NonOperable(toString(), r.toString(), "|"));
#endif
      return Range(); // or throw an exception
    }

    Range rv;
    rv.r_array.resize(r_array.size());
    for(unsigned int i=0; i<r_array.size(); i++)
      rv.r_array[i] = r_array[i].combine(r.r_array[i]);
    return rv;
  }

  // get the minimal range contain the two ranges
  template<class T> inline
  Range<T> Range<T>::hull(const Range& r) const {
    if(!valid() || !r.valid()) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_InvalidRange());
#endif
      return Range();
    }
    if(empty()) return r;
    if(r.empty()) return *this;
    if(!comparable(r)) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_NonComparable(toString(), r.toString(), "hull()"));
#endif
      return Range(); // or throw an exception
    }

    Range rv;
    rv.r_array.resize(r_array.size());
    for(unsigned int i=0; i<r_array.size(); i++)
      rv.r_array[i] = r_array[i].hull(r.r_array[i]);
    return rv;
  }

  // get the shared range
  template<class T> inline
  Range<T> Range<T>::intersection(const Range& r) const {
    if(!valid() || !r.valid()) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_InvalidRange());
#endif
      return Range();
    }
    if(empty() || r.empty()) return Range();
    if(!comparable(r)) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_NonComparable(toString(), r.toString(), "&"));
#endif
      return Range(); // or throw an exception
    }

    Range rv;
    rv.r_array.resize(r_array.size());
    for(unsigned int i=0; i<r_array.size(); i++)
      rv.r_array[i] = r_array[i].intersection(r.r_array[i]);
    return rv;
  }

  //subtraction
  template<class T> inline
  Range<T> Range<T>::complement(const Range& r) const {
    if(!valid() || !r.valid()) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_InvalidRange());
#endif
      return Range();
    }
    if(empty()) return Range();
    if(r.empty()) return *this;
    if(!comparable(r)) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_NonComparable(toString(), r.toString(), "complement()"));
#endif
      return Range(); // or throw an exception
    }
    if(!operable(r)) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_NonOperable(toString(), r.toString(), "complement()"));
#endif
      return Range(); // or throw an exception
    }

    Range rv(r);
    rv.r_array.resize(r_array.size());
    for(unsigned int i=0; i<r_array.size(); i++) {
      if(!r_array[i].equal(r.r_array[i])) {
        rv.r_array[i] = r_array[i].complement(r.r_array[i]);
        return rv;
      }
    }
    
    return Range();          // the two ranges are equal
  }

  //subtraction
  template<class T> inline
  boost::tuple<Range<T>, Range<T>, Range<T> >
  Range<T>::divide(const Range& r) const {
    boost::tuple<Range, Range, Range > rv;
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
    
    if(!comparable(r)) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_NonComparable(toString(), r.toString(), "divide()"));
#endif
      return rv; // or throw an exception
    }
    if(!operable(r)) {
#ifndef CPP_RANGE_NO_EXCEPTION
      throw(RangeException_NonOperable(toString(), r.toString(), "divide()"));
#endif
      return rv; // or throw an exception
    }

    boost::get<0>(rv) = r;
    boost::get<1>(rv) = r;
    boost::get<2>(rv) = r;
    
    for(unsigned int i=0; i<r_array.size(); i++) {
      if(!r_array[i].equal(r.r_array[i])) {
        RangeElement<T> rH, rM, rL;
        boost::tie(rH, rM, rL) = r_array[i].divide(r.r_array[i]);
        boost::get<0>(rv).r_array[i] = rH;
        if(rM.empty())
          boost::get<1>(rv) = Range();
        else
          boost::get<1>(rv).r_array[i] = rM;
        boost::get<2>(rv).r_array[i] = rL;
       return rv;
      }
    }

    // the two ranges are equal
    boost::get<0>(rv) = Range();
    boost::get<1>(rv) = r;
    boost::get<2>(rv) = Range(); 
    return rv;
  }

  // stream out the range
  template<class T> inline
  std::ostream& Range<T>::streamout(std::ostream& os) const{
    if(empty()) os << "[]";
    else {
      for(unsigned int i=0; i<r_array.size(); i++)
        os << r_array[i];
    }
    return os;
  }

  // convert to string
  template<class T> inline
  std::string Range<T>::toString() const {
    std::string rv;
    if(empty()) rv = "[]";
    else {
      for(unsigned int i=0; i<r_array.size(); i++)
        rv += r_array[i].toString();
    }
    return rv;
  }

  //////////////////////////////////////////////
  // Protected Helpers

  // check whether the ranges can be compared
  // indicating they have the same number of dimensions
  template<class T> inline
  bool Range<T>::comparable(const Range& r) const {
    return r_array.size() == r.r_array.size();
  }
    
  // check whether the range and this range satify the operable condition:
  // no more than one dimension is not equal
  template<class T> inline
  bool Range<T>::operable(const Range& r) const {
    if(!comparable(r)) return false;
    
    bool diff = false;
    for(unsigned int i=0; i<r_array.size(); i++) {
      if(!r_array[i].equal(r.r_array[i])) {
        if(diff) return false;  // more than one dimensions are different
        else diff = true;
      }
    }
    return true;
  };

  /////////////////////////////////////////////
  // overload operators

  // rhs range is less than lhs
  template <class T>
  bool operator> (const Range<T>& lhs, const Range<T>& rhs) {
    return rhs.less(lhs);
  }

  // rhs range is less than or equal to lhs
  template <class T>
  bool operator>= (const Range<T>& lhs, const Range<T>& rhs) {
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
  bool operator< (const Range<T>& lhs, const Range<T>& rhs) {
    return lhs.less(rhs);
  }

  // lhs range is larger than or equal to rhs
  template <class T>
  bool operator<= (const Range<T>& lhs, const Range<T>& rhs) {
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
  inline bool operator== (const Range<T>& lhs, const Range<T>& rhs) {
    return lhs.equal(rhs);
  }

  // two ranges are not equal
  template <class T>
  inline bool operator!= (const Range<T>& lhs, const Range<T>& rhs) {
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
  Range<T> operator& (const Range<T>& lhs, const Range<T>& rhs) {
    return lhs.intersection(rhs);
  }

  // return the combined range
  // function does not check the result's validation
  template <class T>  
  Range<T> operator| (const Range<T>& lhs, const Range<T>& rhs) {
    return lhs.combine(rhs);
  }

  // standard out stream
  template<class T>
  std::ostream& operator<< (std::ostream& os, const Range<T>& r) {
    return r.streamout(os);
  }

}

#endif
