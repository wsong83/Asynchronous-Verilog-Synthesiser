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


#include "rtree.hpp"
#include <boost/tuple/tuple.hpp>
#include <boost/foreach.hpp>
#include "dfg_path.hpp"

using namespace SDFG;
using std::string;
using std::multimap;
using std::pair;
using std::set;
using boost::shared_ptr;

/* ------------------------ RRelation ----------------------- */


///////////////////////////////////
//    Constructor
 

// construct a leaf
RTree::RRelation(const string& n, const dfgRange& select, unsigned int t)
  : name(n), select(select), type(t) {}


/* ------------------------ RTree ----------------------- */


///////////////////////////////////
//    Constructor

// construct a leaf
RTree::RTree(const string& n, const dfgRange& select)
  : name(n), select(select) {}


///////////////////////////////////
//    Tree builders

// add a leaf node
void RTree::add(const RRelation& leaf) {
  if(leaves.count(leaf.name)) { // there is a match in the tree
    typename leaf_map::iterator it, iend;
    boost::tie(it, iend) = leaves.equal_range(leaf.name);
    for(; it!=iend; ++it) {
      if(it->second.type == leaf.type) {
        it->second.select = it->second.select.combine(leaf.select);
        return;
      }
    }
  }

  // if goes here, add the leaf directly
  leaves.insert(boost::make_tuple(leaf.name, leaf));
  nameSet.insert(leaf.name);
  return;
}

// combine two leaf maps
void RTree::combine(const RTree& t) {
  typename leaf_map::const_iterator it;
  for(it = t.leaves.begin(); it != t.leaves.end(); ++it)
    add(it->second); 
  return;
}

// combined a sequential flattened tree
void RTree::combine_seq(const RTree& t) {
  typename leaf_map::iterator it;
  for(it=t.leaves.begin(); it!=t.leaves.end(); ++it) {
    if(it->second.relation != dfgEdge::SDFG_DDP)
      add(it->second);
  }
}

// assign a new type
RTree& RTree::assign_type(unsigned int t) {
  typename leaf_map::const_iterator it;
  for(it = leaves.begin(); it != leaves.end(); ++it)
    it->second.type = dfgPath::cal_type(it->second.type, t);
  normalize();
  return *this;
}

// normalize a tree after some modification
void RTree::normalize() {
  leaf_map orig_tree = leaves;
  leaves.clear();
  typename leaf_map::const_iterator it;
  for(it = orig_tree.begin(); it != orig_tree.end(); ++it)
    add(it->second);
}


/* ------------------------ RForest ----------------------- */


///////////////////////////////////
//    Constructor


///////////////////////////////////
//    Forest builders

void RForest::add(const RTree& t) {
  if(trees.count(t.name)) {
    typename leaf_map::iterator it, iend;
    boost::tie(it, iend) = trees.equal_range(t.root);
    for(; it!=iend; ++it) {
      if(it->second.select.overlap(t.select)) {
        if(it->second.select == t.select) {
          it->second.combine_seq(t);
          return;
        } else {
          RTree mtree = it->second;
          dfgRangeMap rangeShared = it->second.select.intersection(t.select);
          it->second.select = rangeShared;
          it->second.combine_seq(t);
          mtree.select = mtree.select.complement(rangeShared);
          if(!mtree.select.empty())
            add(mtree);
          mtree = t;
          mtree.select = mtree.select.complement(rangeShared);
          if(!mtree.select.empty())
            add(mtree);
          return;
        }
      }
    }
  }
  
  tname_set.insert(t.name);
  trees.insert(boost::make_tuple(t.name, t));
  return *this;
}

// combine with another tree 
void RForest::combine(const RForest& f) {
  typename leaf_map::iterator it;
  for(it=f.trees.begin(); it!=f.trees.end(); ++it) {
    add(it->second);
  }
}

// get a plain map to draw SDFG
const plain_map& RForest::get_plain_map() const {
  plain_map rv;

  // build a plain map
  leaf_map::iterator it, sit;
  for(it = trees.begin(); it != trees.end(); ++it) {
    for(sit = it->leaves.begin(); sit != it->leaves.end(); ++sit)
      rv[it->second.name][sit->second.name] = boost::make_tuple(it->second.select, sit->second.select, sit->second.type);
  }

  return rv;
}
