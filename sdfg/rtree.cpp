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
#include "dfg_edge.hpp"

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
RTree::RTree(const string& root, const dfgRange& select)
  : root(root), select(select) {}


///////////////////////////////////
//    Tree builders

// add a leaf node
RTree& RTree::add(const RTree& leaf) {
  if(leaves.count(leaf.root)) { // there is a match in the tree
    typename leaf_map::iterator it, iend;
    boost::tie(it, iend) = leaves.equal_range(leaf.root);
    for(; it!=iend; ++it) {
      if(it->select == leaf.select && it->relation == leaf.relation) {
        it->combine(leaf);
        return *this;
      }
    }
  }

  // if goes here, add the leaf directly
  leaves.insert(boost::make_tuple(leaf.root, leaf));
  fname_set.insert(leaf.root);
  return *this;
}

// combine two leaf maps
RTree& RTree::combine(const RTree& t) {
  typename leaf_map::const_iterator it;
  for(it = t.leaves.begin(); it != t.leaves.end(); ++it)
    add(*it); 
  return *this;
}

// flatten the sub-tree
void RTree::flatten() {
  leaf_map orig_map = leaves;
  leaves.clear();

  typename leaf_map::iterator it;
  for(it=orig_map.begin(); it!=orig_map.end(); ++it) {
    it->flatten();
    flatten_insert(*it);
    typename leaf_map::iterator sub_it;
    for(sub_it=it->leaves.begin(); sub_it!=it->leaves.end(); ++sub_it)
      flatten_insert(*sub_it);
  }
  
}

// add a relation in the flattened tree
void RTree::flatten_insert(const RTree& t) {
  if(leaves.count(t.root)) {
    typename leaf_map::iterator it, iend;
    boost::tie(it, iend) = leaves.equal_range(t.root);
    for(; it!=iend; ++it) {
      if(it->relation == t.relation) {
        select = select.combine(t.select);
        return;
      }
    }
  }
  
  fname_set.insert(t.root);
  leaves.insert(boost::make_tuple(t.root, t));
}

// combined a sequential flattened tree
void RTree::combine_sequential(const RTree& t) {
  assert(root == t.root && select == t.select);
  typename leaf_map::iterator it;
  for(it=t.leaves.begin(); it!=t.leaves.end(); ++it) {
    if(it->relation != dfgEdge::SDFG_DDP)
      flatten_insert(*it);
  }
}



/* ------------------------ RForest ----------------------- */


///////////////////////////////////
//    Constructor


///////////////////////////////////
//    Forest builders

RForest& RForest::add(const RTree& t) {
  if(trees.count(t.root)) {
    typename leaf_map::iterator it, iend;
    boost::tie(it, iend) = trees.equal_range(t.root);
    for(; it!=iend; ++it) {
      if(it->select == t.select) {
        it->combine(t);
        return *this;
      }
    }
  }
  
  tname_set.insert(t.root);
  trees.insert(boost::make_tuple(t.root, t));
  return *this;
}

// combine with another tree 
RForest& RForest::combine(const RForest& f) {
  typename leaf_map::iterator it;
  for(it=f.trees.begin(); it!=f.trees.end(); ++it) {
    add(*it);
  }
  return *this;
}

// remove all hierarchies 
void RForest::flatten() {
  typename leaf_map::iterator it;
  for(it=f.trees.begin(); it!=f.trees.end(); ++it) {
    it->flatten();
  }

  leaf_map orig_trees = trees;
  trees.clear();

  // combine ranges
  BOOST_FOREACH(tr, tname_set) {
    typename leaf_map::iterator it, iend;
    boost::tie(it, iend) = orig_trees.equal_range(tr);
    
    // make a combined map
    map<unsigned int, map<string, list<pair<dfgRangeMap, dfgRangeMap> > > > rmap;
    for(; it!= iend; ++it) {
      typename leaf_map::iterator sub_it;
      for(sub_it = it->leaves.begin(); sub_it != it->leaves.end(); ++sub_it) {
        list<pair<dfgRangeMap, dfgRangeMap> >& rlist = rmap[sub_it->relation][sub_it->root];
        list<pair<dfgRangeMap, dfgRangeMap> >::iterator lit;
        for(lit = rlist.begin(); lit != rlist.end(); ++lit) {
          if(lit->first == sub_it->select) {
            lit->second = lit->second.combine(it->select);
            break;
          }
        }
        if(lit == rlist.end())
          rlist.push_back(boost::make_tuple(sub_it->select, it->select));
      }
    }

    // rebuild the trees
    map<unsigned int, map<string, list<pair<dfgRangeMap, dfgRangeMap> > > >::iterator rit;
    for(rit = rmap.begin(); rit != rmap.end(); ++rit) {
      map<string, list<pair<dfgRangeMap, dfgRangeMap> > >::iterator sit;
      for(sit = rit->second.begin(); sit != rit->second.end(); ++sit) {
        list<pair<dfgRangeMap, dfgRangeMap> >::iterator lit;
        for(lit = sit->second.begin(); lit!= sit->second.end(); ++lit) {
          RTree new_tree(tr, lit->second);
          new_tree.add(RTree(sit->first, lit->first, rit->first));
        }
      }
    } 
  }
}

// combined a sequential flattened tree
void RForest::combine_sequential(const RForest& f) {
  leaf_map::iterator it, lit, liend;
  for(it=f.trees.begin(); it != f.trees.end(); ++it) {
    if(trees.count(it->root)) {
      boost::tir(lit, liend) = trees.equal_range(it->root);
      for(; lit != liend; ++lit) {
        if(lit->select == it->select) {
          lit->combine_sequential(*it);
          break;
        }
      }
      if(lit == liend)
        add(*it);
    } else {
      add(*it);
    }
  }

  flatten();
}

// get a plain map to draw SDFG
const plain_map& RForest::get_plain_map() const {
  plain_map rv;

  // build a plain map
  leaf_map::iterator it, sit;
  for(it = trees.begin(); it != trees.end(); ++it) {
    for(sit = it->leaves.begin(); sit != it->leaves.end(); ++sit)
      rv[it->root][sit->root] = boost::make_tuple(it->select, sit->select, sit->relation);
  }

  return rv;
}
