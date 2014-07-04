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

///////////////////////////////////
//    Constructor
 

// construct a leaf
RTree::RTree(const string& root, const dfgRange& select, unsigned int t)
  : root(root), select(select), relation(t) {}


///////////////////////////////////
//    Tree builders

// add a leaf node
RTree& RTree::add(shared_ptr<RTree> leaf, seq) {
  if(leaves.count(leaf.root)) { // there is a match in the tree
    typename leaf_map::iterator it, iend;
    boost::tie(it, iend) = leaves.equal_range(leaf.root);
    for(; it!=iend; ++it) {
      if(seq) {
        if(it->select == leaf.select) {
          if(!(it->relation & dfgEdge::SDFG_DFF))  leaf.relation &= ~dfgEdge::SDFG_DFF;
          it->relation |= leaf.relation;
          assert(leaf.leaves.empty());
          return *this;
        }
      } else {
        if(it->select == leaf.select && it->relation == leaf.relation) {
          it->combine(leaf.leaves);
          return *this;
        }
      }
    }
  }

  // if goes here, add the leaf directly
  leaves.insert(boost::make_tuple(leaf.root, leaf));
  fname_set.insert(leaf.root);
  return *this;
}

// combine two leaf maps
void RTree::combine(const leaf_map& fmap, bool seq) {
  typename leaf_map::const_iterator it;
  for(it = fmap.begin; it != fmap.end(); ++it)
    add(*it, seq); 
}

///////////////////////////////////
//    Helpers

// flatten the sub-tree
void RTree::flatten() {
  leaf_map orig_map = leaves;
  leaves.clear();

  typename leaf_map::iterator it;
  for(it=orig_map.begin(); it!=orig_map.end(); ++it) {
    it->flatten();
    
  }
  
}
