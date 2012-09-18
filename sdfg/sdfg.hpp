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
 * A help library of SDFG
 * 17/09/2012   Wei Song
 *
 *
 */

#ifndef _SDFG_H_
#define _SDFG_H_

#include <boost/shared_ptr.hpp>
#include <string>
#include <map>

// the BGL library
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>


namespace netlist {
  class NetComp;
}

namespace SDFG {

  typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS> GType;
  typedef boost::graph_traits<GType> GraphTraits;
  typedef typename GraphTraits::edge_descriptor edge_descriptor;
  typedef typename GraphTraits::vertex_descriptor vertex_descriptor;
  typedef typename GraphTraits::vertex_iterator vertex_iterator;
  typedef typename GraphTraits::edge_iterator edge_iterator;

  class dfgGraph;

  class dfgNode {
  public:

    boost::shared_ptr<netlist::NetComp> ptr;   // pointer to the netlist component
    boost::shared_ptr<dfgGraph> child;         // when it is a module, it should has a child
    std::string name;           // description of this node
    vertex_descriptor id;         // node id
    enum node_type_t {          // node type
      SDFG_DF             = 0x0000, // default, unknown yet
      SDFG_COMB           = 0x0001, // combinational assign or always
      SDFG_FF             = 0x0002, // flip-flop
      SDFG_LATCH          = 0x0004, // latch ?!
      SDFG_MODULE         = 0x0008, // module entity
      SDFG_IPORT          = 0x0010, // input port
      SDFG_OPORT          = 0x0020, // output port
      SDFG_PORT           = 0x0030  // all ports
    } type;


    dfgNode() {}
    dfgNode(const std::string& n, node_type_t t = SDFG_DF) : name(n), type(t) {}

  };

  class dfgEdge {
  public:

    std::string name;           // edge name
    edge_descriptor id;            // edge id
    enum edge_type_t {
      SDFG_DF             = 0x0000, // default, unknown yet
      SDFG_DP             = 0x0001, // data path
      SDFG_CTL            = 0x0004, // control path
      SDFG_CLK            = 0x0005, // clk
      SDFG_RST            = 0x0006  // reset
    } type;

    dfgEdge() {}
    dfgEdge(const std::string& n, edge_type_t t = SDFG_DF) : name(n), type(t) {}

  };

  class dfgGraph{
  public:
    GType bg_;                  // BGL graph
    boost::shared_ptr<dfgNode> father; // father when it is a entity of another module
    std::string name;           // description of this node
    
    std::map<edge_descriptor, boost::shared_ptr<dfgEdge> > edges;
    std::map<vertex_descriptor, boost::shared_ptr<dfgNode> > nodes;

    std::map<std::string, vertex_descriptor > port_map;
    std::map<std::string, vertex_descriptor> node_map;

    dfgGraph() {}
    dfgGraph(const std::string& n) : name(n) {}

    void add_node(boost::shared_ptr<dfgNode>);
    boost::shared_ptr<dfgNode> add_node(const std::string&, dfgNode::node_type_t);
    void add_edge(boost::shared_ptr<dfgEdge>, const std::string&, const std::string&);
    boost::shared_ptr<dfgEdge> add_edge(const std::string&, dfgEdge::edge_type_t, const std::string&, const std::string&);
    boost::shared_ptr<dfgEdge> get_edge(edge_descriptor);
    boost::shared_ptr<dfgEdge> get_edge(edge_descriptor) const;
    boost::shared_ptr<dfgNode> get_node(vertex_descriptor);
    boost::shared_ptr<dfgNode> get_node(vertex_descriptor) const;
    boost::shared_ptr<dfgNode> get_node(const std::string&);
    boost::shared_ptr<dfgNode> get_node(const std::string&) const;

  };

}

#endif /* _SDFG_H_ */
