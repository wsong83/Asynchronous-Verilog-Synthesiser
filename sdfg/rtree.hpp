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
#include <dfg_range.hpp>
#include <boost/shared_ptr.hpp>

namespace SDFG {

  // forward declaration
  class RTree;

  // global type definition
  typedef std::list<shared_ptr<RTree> > leaf_list_type;
  typedef std::pair<const std::string&, leaf_list_type> leaves_type;
  typedef std::multimap<std::string, leaf_list_type> leaf_map;

  class RTree : public dfgRangeMap {

    std::string root;                           // the root of this sub-tree 
    unsigned int relation;                      // relation (type) with higher node 

    std::set<std::string> fanme_set;            // store the leaf names 
    leaf_map leaves;                            // store the leaves with different ranges and types

  public:
    RTree(const string&, const dfgRangeMap& select = dfgRangeMap(), 
          unsigned int t = dfgEdge::SDFG_ASS);
    
    // builders

    // when seq = true, combine sequential statements
    RTree& add(boost::shared_ptr<RTree>, bool seq = false); // add a sub tree
    void combine(const leaf_map&, bool seq = false);        // combine a map of leaves

    // helpers
    void flatten();                             // remove all hierarchies 

    // other
    friend class RForest;

  };


  class RForest {

    leaf_map trees;                             // all trees in this forest 

  public:
    
    //builder
    RForest& add(boost::shared_ptr<RTree>);     // add a tree
    RForest& combine(const RForest&);           // combine with another tree 

  };


}


#endif
