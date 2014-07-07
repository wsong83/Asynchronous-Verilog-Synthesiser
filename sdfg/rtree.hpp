/*
 * Copyright (c) 2014-2014 Wei Song <songw@cs.man.ac.uk> 
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
 * relation tree used to extract the input arcs for a set of signals in a block
 * 04/07/2014   Wei Song
 *
 *
 */

#ifndef _SDFG_RTREE_H_
#define _SDFG_RTREE_H_

#include <list>
#include <string>
#include <set>
#include <map>
#include "dfg_range.hpp"
#include "dfg_edge.hpp"

namespace SDFG {

  class RTree {

    std::string root;                           // the root of this sub-tree 
    dfgRangeMap select;                         // the range expression
    unsigned int relation;                      // relation (type) with higher node 

    std::set<std::string> fname_set;            // store the leaf names 
    std::multimap<std::string, RTree> leaves;   // store the leaves with different ranges and types

  public:
    RTree(const std::string&, const dfgRangeMap& select = dfgRangeMap(), 
          unsigned int t = dfgEdge::SDFG_ASS);
    
    // builders

    // when seq = true, combine sequential statements
    RTree& add(const RTree&);                   // add a sub tree, unflattened
    RTree& combine(const RTree&);               // combine a map of leaves, unflattened
    void flatten();                             // remove all hierarchies 
    void flatten_insert();                      // add a relation in the flattened tree
    void combine_sequential(const RTree&);      // combined a sequential flattened tree

    // other
    friend class RForest;

  };


  typedef std::multimap<std::string, RTree> leaf_map;
  typedef std::map<std::string, 
                   std::map<std::string, 
                            std::list<boost::tuple<SDFG::dfgRangeMap,
                                                   SDFG::dfgRangeMap,
                                                   unsigned int
                                                   > > > > plain_map;
  class RForest {

    std::set<std::string> tname_set;            // store the tree names 
    leaf_map trees;                             // all trees in this forest 

  public:
    
    //builder
    RForest& add(const RTree&);                 // add a tree
    RForest& combine(const RForest&);           // combine with another tree 
    void flatten();                             // remove all hierarchies 
    void combine_sequential(const RForest&);    // combined a sequential flattened tree
    const plain_map& get_plain_map() const;     // get a plain map to draw SDFG
  };


}


#endif
