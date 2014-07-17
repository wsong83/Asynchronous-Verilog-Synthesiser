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

  class RRelation {
    std::string name;
    dfgRangeMap select;
    unsigned int type;
    
  public:
    RRelation(const std::string& n = "", const dfgRangeMap& select = dfgRangeMap(), 
              unsigned int t = dfgEdge::SDFG_ASS);

    // accessor
    const dfgRangeMap& get_select() const { return select; }
    const std::string& get_name() const { return name; }
    unsigned int get_type() const { return type; }
    
    friend class RTree;
    friend class RForest;
  };

  typedef std::multimap<std::string, RRelation> leaf_map;
  typedef std::map<std::string, dfgRangeMap> sig_map;

  class RTree {

    std::string name;                           // the root of this sub-tree 
    dfgRangeMap select;                         // the range expression

    //std::set<std::string> nameSet;              // store the leaf names 
    leaf_map leaves;                            // store the leaves with different ranges and types

  public:
    RTree(const std::string& n = "", const dfgRangeMap& select = dfgRangeMap());
    
    // builders
    void add(const RRelation&);                 // add a sub tree, unflattened
    RTree& combine(const RTree&);               // combine a map of leaves, unflattened
    void combine_seq(const RTree&);             // combined a sequential flattened tree
    RTree& assign_type(unsigned int);           // assign a type which may override the original type

    // accessor
    const dfgRangeMap& get_select() const { return select; }
    const std::string& get_name() const { return name; }
    leaf_map::iterator begin() { return leaves.begin(); }
    leaf_map::iterator end() { return leaves.end(); }
    typedef leaf_map::iterator iterator;

    // helper
    sig_map get_all_signals() const;            // return a map of all right hand side signals 
    sig_map get_data_signals() const;           // return a map of all right hand side data sources
    sig_map get_control_signals() const;        // return a map of all control signals

    // other
    friend class RForest;

  private:
    void normalize();                           // normalize a tree after some modification

  };


  typedef std::multimap<std::string, RTree> tree_map;
  typedef std::list<boost::tuple<SDFG::dfgRangeMap,
                                 SDFG::dfgRangeMap,
                                 unsigned int
                                 > > plain_relation;
  typedef std::map<std::string, plain_relation> plain_map_item;
  typedef std::map<std::string, plain_map_item> plain_map;
  class RForest {

    //std::set<std::string> nameSet;              // store the tree names 
    tree_map trees;                             // all trees in this forest 

  public:

    // accessor
    tree_map::iterator begin() { return trees.begin(); }
    tree_map::iterator end() { return trees.end(); }
    typedef tree_map::iterator iterator;
    
    //builder
    void add(const RTree&);                     // add a tree
    void combine(const RForest&);               // combine with another tree 

    // helpers
    sig_map get_all_signals() const;            // return a map of all right hand side signals 
    sig_map get_data_signals() const;           // return a map of all right hand side data sources
    sig_map get_control_signals() const;        // return a map of all control signals
    sig_map get_target_signals() const;         // return a map of all targets

    const plain_map get_plain_map() const;      // get a plain map to draw SDFG
    
  };


}


#endif
