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
RRelation::RRelation(const string& n, const dfgRangeMap& select, unsigned int t)
  : name(n), select(select), type(t) {}

// stream out
std::ostream& RRelation::streamout(std::ostream& os) const {
  std::list<dfgRange> rlist = select.toRange();
  std::list<dfgRange>::iterator it = rlist.begin();
  while(it != rlist.end()) {
    os << "{" << name << it->toString() << "<" << dfgPath::get_stype(type) << ">}";
    ++it;
    if(it != rlist.end()) os << ";";
  }
  return os;
}

/* ------------------------ RTree ----------------------- */


///////////////////////////////////
//    Constructor

// construct a leaf
RTree::RTree(const string& n, const dfgRangeMap& select)
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
  leaves.insert(pair<string, RRelation>(leaf.name, leaf));
  //nameSet.insert(leaf.name);
  return;
}

// combine two leaf maps
RTree& RTree::combine(const RTree& t) {
  typename leaf_map::const_iterator it;
  for(it = t.leaves.begin(); it != t.leaves.end(); ++it)
    add(it->second); 
  return *this;
}

// combined a sequential flattened tree
void RTree::combine_seq(const RTree& t) {
  typename leaf_map::const_iterator it;
  for(it=t.leaves.begin(); it!=t.leaves.end(); ++it) {
    if(it->second.type != dfgEdge::SDFG_DDP)
      add(it->second);
  }
}

// assign a new type
RTree& RTree::assign_type(unsigned int t) {
  typename leaf_map::iterator it;
  for(it = leaves.begin(); it != leaves.end(); ++it)
    it->second.type = dfgPath::cal_type(it->second.type, t);
  normalize();
  return *this;
}

// return a map of all right hand side signals
sig_map RTree::get_all_signals() const {
  sig_map rv;
  typename leaf_map::const_iterator it;
  for(it = leaves.begin(); it != leaves.end(); ++it)
    rv[it->second.name] = rv[it->second.name].combine(it->second.select);
  return rv;
}

// return a map of all right hand data sources
sig_map RTree::get_data_signals() const {
  sig_map rv;
  typename leaf_map::const_iterator it;
  for(it = leaves.begin(); it != leaves.end(); ++it)
    if(it->second.type & dfgEdge::SDFG_DAT_MASK)
      rv[it->second.name] = rv[it->second.name].combine(it->second.select);
  return rv;
}

// return a map of all control signals
sig_map RTree::get_control_signals() const {
  sig_map rv;
  typename leaf_map::const_iterator it;
  for(it = leaves.begin(); it != leaves.end(); ++it)
    if(it->second.type & dfgEdge::SDFG_CTL_MASK)
      rv[it->second.name] = rv[it->second.name].combine(it->second.select);
  return rv;
}

// stream out
std::ostream& RTree::streamout(std::ostream& os) const {
  os << name << select.toString() << "<-" << leaves.size() << std::endl;
  leaf_map::const_iterator it = leaves.begin();
  while(it != leaves.end()) {
    it->second.streamout(os);
    ++it;
    if(it != leaves.end())
      os << ";";
  }
  os << std::endl;
  return os;
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
    typename tree_map::iterator it, iend;
    boost::tie(it, iend) = trees.equal_range(t.name);
    for(; it!=iend; ++it) {
      if(it->second.select.overlap(t.select) || it->second.select == t.select) {
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
  
  //tname_set.insert(t.name);
  trees.insert(pair<string, RTree>(t.name, t));
  return;
}

// combine with another tree 
void RForest::combine(const RForest& f) {
  typename tree_map::const_iterator it;
  for(it=f.trees.begin(); it!=f.trees.end(); ++it) {
    add(it->second);
  }
}

// return a map of all right hand side signals
sig_map RForest::get_all_signals() const {
  sig_map rv;
  typename tree_map::const_iterator it;
  for(it=trees.begin(); it!=trees.end(); ++it) {
    sig_map tmap = it->second.get_all_signals();
    rv.insert(tmap.begin(), tmap.end());
  }
  return rv;
}

// return a map of all right hand data sources
sig_map RForest::get_data_signals() const {
  sig_map rv;
  typename tree_map::const_iterator it;
  for(it=trees.begin(); it!=trees.end(); ++it) {
    sig_map tmap = it->second.get_data_signals();
    rv.insert(tmap.begin(), tmap.end());
  }
  return rv;
}

// return a map of all control signals
sig_map RForest::get_control_signals() const {
  sig_map rv;
  typename tree_map::const_iterator it;
  for(it=trees.begin(); it!=trees.end(); ++it) {
    sig_map tmap = it->second.get_control_signals();
    rv.insert(tmap.begin(), tmap.end());
  }
  return rv;
}

// return a map of all targets
sig_map RForest::get_target_signals() const {
  sig_map rv;
  typename tree_map::const_iterator it;
  for(it=trees.begin(); it!=trees.end(); ++it) {
    rv[it->second.get_name()] = rv[it->second.get_name()].combine(it->second.get_select());
  }
  return rv;
}

// stream out
std::ostream& RForest::streamout(std::ostream& os) const {
  tree_map::const_iterator it = trees.begin();
  while(it != trees.end()) {
    it->second.streamout(os);
    ++it;
  }
  return os;
}

// get a plain map to draw SDFG
plain_map RForest::get_plain_map() const {
  plain_map rv;

  // build a plain map
  tree_map::const_iterator it;
  leaf_map::const_iterator sit;
  for(it = trees.begin(); it != trees.end(); ++it) {
    for(sit = it->second.leaves.begin(); sit != it->second.leaves.end(); ++sit)
      rv[it->second.name][sit->second.name].push_back(
        boost::tuple<dfgRangeMap, dfgRangeMap, unsigned int>
        (it->second.select, sit->second.select, sit->second.type));
  }

  return rv;
}
