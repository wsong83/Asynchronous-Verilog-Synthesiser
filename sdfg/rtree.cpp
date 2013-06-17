/*
 * Copyright (c) 2011-2013 Wei Song <songw@cs.man.ac.uk> 
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
#include <map>
#include <boost/foreach.hpp>
#include <boost/format.hpp>

#include "dfg_path.hpp"

using namespace SDFG;
using std::string;
using std::list;
using std::map;
using boost::shared_ptr;

string const SDFG::RTree::DTarget("@");

SDFG::RTree::RTree(bool default_case) {
  if(default_case) tree[DTarget] = map<string, int>();
}

SDFG::RTree::RTree(const string& sig, int etype) {
  tree[DTarget][sig] = etype;
}

SDFG::RTree::RTree(shared_ptr<RTree> st, int etype) {
  assert(st->tree.size() == 1 && st->tree.count(DTarget));
  add_tree(st, etype);
}

SDFG::RTree::RTree(shared_ptr<RTree> st0, shared_ptr<RTree> st1, int etype) {
  assert(st0->tree.size() == 1 && st0->tree.count(DTarget));
  assert(st1->tree.size() == 1 && st1->tree.count(DTarget));

  add_tree(st0, etype)->add_tree(st1, etype);
}
  
SDFG::RTree::RTree(shared_ptr<RTree> stc, shared_ptr<RTree> st0, shared_ptr<RTree> st1) {
  assert(stc->tree.size() == 1 && stc->tree.count(DTarget));
  assert(st0->tree.size() == 1 && st0->tree.count(DTarget));
  assert(st1->tree.size() == 1 && st1->tree.count(DTarget));
  
  add_tree(stc, SDFG::dfgEdge::SDFG_CTL)->add_tree(st0)->add_tree(st1);
}

RTree* SDFG::RTree::add_edge(const string& sig, int etype) {
  assert(tree.size() == 1 && tree.count(DTarget));
  return add_edge(sig, DTarget, etype);
}

RTree* SDFG::RTree::add_edge(const string& sig, const string& root, int etype) {
  assert(tree.count(root));
  if(tree[root].count(sig))
    tree[root][sig] |= etype;
  else
    tree[root][sig] = etype;
  return this;
}

RTree* SDFG::RTree::add_tree(shared_ptr<RTree> pt, int rtype) {
  BOOST_FOREACH(sub_tree_type& t, pt->tree) {
    if(!tree.count(t.first)) tree[t.first] = map<string, int>();
    copy_subtree(t.first, t.second, rtype);
  }
  return this;
}

RTree* SDFG::RTree::add_tree(shared_ptr<RTree> pt, const string& root, int rtype) {
  BOOST_FOREACH(sub_tree_type& t, pt->tree) {
    if(t.first == DTarget) {    // default sub-tree
      if(!tree.count(root)) tree[root] = map<string, int>();
      copy_subtree(root, t.second, rtype);
    } else {                    // named sub-tree
      if(!tree.count(t.first)) tree[t.first] = map<string, int>();
      copy_subtree(t.first, t.second, rtype);
    }
  }
  return this;
}

RTree* SDFG::RTree::combine(shared_ptr<RTree> rhs, int rtype) {
  //std::cout << "combine:" << std::endl;
  //std::cout << "this:\n" << *this << std::endl;
  //std::cout << "rhs:\n" << *rhs << std::endl;
  BOOST_FOREACH(sub_tree_type& t, tree) {
    add_tree(rhs, t.first, rtype);
  }
  //std::cout << "result:\n" << *this << std::endl;
  return this;
}

std::set<string> SDFG::RTree::get_control(const string& t) const {
  return get_signals (dfgEdge::SDFG_CTL | dfgEdge::SDFG_CMP | dfgEdge::SDFG_EQU, t);
}

std::set<string> SDFG::RTree::get_data(const string& t) const {
  return get_signals (dfgEdge::SDFG_DDP | dfgEdge::SDFG_CAL | dfgEdge::SDFG_ASS | dfgEdge::SDFG_DAT, t);
}

std::set<string> SDFG::RTree::get_all(const string& t) const {
  return get_signals (dfgEdge::SDFG_DAT_MASK | dfgEdge::SDFG_CTL_MASK | dfgEdge::SDFG_CR_MASK, t);
}

std::set<string> SDFG::RTree::get_signals(int stype, const string& sig) const {
  std::set<string> rv;
  if(sig == "") {
    BOOST_FOREACH(const sub_tree_type& st, tree) {
      BOOST_FOREACH(const rtree_edge_type& se, st.second) {
        if(se.second & stype) rv.insert(se.first);
      }
    }
  } else {
    if(tree.count(sig)) {
      BOOST_FOREACH(const rtree_edge_type& se, tree.find(sig)->second) {
        if(se.second & stype) rv.insert(se.first);
      }
    }
  }
  return rv;
}

std::ostream& SDFG::RTree::streamout(std::ostream& os) const {
  BOOST_FOREACH(const sub_tree_type& t, tree) {
    os << t.first << ": " << std::endl;
    BOOST_FOREACH(const rtree_edge_type& e, t.second) {
      os << "    " << dfgPath::get_stype(e.second) << " " << e.first << std::endl;
    }
  }
  return os;
}

void SDFG::RTree::copy_subtree(const string& root,  const map<string, int>& st, int rtype) {
  BOOST_FOREACH(const rtree_edge_type& e, st) {
    if(tree[root].count(e.first))
      tree[root][e.first] |= dfgPath::cal_type(e.second, rtype);
    else
      tree[root][e.first] = dfgPath::cal_type(e.second, rtype);
  }
}

