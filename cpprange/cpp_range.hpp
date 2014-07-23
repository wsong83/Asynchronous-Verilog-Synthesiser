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

// forward definitions of the utility functions
#include "cpp_range_util_def.hpp"

// RangeElement
#include "cpp_range_element.hpp"

// Simple multi-dimensional Range
// Does not allow operations when ranges have multiple different dimensions
#include "cpp_range_multi.hpp"

// Complex multi-dimensional Range
// Allow arbitrary different dimensions
#include "cpp_range_map_base.hpp"
#include "cpp_range_map.hpp"

// the utility function
#include "cpp_range_util.hpp"

#endif
