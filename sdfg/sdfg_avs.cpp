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
 * the part of SDFG related to AVS
 * 01/10/2012   Wei Song
 *
 *
 */


#include "sdfg.hpp"
#include <boost/tuple/tuple.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <set>
#include <algorithm>
#include <iostream>

using namespace SDFG;
using boost::shared_ptr;
using std::map;
using std::string;
using std::pair;
using std::list;


/////////////////////////////////////////////////////////////////////////////
/********        Node                                               ********/
/////////////////////////////////////////////////////////////////////////////

void SDFG::dfgNode::simplify(std::set<boost::shared_ptr<dfgNode> >& proc_set, bool quiet) {

  unsigned int oe_num = pg->size_out_edges(id);
  unsigned int ie_num = pg->size_in_edges(id);

  // remove internal node without output edges
  if(oe_num == 0) {
    if(!((type & SDFG_PORT) && (type != SDFG_IPORT) && (pg->father == NULL))) {
      BOOST_FOREACH(shared_ptr<dfgNode> m, pg->get_in_nodes(id)) {
        assert(m);
        proc_set.insert(m);
      }
      
      if((type & SDFG_PORT) && (type != SDFG_OPORT) && (pg->father)) // if it is an input, the whole module may be useless
        assert(pg->father->pg->nodes[pg->father->id]);
        proc_set.insert(pg->father->pg->nodes[pg->father->id]);
      
      pg->remove_node(id);        // remove it
      
      if(!quiet)
        std::cout << "node \"" << pg->name << "/" << get_hier_name() << "\" is removed as it has no output edges." << std::endl;
      return;
    }
  }

  // remove the node that has only one input and it is a comb or unknown
  if(ie_num == 1 && (type == SDFG_COMB || type == SDFG_DF)) {
    // get its source and target
    shared_ptr<dfgEdge> src = pg->get_in_edges(id).front();
    list<shared_ptr<dfgEdge> > tar_list = pg->get_out_edges(id);
    bool no_module = true;
    BOOST_FOREACH(shared_ptr<dfgEdge> m, tar_list) {
      no_module &= pg->get_target(m)->type != SDFG_MODULE;
    }
    shared_ptr<dfgNode> src_node = pg->get_source(src);

    // make sure it is not a signal connected to a module/iport (remove it will make the graph to crowed)
    if(!(src_node->type & SDFG_MODULE) && no_module) {
      // choosing the target edge type
      dfgEdge::edge_type_t etype;
      
      // add edges to by pass the node
      BOOST_FOREACH(shared_ptr<dfgEdge> m, tar_list) {
        // specify the type
        if(m->type == dfgEdge::SDFG_DF)
          etype = src->type;   // use src type if tar type is unknown
        else
          etype = m->type;     // always use the tar type if it is available

        // add the new path
        shared_ptr<dfgNode> tar_node = pg->get_target(m);
        pg->add_edge(src->name, etype, src_node->id, tar_node->id);

        // process target again later
        assert(tar_node);
        proc_set.insert(tar_node);
      }

      // remove the node
      pg->remove_node(id);
      
      // process source again later
      assert(src_node);
      proc_set.insert(src_node);
      
      if(!quiet)
        std::cout << "node \"" << pg->name << "/" << get_hier_name()
                  << "\" is removed and its single input node \""
                  << src->pg->name << "/" << src_node->get_hier_name() 
                  << "\" is connected to all output nodes." << std::endl;
      return;
    }
  }

  // remove the node that has only one output, and it is a comb or unknown
  if(oe_num == 1 && (type == SDFG_COMB || type == SDFG_DF)) {
    // get its source and target
    list<shared_ptr<dfgEdge> > src_list = pg->get_in_edges(id);
    bool no_module = true;
    BOOST_FOREACH(shared_ptr<dfgEdge> m, src_list) {
      no_module &= pg->get_source(m)->type != SDFG_MODULE;
    }
    shared_ptr<dfgEdge> tar = pg->get_out_edges(id).front();
    shared_ptr<dfgNode> tar_node = pg->get_target(tar);
    
    // make sure it is not a signal connected to a module/oport (remove it will make the graph to crowed)
    if(!(tar_node->type & SDFG_MODULE) && no_module) {
      // choosing the target edge type
      dfgEdge::edge_type_t etype;

      // add edges to by pass the node
      BOOST_FOREACH(shared_ptr<dfgEdge> m, src_list) {
        // specify the type
        if(tar->type == dfgEdge::SDFG_DF)
          etype = m->type;   // use src type if tar type is unknown
        else
          etype = tar->type;     // always use the tar type if it is available

        // add the new path
        shared_ptr<dfgNode> src_node = pg->get_source(m);
        pg->add_edge(m->name, etype, src_node->id, tar_node->id);

        // process source again later
        assert(src_node);
        proc_set.insert(src_node);
      }

      // remove the node
      pg->remove_node(id);
      
      // process source again later
      assert(tar_node);
      proc_set.insert(tar_node);
      
      if(!quiet)
        std::cout << "node \"" << pg->name << "/" << get_hier_name()
                  << "\" is removed and its single output node \""
                  << tar->pg->name << "/" << tar_node->get_hier_name() 
                  << "\" is connected with all input nodes." << std::endl;
      return;
    }
  }

  // remove through wires
  if(ie_num == 1 && type == SDFG_OPORT && pg->father != NULL) {
    shared_ptr<dfgNode> iport = pg->get_in_nodes(id).front();
    if(pg->size_out_edges(iport) == 1 && iport->type == SDFG_IPORT) {
      // input port
      shared_ptr<dfgNode> iport = pg->get_in_nodes(id).front();
      
      // get source and target
      shared_ptr<dfgNode> src = 
        pg->father->pg->get_node(pg->father->port2sig[iport->get_hier_name()]);
      shared_ptr<dfgNode> tar = 
        pg->father->pg->get_node(pg->father->port2sig[get_hier_name()]);
      
      // set up the bypass
      pg->father->pg->add_edge(src->get_hier_name(), pg->get_in_edges(id).front()->type, src->id, tar->id);
      
      // remove the through wire
      pg->remove_node(iport);
      pg->remove_node(id);
      
      // add src and tar to proc_set
      assert(src);
      proc_set.insert(src);
      assert(tar);
      proc_set.insert(tar);
      
      // remove iport from proc_set if it is in the list
      proc_set.erase(iport);

      if(!quiet)
        std::cout << "move the through wire from \"" 
                  << pg->name << "/" << iport->get_hier_name()
                  << "\" to \""
                  << pg->name << "/" << get_hier_name() 
                  << "\" to the upper module \"" 
                  << pg->father->pg->name 
                  << "\"." << std::endl;
      return;
    }
  }

  // go lower if it is module node
  if(type == SDFG_MODULE && child)
    child->simplify(proc_set, quiet);
}



/////////////////////////////////////////////////////////////////////////////
/********        Edge                                               ********/
/////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////
/********        Graph                                              ********/
/////////////////////////////////////////////////////////////////////////////


///////////////////////////////
// analyse functions
///////////////////////////////
void SDFG::dfgGraph::simplify(bool quiet) {
  // node cache
  std::set<shared_ptr<dfgNode> > proc_set;

  // in the first round, traverse all nodes
  simplify(proc_set, quiet);

  // handle the nodes need further operation
  while(!proc_set.empty()) {
    shared_ptr<dfgNode> pn = *(proc_set.begin());
    assert(pn);
    proc_set.erase(pn);
    pn->simplify(proc_set, quiet);
  }
}

void SDFG::dfgGraph::simplify(std::set<shared_ptr<dfgNode> >& proc_set, bool quiet) {

  // make a local copy of the node map, as nodes may be erased during the process
  map<vertex_descriptor, shared_ptr<dfgNode> > local_node_map = nodes;
  
  // do the simplification
  for_each(local_node_map.begin(), local_node_map.end(),
           [&](pair<const vertex_descriptor, shared_ptr<dfgNode> >& m) {
             m.second->simplify(proc_set, quiet);
           });
}


