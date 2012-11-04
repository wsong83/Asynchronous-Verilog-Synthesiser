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

void SDFG::RTree::write(pugi::xml_node& g, pugi::xml_node& father, unsigned int& index) const {
  string name;
  BOOST_FOREACH(const string& s, sig)
    name += s + ";";
  name.erase(name.size() - 1);

  // add this node
  pugi::xml_node cnode = g.append_child("node");
  cnode.append_attribute("id") = index++;
  cnode.append_attribute("name") = name;
  cnode.append_attribute("type") = unknown;
  
  // add arc
  pugi::xml_node carc = g.append_child("edge");
  carc.append_attribute("source") = cnode.attribute("id");
  carc.append_attribute("target") = father.attribute("id");
  if(type == RT_CTL)
    carc.append_attribute("type") = "control";
  else
    carc.append_attribute("type") = "data";

  // connect all child
  BOOST_FOREACH(const shared_ptr<RTree>& t, child)
    t->write(g, cnode, index);
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
    // build up the control for this node
    tree[t] = shared_ptr<RTree>(ctree.deep_copy());
    BOOST_FOREACH(shared_ptr<RForest> f, branches) {
      if(f->tree.count(t)) {
        tree[t]->child->push_back(shared_ptr<RTree>(f->tree[t]->deep_copy()));
      } else {
        tree[t]->child->push_back(shared_ptr<RTree>()); // empty shared_ptr to represent self loop
      }
    }
  }
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
    pugixml::xml_node subtree = xgraph.append_child("node");
    subtree.append_attribute("id") = i++;
    subtree.append_attribute("name") = t.first.c_str();
    subtree.append_attribute("type") = "unknown";
    t.second->write(xgraph, subtree, i);
  }

  sdfg_file.save(os);
}

