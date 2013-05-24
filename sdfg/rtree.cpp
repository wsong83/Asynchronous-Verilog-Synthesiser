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

SDFG::RTree::RTree() {
  tree[DTarget] = map<string, int>();
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
    BOOST_FOREACH(rtree_edge_type& e, t.second) {
      if(tree[t.first].count(e.first))
        tree[t.first][e.first] |= dfgPath::cal_type(e.second, rtype);
      else
        tree[t.first][e.first] = dfgPath::cal_type(e.second, rtype);
    }
  }
  return this;
}
  


/////////////////////////////////////////////////////////////////////////////
/********        relation tree nodes                                ********/
/////////////////////////////////////////////////////////////////////////////
/*
SDFG::RTree::RTree()
  : type(RT_DF) {}

SDFG::RTree::RTree(node_type_t t)
  : type(t) {}

RTree* SDFG::RTree::deep_copy() const {
  RTree* rv = new RTree(type);
  rv->sig = sig;
  BOOST_FOREACH(const shared_ptr<RTree>& m, child) {
    if(m)
      rv->child.insert(shared_ptr<RTree>(m->deep_copy()));
    else          // possible self-loop
      rv->child.insert(shared_ptr<RTree>());
  }
  return rv;
}

void SDFG::RTree::build(shared_ptr<RForest> rexp) {
  if(rexp->tree["@CTL"]) {
    sig = rexp->tree["@CTL"]->sig;
    type = RT_CTL;
    if(rexp->tree["@DATA"])
      child.insert(shared_ptr<RTree>(rexp->tree["@DATA"]->deep_copy()));
  } else {
    if(rexp->tree["@DATA"])
      sig = rexp->tree["@DATA"]->sig;
    type = RT_DATA;
  }
}

bool SDFG::RTree::insert_default(shared_ptr<RTree> t) {
  bool inserted = false;
  std::set<shared_ptr<RTree> > mchild = child;
  BOOST_FOREACH(shared_ptr<RTree> c, mchild) {
    if(c) inserted |= c->insert_default(t);
    else  {
      child.erase(c);
      child.insert(shared_ptr<RTree>(t->deep_copy()));
      inserted = true;
    }
  }
  return inserted;
}

void SDFG::RTree::append(shared_ptr<RTree> leaf) {
  if(child.empty()) {
    child.insert(shared_ptr<RTree>(leaf->deep_copy()));
  } else {
    BOOST_FOREACH(shared_ptr<RTree> c, child) {
      c->append(leaf);
    }
  }
}

void SDFG::RTree::get_control(std::set<string>& sigset) const {
  if(type == RT_CTL) {
    sigset.insert(sig.begin(), sig.end());
  }
  if(!child.empty()) {
    BOOST_FOREACH(const shared_ptr<RTree>& t, child)
      if(t) t->get_control(sigset);
  }
}

void SDFG::RTree::get_data(std::set<string>& sigset) const {
  if(type == RT_DATA) {
    sigset.insert(sig.begin(), sig.end());
  }
  if(!child.empty()) {
    BOOST_FOREACH(const shared_ptr<RTree>& t, child)
      if(t) t->get_data(sigset);
      else sigset.insert("");
  }
}

void SDFG::RTree::combine(shared_ptr<RTree> t0, shared_ptr<RTree> t1) {
  switch(t0->type) {
  case RT_DATA: {
    if(t0->sig.empty()) {
      sig = t1->sig;
      child = t1->child;
      type = RT_DATA;
    } else { 
      switch(t1->type) {
      case RT_DATA: {
        sig.insert(t0->sig.begin(), t0->sig.end());
        sig.insert(t1->sig.begin(), t1->sig.end());
        type = RT_DATA;
        break;
      }
      case RT_CTL: {
        t1->child.insert(t0);
        sig = t1->sig;
        child = t1->child;
        type = RT_CTL;
        break;
      }
      default: {
        assert(0 == "cannot combine these two trees");
        break;
      }
      }
    }
    break;
  }
  case RT_CTL: {
    t0->child.insert(t1);
    sig = t0->sig;
    child = t0->child;
    type = RT_CTL;
    break;
  }
  default: {
    assert(0 == "cannot combine these two trees");
    break;
  }
  }
}

void SDFG::RTree::write(pugi::xml_node& g, pugi::xml_node& father, unsigned int& index) const {
  string name;
  if(sig.empty()) {
    // const numbers
    name = "@const " + boost::str(boost::format("%u") % index);
  } else {
    BOOST_FOREACH(const string& s, sig)
      name += s + ";";
      name.erase(name.size() - 1);
  }

  // add this node
  pugi::xml_node cnode = g.append_child("node");
  cnode.append_attribute("id") = index++;
  cnode.append_attribute("name") = name.c_str();
  cnode.append_attribute("type") = "unknown";
  
  // add arc
  pugi::xml_node carc = g.append_child("edge");
  carc.append_attribute("source") = cnode.attribute("id").value();
  carc.append_attribute("target") = father.attribute("id").value();
  if(type == RT_CTL)
    carc.append_attribute("type") = "control";
  else
    carc.append_attribute("type") = "data";

  // connect all child
  BOOST_FOREACH(const shared_ptr<RTree>& t, child) {
    if(t) t->write(g, cnode, index);
    else {                      // possible self-loop
      pugi::xml_node sloop_node = g.append_child("node");
      sloop_node.append_attribute("id") = index++;
      sloop_node.append_attribute("name") = ("* " + boost::str(boost::format("%u") % (index-1))).c_str();
      sloop_node.append_attribute("type") = "unknown";
      pugi::xml_node sloop_arc = g.append_child("edge");
      sloop_arc.append_attribute("source") = sloop_node.attribute("id").value();
      sloop_arc.append_attribute("target") = cnode.attribute("id").value();
      sloop_arc.append_attribute("type") = "data";
    }
  }
}
*/
/////////////////////////////////////////////////////////////////////////////
/********        relation forest                                    ********/
/////////////////////////////////////////////////////////////////////////////
/*
SDFG::RForest::RForest(bool d_init) {
  if(d_init) {
    tree["@CTL"] = shared_ptr<RTree>(new RTree(RTree::RT_CTL));
    tree["@DATA"] = shared_ptr<RTree>(new RTree(RTree::RT_DATA));
  }
}
*/
/*
RForest* SDFG::RForest::deep_copy() const {
  RForest* rv = new RForest();
  BOOST_FOREACH(const tree_map_type& m, tree) {
    rv->tree[m.first] = shared_ptr<RTree>(m.second->deep_copy());
  }
  return rv;
}

void SDFG::RForest::build(shared_ptr<RForest> lval, shared_ptr<RForest> rexp) {
  shared_ptr<RTree> rt(new RTree());
  rt->build(rexp);
  
  BOOST_FOREACH(tree_map_type& t, lval->tree) {
    if(t.second) {
      tree[t.first].reset(t.second->deep_copy());
      tree[t.first]->append(rt);
    } else {
      tree[t.first].reset(rt->deep_copy());
    }
  }
}

void SDFG::RForest::add(shared_ptr<RForest> exp, list<shared_ptr<RForest> > branches) {
  shared_ptr<RForest> mf(exp->deep_copy());
  mf->combine(branches);
  add(mf);
}

void SDFG::RForest::add(shared_ptr<RForest> f) {
  BOOST_FOREACH(tree_map_type& t, f->tree) {
    if(tree.count(t.first)) {
      // use the tree in this as default
      shared_ptr<RTree> dftree = tree[t.first];
      tree[t.first] = t.second;
      if(!t.second->insert_default(dftree)) {
        if(!dftree->insert_default(t.second)) {
          shared_ptr<RTree> mtree(new RTree());
          mtree->combine(dftree, t.second); // compromised behavioral as there is no range check
          dftree = mtree;
        }
        tree[t.first] = dftree;
      }
    } else {
      tree[t.first] = t.second;
    }
  }
}

void SDFG::RForest::combine(list<shared_ptr<RForest> > branches) {
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
    // build up the control for this node
    tree[t] = shared_ptr<RTree>(ctree->deep_copy());
    BOOST_FOREACH(shared_ptr<RForest> f, branches) {
      if(f->tree.count(t)) {
        tree[t]->child.insert(shared_ptr<RTree>(f->tree[t]->deep_copy()));
      } else {
        tree[t]->child.insert(shared_ptr<RTree>()); // empty shared_ptr to represent self loop
      }
    }
  }
}

std::set<string> SDFG::RForest::get_control(const string& target) const {
  std::set<string> rv;
  if(tree.count(target)) {
    tree.find(target)->second->get_control(rv);
  }
  return rv;
}

std::set<string> SDFG::RForest::get_data(const string& target) const {
  std::set<string> rv;
  if(tree.count(target)) {
    tree.find(target)->second->get_data(rv);
  }
  return rv;
}

void SDFG::RForest::write(std::ostream& os) const {
  unsigned int i = 0;
  pugi::xml_document sdfg_file;       // using the pugixml library
  pugi::xml_node node_xml = sdfg_file.append_child(pugi::node_declaration);
  node_xml.append_attribute("version") = "1.0";
  node_xml.append_attribute("encoding") = "UTF-8";
  pugi::xml_node xgraph = sdfg_file.append_child("graph");

  // write out all trees
  BOOST_FOREACH(const tree_map_type& t, tree) {
    pugi::xml_node subtree = xgraph.append_child("node");
    subtree.append_attribute("id") = i++;
    subtree.append_attribute("name") = t.first.c_str();
    subtree.append_attribute("type") = "unknown";
    if(t.second) t.second->write(xgraph, subtree, i);
  }

  sdfg_file.save(os);
}

*/
