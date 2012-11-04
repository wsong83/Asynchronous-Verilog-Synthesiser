/*
 * Copyright (c) 2011-2012 Wei Song <songw@cs.man.ac.uk> 
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
 * relation tree and forest needed in SDFG generation
 * 02/11/2012   Wei Song
 *
 *
 */

#include "rtree.hpp"
#include <boost/foreach.hpp>

using namespace SDFG;
using std::string;
using std::list;
using boost::shared_ptr;

/////////////////////////////////////////////////////////////////////////////
/********        relation tree nodes                                ********/
/////////////////////////////////////////////////////////////////////////////
SDFG::RTree::RTree()
  : type(RT_DF) {}

SDFG::RTree::RTree(node_type_t t)
  : type(t) {}

RTree* SDFG::RTree::deep_copy() const {
  RTree* rv = new RTree(type);
  rv->sig = sig;
  BOOST_FOREACH(const shared_ptr<RTree>& m, child) {
    rv->child.push_back(shared_ptr<RTree>(m->deep_copy()));
  }
  return rv;
}


/////////////////////////////////////////////////////////////////////////////
/********        relation forest                                    ********/
/////////////////////////////////////////////////////////////////////////////

SDFG::RForest::RForest(bool d_init) {
  if(d_init) {
    tree["@CTL"] = shared_ptr<RTree>(new RTree(RTree::RT_CTL));
    tree["@DATA"] = shared_ptr<RTree>(new RTree(RTree::RT_DATA));
  }
}

RForest* SDFG::RForest::deep_copy() const {
  RForest* rv = new RForest();
  BOOST_FOREACH(const tree_map_type& m, tree) {
    rv->tree[m.first] = shared_ptr<RTree>(m.second->deep_copy());
  }
  return rv;
}

void SDFG::RForest::combine(std::list<shared_ptr<RForest> > branches) {
  // ATTN: assuming this forest is representing the control signals
  
  // get a combined target signal map
  std::set<string> targets;
  BOOST_FOREACH(shared_ptr<RForest> f, branches) {
    BOOST_FOREACH(tree_map_type t, f->tree) {
      targets.insert(t.first);
    }
  }

  // fetch the control tree
  shared_ptr<RTree> ctree = tree["@CTL"];
  tree.erase("@CTL");
  
  // build up the tree
  BOOST_FOREACH(const string& t, targets) {
    
  }
}
