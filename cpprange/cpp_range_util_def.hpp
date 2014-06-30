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
 * Define utility functions fot the C++ STL static range calculation library
 * 25/06/2014   Wei Song
 *
 *
 */

#ifndef _CPP_RANGE_UTIL_DEF_H_
#define _CPP_RANGE_UTIL_DEF_H_

#include <list>
#include <string>

namespace CppRange {

  // forward decaration
  template<class T> class RangeElement;
  template<class T> class Range;
  template<class T> class RangeMapBase;
  template<class T> class RangeMap;
  
  // parse a string into a range element
  template<class T>
  RangeElement<T> parse_range (const std::string&);

  // parse a string into a list of range element
  template<class T>
  std::list<RangeElement<T> > parse_range_list (const std::string&);


}

#endif
