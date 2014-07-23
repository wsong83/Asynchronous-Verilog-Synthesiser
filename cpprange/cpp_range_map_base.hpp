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
 * 20/05/2014   Wei Song
 *
 * Note: this class is not suppose to be used by user directly
 *
 */

#ifndef _CPP_RANGE_MAP_BASE_H_
#define _CPP_RANGE_MAP_BASE_H_

#include <list>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

#include "cpp_range_multi.hpp"

namespace CppRange {

  // forward definition
  template<class T> class RangeMap;


  //////////////////////////////////////////////////
  // RangeMapBase
  //
  // store the current range and its child ranges of RangeMap
  //
  //////////////////////////////////////////////////
  template <class T>
  class RangeMapBase : public RangeElement<T> {
  private:
    std::list<RangeMapBase> child;    // sub-dimensions
    unsigned int level;               // level of sub-ranges
  public:

    // allow RangeMap to use protected member functions
    friend class RangeMap<T>;

    //////////////////////////////////////////////
    // constructors

    RangeMapBase();
    RangeMapBase(const T&);                             // a single bit range 
    RangeMapBase(const T&, const T&);                   // single level range
    explicit RangeMapBase(const RangeElement<T>&);      // range element to range map base
    explicit RangeMapBase(const Range<T>&);             // multidimensional range to range map base
    explicit RangeMapBase(const RangeElement<T>&, const std::list<RangeMapBase>&);
                                                        // multidimensional range map base
    template<class InputIterator>
    RangeMapBase(const RangeElement<T>&, InputIterator first, InputIterator last);
                                                        // build a multidimensional range map 
                                                        // using iterators

    //////////////////////////////////////////////
    // Helpers

    //using RangeElement<T>::upper;
    //using RangeElement<T>::lower;

    unsigned int dimension() const;                     // the number of dimensions
    T size() const;                                     // the size of the range
                                                        // ** check: validation beforehand
                                                        // ** the same for all followings
    bool valid() const;                                 // ? this is a valid range
    bool empty() const;                                 // ? this is an empty range 
    bool equal(const RangeMapBase& r) const;            // ? this == r 
    boost::tuple<RangeMapBase, RangeMapBase, RangeMapBase> 
    combine(const RangeMapBase& r) const;               // get the union of this and r
    RangeMapBase intersection(const RangeMapBase& r) const;
                                                        // get the intersection of this and r
    
    std::list<Range<T> > toRange() const;               // convert a RangeMap to Ranges
    std::string toString(bool compress = true) const;   // simple conversion to string 
  protected:

    void set_child(const std::list<RangeMapBase>&);     // set a new child range list 
    bool add_child(const RangeMapBase&);                // insert a sub-range to the child list
    
    //////////////////////////////////
    // static helper functions

    static T size(const std::list<RangeMapBase>&);      // calculate the bit size of a range list
    static bool empty(const std::list<RangeMapBase>&);  // ? a range list is empty
    static bool valid(const std::list<RangeMapBase>&, unsigned int level);  
                                                        // ? a range list is valid
    static bool subset(const std::list<RangeMapBase>&, const std::list<RangeMapBase>&);
                                                        // subset relation of two range lists
    static bool equal(const std::list<RangeMapBase>&, const std::list<RangeMapBase>&);
                                                        // ? two range lists are equal
    static std::list<RangeMapBase> 
    combine(const std::list<RangeMapBase>&, const std::list<RangeMapBase>&);
                                                        // combine two range lists
    static std::list<RangeMapBase>
    intersection(const std::list<RangeMapBase>&, const std::list<RangeMapBase>&);
                                                        // get the intersection of two range lists
    static std::list<RangeMapBase>
    complement(const std::list<RangeMapBase>&, const std::list<RangeMapBase>&);
                                                        // get the result of list l - list r
    static void normalize(std::list<RangeMapBase>&);    // normalize a range list
    static void add_child(std::list<RangeMapBase>&, const RangeMapBase&);
                                                        // add a Range into a list of ranges
    static std::list<Range<T> > toRange(const std::list<RangeMapBase>&);
                                                        // convert a RangeMap to Ranges
    static std::string toString(const std::list<RangeMapBase>&, bool compress = true);
                                                        // simple conversion to string 

  private:
    // Disable some derived member functions

    // connected() is too difficult and with no explicit usage in RangeMap
    void connected();
    
    // divide() and hull() are no longer useful because the combine() is now capable of handling
    // complex ranges
    void divide();
    void hull();

    // kind impossible to implement a strict less
    void less();

    // not needed in a not user friendly class
    void subset();
    void proper_subset();
    void superset();
    void proper_superset();
    void singleton();
    void overlap();
    void disjoint();
    void complement();
  };

  /////////////////////////////////////////////
  // implementation of class methods

  // constructors
  // default to construct an range with undefined value
  template<class T> inline
  RangeMapBase<T>::RangeMapBase()
    : RangeElement<T>(), level(0) {}

  // single bit range
  template<class T> inline
  RangeMapBase<T>::RangeMapBase(const T& r)
    : RangeElement<T>(r), level(1) {}

  // bit range
  template<class T> inline
  RangeMapBase<T>::RangeMapBase(const T& rh, const T& rl)
    : RangeElement<T>(rh, rl), level(1) {}

  // type conversion
  template<class T> inline
  RangeMapBase<T>::RangeMapBase(const RangeElement<T>& r)
    : RangeElement<T>(r), level(1) {}

  // type conversion
  template<class T> inline
  RangeMapBase<T>::RangeMapBase(const Range<T>& r) 
    :level(0) {
    typename std::vector<RangeElement<T> >::const_iterator it = r.begin();
    if(it != r.end()) {
      RangeElement<T> base_range = *it++;
      *this = RangeMapBase(base_range, it, r.end());
    }
  }

  // combined build
  template<class T> inline
  RangeMapBase<T>::RangeMapBase(const RangeElement<T>& r, const std::list<RangeMapBase>& rlist)
    : RangeElement<T>(r), child(rlist) {
    if(child.empty()) level = 1;
    else              level = child.front().level + 1;
  }

  // build a multidimensional range map using iterators
  template<class T> template<class InputIterator> inline
  RangeMapBase<T>::RangeMapBase(const RangeElement<T>& r, InputIterator first, InputIterator last)
    : RangeElement<T>(r) {
    if(first != last) {
      RangeElement<T> base_range = *first++;
      child.push_back(RangeMapBase(base_range, first, last));
      level = child.front().level + 1;
    } else {
      level = 1;
    }
  }

  //////////////////////////////////////////////
  // Helpers

  // get the number of dimensions
  template<class T> inline
  unsigned int RangeMapBase<T>::dimension() const {
    return level;
  }
    
  // get the size of this range
  template<class T> inline
  T RangeMapBase<T>::size() const {
    if(child.empty()) return RangeElement<T>::size(); // leaf node
    return size(child) * RangeElement<T>::size();
  }
  
  // whether the range is valid
  template<class T> inline
  bool RangeMapBase<T>::valid() const {
    return RangeElement<T>::valid() && valid(child, level - 1);
  }
  
  // whether the range is empty
  template<class T> inline
  bool RangeMapBase<T>::empty() const {
    return RangeElement<T>::empty() || empty(child) || (child.empty() && level > 1);
  }

  // check whether range r is equal with this range
  template<class T> inline
  bool RangeMapBase<T>::equal(const RangeMapBase& r) const {
    return RangeElement<T>::equal(r) && equal(child, r.child);
  }
  
  // combine two ranges
  template<class T> inline
  boost::tuple<RangeMapBase<T>, RangeMapBase<T>, RangeMapBase<T> > 
  RangeMapBase<T>::combine(const RangeMapBase& r) const {
    boost::tuple<RangeMapBase, RangeMapBase, RangeMapBase> rv;
    
    RangeElement<T> RAnd = RangeElement<T>::intersection(r);
    if(!RAnd.empty()) {
      // get the standard division
      RangeElement<T> rH, rM, rL;
      boost::tie(rH, rM, rL) = RangeElement<T>::divide(r);
      boost::get<0>(rv) = RangeMapBase<T>(rH);
      boost::get<1>(rv) = RangeMapBase<T>(rM);
      boost::get<2>(rv) = RangeMapBase<T>(rL);
        
      // get the higher part
      if(!rH.empty()) {
        if(rH.subset(*this)) // rH belongs to this range
          boost::get<0>(rv).set_child(child);
        else
          boost::get<0>(rv).set_child(r.child);
      }

      // the overlapped part
      boost::get<1>(rv).set_child(combine(child, r.child));

      // get the lower part
      if(!rL.empty()) {
        if(rL.subset(*this)) // rL belongs to this
          boost::get<2>(rv).set_child(child);
        else
          boost::get<2>(rv).set_child(r.child);
      }
    } else {                  // two inadjacent ranges
      if(RangeElement<T>::less(r)) {
        boost::get<0>(rv) = r;
        boost::get<2>(rv) = *this;
      } else {
        boost::get<0>(rv) = *this;
        boost::get<2>(rv) = r;
      }
    }
    return rv;
  }

  // get the intersection of two ranges
  template<class T> inline
  RangeMapBase<T> RangeMapBase<T>::intersection(const RangeMapBase& r) const {
    RangeMapBase rv(RangeElement<T>::intersection(r), intersection(child, r.child));
    if(rv.level != level) return RangeMapBase();
    else return rv;
  }
  
  // convert to a list of ranges
  template<class T> inline
  std::list<Range<T> > RangeMapBase<T>::toRange() const {
    std::list<Range<T> > rv;
    if(child.size()) {
      rv = toRange(child);
      BOOST_FOREACH(Range<T>& r, rv) 
        r.add_upper(*this);
    } else 
      rv.push_back(Range<T>(static_cast<RangeElement<T> >(*this)));
    return rv;
  }

  // convert to string
  template<class T> inline
  std::string RangeMapBase<T>::toString(bool compress) const{
    return RangeElement<T>::toString() + toString(child, compress);
  }

  //////////////////////////////////
  // protected helper functions

  template<class T> inline
  void RangeMapBase<T>::set_child(const std::list<RangeMapBase>& c) {
    child = c;
    if(c.empty())
      level = 1;
    else
      level = 1 + c.front().level;      
  }

  // insert a sub-range
  template<class T> inline
  bool RangeMapBase<T>::add_child(const RangeMapBase& r) {
    if(r.empty()) return false;
    
    if(child.empty())
      level = r.level + 1;
    else if(level != r.level + 1)
      return false;

    add_child(child, r);
    return true;
  }

  //////////////////////////////////
  // static helper functions

  // claculat the bit size
  template<class T> inline
  T RangeMapBase<T>::size(const std::list<RangeMapBase>& rlist) {
    // will not check the validation, the non-static method should check it
    T rv(0);
    BOOST_FOREACH(const RangeMapBase& b, rlist)
      rv = rv + b.size();  // disable check
    return rv;
  }

  // valid range expression
  template<class T> inline
  bool RangeMapBase<T>::valid(const std::list<RangeMapBase>& rlist, unsigned int l) {
    BOOST_FOREACH(const RangeMapBase& b, rlist)
      if(!b.valid() || l != b.level) {
        return false;
      }
    return true;
  }

  // check whether the child list is empty
  template<class T> inline
  bool RangeMapBase<T>::empty(const std::list<RangeMapBase>& rlist) {
    // will not check the validation, the non-static method should check it
    if(rlist.empty()) return false; // empty child means it is a leaf range
                                    // the emptiness is then depends on the base range
    BOOST_FOREACH(const RangeMapBase& b, rlist)
      if(!b.empty()) return false;  // any non-empty sub-range means non-empty

    return true;                    // all sub-ranges are empty, should be normalized!
  }

  // check whether 'lhs' is a subset of 'rhs'
  template<class T>
  bool RangeMapBase<T>::subset(const std::list<RangeMapBase>& lhs_arg, 
                               const std::list<RangeMapBase>& rhs_arg
                               ) {
    
    std::list<RangeMapBase> lhs = lhs_arg;
    std::list<RangeMapBase> rhs = rhs_arg;

    typename std::list<RangeMapBase<T> >::iterator lit, rit;
    for(lit = lhs.begin(), rit = rhs.begin();
        lit != lhs.end() && rit != rhs.end();
        ) {
      
      if(!lit->RangeElement<T>::intersection(*rit).empty()) {
        
        // using the standard divide function
        RangeElement<T> rH, rM, rL;
        boost::tie(rH, rM, rL) = lit->RangeElement<T>::divide(*rit);
          
        // check the higher part
        if(!rH.empty() && rH.subset(*lit)) // rH belongs to lit
          return false;
        
        // the overlapped part
        if(!subset(lit->child, rit->child))
          return false;
        
        // check the lower part
        if(!rL.empty()) {
          if(rL.subset(*rit)) {
            ++lit;
            *rit = RangeMapBase(rL, rit->child);
          } else {
            ++rit;
            *lit = RangeMapBase(rL, lit->child);
          }
        } else {
          ++lit;
          ++rit;
        }
      } else {
        if(rit->RangeElement<T>::less(*lit)) // some part belongs to lit but not rit
          return false;
        else
          ++rit;
      }
    }

    if(lit != lhs.end())
      return false;
    else
      return true;
  }

  // check whether two range lists are equal
  template<class T> inline
  bool RangeMapBase<T>::equal(const std::list<RangeMapBase>& lhs_arg, 
                              const std::list<RangeMapBase>& rhs_arg
                              ) {
    typename std::list<RangeMapBase>::const_iterator lit, rit;
    for(lit = lhs_arg.begin(), rit = rhs_arg.begin();
        lit != lhs_arg.end() && rit != rhs_arg.end();
        ++lit, ++rit) 
      if(!lit->equal(*rit)) return false;
       
    if(lit != lhs_arg.end() || rit != rhs_arg.end())
      return false;
    else
      return true;
  }

  // combine two child lists
  template<class T> inline
  std::list<RangeMapBase<T> > 
  RangeMapBase<T>::combine (const std::list<RangeMapBase>& lhs_arg, 
                            const std::list<RangeMapBase>& rhs_arg
                            ) {
    std::list<RangeMapBase> lhs = lhs_arg;
    std::list<RangeMapBase> rhs = rhs_arg;
    std::list<RangeMapBase> rv;
      
    typename std::list<RangeMapBase<T> >::iterator lit, rit;
    for(lit = lhs.begin(), rit = rhs.begin();
        lit != lhs.end() && rit != rhs.end();
        ) {
      // using the standard combine function
      RangeMapBase rH, rM, rL;
      boost::tie(rH, rM, rL) = lit->combine(*rit);
       
      // check result
      if(!rH.empty()) rv.push_back(rH);
      if(!rM.empty()) {
        // the two ranges are overlapped
        rv.push_back(rM);
        
        if(!rL.empty()) {
          if(rL.RangeElement<T>::subset(*lit)) {
            // the lower part belongs to lit, rit proceeds and lit recounts
            ++rit; *lit = rL;
          } else {
            // otherwise lit proceeds and rit recounts
            ++lit; *rit = rL;
          }
        } else {
          // both range list proceed
          ++rit; ++lit;
        }
      } else {
        // the two ranges are disjunctive
        if(rL.RangeElement<T>::subset(*lit))  ++rit;
        else                 ++lit;
      }
    }
    
    // push the rest
    if(lit != lhs.end()) rv.splice(rv.end(), lhs, lit, lhs.end());
    if(rit != rhs.end()) rv.splice(rv.end(), rhs, rit, rhs.end());
    
    normalize(rv);
    return rv;
  }


  // get the intersection of two ranges
  template<class T> inline
  std::list<RangeMapBase<T> >
  RangeMapBase<T>::intersection(const std::list<RangeMapBase>& lhs_arg, 
                                const std::list<RangeMapBase>& rhs_arg) {
    std::list<RangeMapBase> rv;
    
    BOOST_FOREACH(const RangeMapBase<T>& cl, lhs_arg) {
      BOOST_FOREACH(const RangeMapBase<T>& cr, rhs_arg) {
        RangeMapBase<T> result(cl.intersection(cr));
        if(!result.empty()) rv.push_back(result);
      }
    }
    normalize(rv);
    return rv;
  }

  // reduce the content of a range list (rhs) from another (lhs)
  template<class T> inline
  std::list<RangeMapBase<T> >
  RangeMapBase<T>::complement(const std::list<RangeMapBase>& lhs_arg, 
                              const std::list<RangeMapBase>& rhs_arg) {
      
    std::list<RangeMapBase> lhs = lhs_arg;
    std::list<RangeMapBase> rhs = rhs_arg;
    std::list<RangeMapBase> rv;
    
    typename std::list<RangeMapBase<T> >::iterator lit, rit;
    for(lit = lhs.begin(), rit = rhs.begin();
        lit != lhs.end() && rit != rhs.end();
        ) {
      // using the standard combine function
      RangeElement<T> rH, rM, rL;
      boost::tie(rH, rM, rL) = lit->RangeElement<T>::divide(*rit);
      
      // check result
      if(!rH.empty() && rH.subset(*lit)) {
        RangeMapBase mH(rH, lit->child);          
        rv.push_back(mH);
      }

      if(!rM.empty()) {
        // the two ranges are overlapped
        if(!lit->child.empty()) {
          RangeMapBase mM(rM, complement(lit->child, rit->child));
          if(lit->level == mM.level) rv.push_back(mM); // avoid empty range
        }
        
        if(!rL.empty()) {
          if(rL.subset(*lit)) {
            // the lower part belongs to lit, rit proceeds and lit recounts
            ++rit; *lit = RangeMapBase<T>(rL, lit->child);
          } else {
            // otherwise lit proceeds and rit recounts
            ++lit; *rit = RangeMapBase<T>(rL, rit->child);
          }
        } else {
          // both range list proceed
          ++rit; ++lit;
        }
      } else {
        // the two ranges are disjunctive
        if(rL.subset(*lit))  ++rit;
        else                 ++lit;
      }
    }
    
    // push the rest
    if(lit != lhs.end()) rv.splice(rv.end(), lhs, lit, lhs.end());

    normalize(rv);
    return rv;
  }
  
  // normalize the range list
  // remove empty range and combine sub-ranges with equal sub trees
  template<class T> inline
  void RangeMapBase<T>::normalize (std::list<RangeMapBase>& rlist) {
    if(rlist.empty()) return;

    // it is assumed that all sub-ranges are not overlapped and in weak order
    typename std::list<RangeMapBase>::iterator it, nt;
    nt = rlist.begin();
    it = nt++;

    while(nt != rlist.end()) {
      if(it->empty()) {         // remove empty it range
        rlist.erase(it);
        it = nt++;
      } else if(nt->empty()) {
        nt = rlist.erase(nt);   // remove empty nt range
      } else if(it->RangeElement<T>::connected(*nt) && equal(it->child, nt->child)) {
        it->lower(nt->lower()); // combine nt and it as they have the same sub-tree
        nt = rlist.erase(nt);
      } else {
        it = nt++;
      }
    }
    
    if(it->empty()) rlist.erase(it);
  }

  template<class T> inline
  void RangeMapBase<T>::add_child(std::list<RangeMapBase>& rlist, const RangeMapBase& r) {
    std::list<RangeMapBase> rv;
    RangeMapBase mr = r;
    
    typename std::list<RangeMapBase<T> >::iterator lit;
    for(lit = rlist.begin(); lit != rlist.end(); ) {
      // using the standard combine function
      RangeMapBase rH, rM, rL;
      boost::tie(rH, rM, rL) = lit->combine(mr);
      
      // check result
      if(!rH.empty()) rlist.insert(lit, rH);
      if(!rM.empty()) {
        // the two ranges are overlapped
        rlist.insert(lit, rM);
        
        if(!rL.empty()) {
          if(rL.RangeElement<T>::subset(*lit)) {
            // the lower part belongs to lit, insertion finished
            mr = RangeMapBase();
            *lit = rL;
            break;
          } else {
            // otherwise lit proceeds and mr recounts
            lit = rlist.erase(lit);
            mr = rL;
          }
        } else {
          // rL empty also means finished
          mr = RangeMapBase();
          lit = rlist.erase(lit);
          break;
        }
      } else {
        // the two ranges are disjunctive
        if(rL.RangeElement<T>::subset(*lit)) {
          // mr already inserted
          mr = RangeMapBase();
          break;
        } else
          ++lit;
      }
    }
    
    // push the rest
    if(!mr.empty()) rlist.insert(lit, mr);
    
    normalize(rlist);
  }

  // convert to a list of ranges
  template<class T> inline
  std::list<Range<T> > RangeMapBase<T>::toRange(const std::list<RangeMapBase>& rlist) {
    std::list<Range<T> > rv;
    for(typename std::list<RangeMapBase<T> >::const_iterator it = rlist.begin();
        it != rlist.end(); ++it) {
      std::list<Range<T> > slist = it->toRange();
      rv.insert(rv.end(), slist.begin(), slist.end());
    }
    return rv;
  }

  // convert to string
  template<class T> inline
  std::string RangeMapBase<T>::toString(const std::list<RangeMapBase>& rlist, bool compress) {
    std::string rv;
    if(!rlist.empty()) {
      if(rlist.size() > 1) {  // more than one sub-ranges
        rv += "{";
        for(typename std::list<RangeMapBase<T> >::const_iterator it = rlist.begin();
            it != rlist.end(); ) {
          rv += it->toString(compress);
          ++it;
          if(it != rlist.end()) rv += ";";
        }
        rv += "}";
      } else {                // only one sub-range
        rv += rlist.front().toString(compress);
      }
    }
    return rv;
  }

  // standard out stream
  template<class T>
  std::ostream& operator<< (std::ostream& os, const RangeMapBase<T>& r) {
    os << r.toString();
    return os;
  }

}

#endif
