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
 * A help library of Synchronous Data-Flow Graph (SDFG)
 * 17/09/2012   Wei Song
 *
 *
 */

#ifndef _SDFG_H_
#define _SDFG_H_

#include <boost/shared_ptr.hpp>
#include <string>
#include <map>
#include <list>

// the BGL library
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>

// pugixml library
#include "pugixml/pugixml.hpp"

// forward decalration
namespace netlist {
  class NetComp;
}

// forward declaration
namespace ogdf {
  class Graph;
  class GraphAttributes;
}

// the Synchronous Data-Flow Graph (SDFG) library
namespace SDFG {

  typedef boost::adjacency_list<boost::multisetS, boost::vecS, boost::bidirectionalS> GType;
  typedef boost::graph_traits<GType> GraphTraits;
  typedef typename GraphTraits::edge_descriptor edge_descriptor;
  typedef typename GraphTraits::vertex_descriptor vertex_descriptor;
  typedef typename GraphTraits::vertex_iterator vertex_iterator;
  typedef typename GraphTraits::edge_iterator edge_iterator;

  class dfgGraph;

  class dfgNode {
  public:

    boost::shared_ptr<netlist::NetComp> ptr;   // pointer to the netlist component
    boost::shared_ptr<dfgGraph> child;         // when it is a module entity, it should has a child
    std::string child_name;                    // when it is a module entity, this is the module name of the module
    std::multimap<std::string, std::string> sig2port;   // remember the port connection if it is a module entity
    std::map<std::string, std::string> port2sig;        // remember the port connection if it is a module entity
    dfgGraph* pg;                                       // a pointer pointing to the father Graph
    std::string name;           // description of this node
    vertex_descriptor id;         // node id
    enum node_type_t {          // node type
      SDFG_DF             = 0x0000, // default, unknown yet
      SDFG_COMB           = 0x0001, // combinational assign or always
      SDFG_FF             = 0x0002, // flip-flop
      SDFG_LATCH          = 0x0004, // latch ?!
      SDFG_MODULE         = 0x0008, // module entity
      SDFG_IPORT          = 0x0050, // input port
      SDFG_OPORT          = 0x0060, // output port
      SDFG_PORT           = 0x0040  // all ports
    } type;


    dfgNode(): pg(NULL), position(0,0), bbox(0,0) {}
    dfgNode(const std::string& n, node_type_t t = SDFG_DF) : pg(NULL), name(n), type(t) {}
    void write(pugi::xml_node&, std::list<boost::shared_ptr<dfgGraph> >&) const;
    void write(void *, ogdf::GraphAttributes *);
    bool read(const pugi::xml_node&);
    bool read(void * const, ogdf::GraphAttributes * const);

    std::pair<double, double> position; // graphic position
    std::pair<double, double> bbox;     // bounding box
    void graphic_init();                // set initial graphic info.
  };

  class dfgEdge {
  public:

    dfgGraph* pg;                                       // a pointer pointing to the father Graph
    std::string name;           // edge name
    edge_descriptor id;            // edge id
    enum edge_type_t {
      SDFG_DF             = 0x0000, // default, unknown yet
      SDFG_DP             = 0x0001, // data path
      SDFG_CTL            = 0x0008, // control path
      SDFG_CLK            = 0x000a, // clk
      SDFG_RST            = 0x000c  // reset
    } type;

    dfgEdge() : pg(NULL) {}
    dfgEdge(const std::string& n, edge_type_t t = SDFG_DF) : pg(NULL), name(n), type(t) {}
    void write(pugi::xml_node&) const;
    void write(void *, ogdf::GraphAttributes *);
    bool read(const pugi::xml_node&);
    bool read(void * const, ogdf::GraphAttributes * const);

    std::list<std::pair<double, double> > bend; // bending points of the edge

  };

  class dfgGraph{
  public:
    GType bg_;                  // BGL graph
    dfgNode* father;            // father when it is a entity of another module
    std::string name;           // description of this node
    
    std::map<edge_descriptor, boost::shared_ptr<dfgEdge> > edges;
    std::map<vertex_descriptor, boost::shared_ptr<dfgNode> > nodes;

    std::map<std::string, vertex_descriptor > port_map;
    std::map<std::string, vertex_descriptor> node_map;

    dfgGraph() : father(NULL) {}
    dfgGraph(const std::string& n) : father(NULL), name(n) {}

    // add, remove and fetch nodes
    void add_node(boost::shared_ptr<dfgNode>);
    boost::shared_ptr<dfgNode> add_node(const std::string&, dfgNode::node_type_t);
    void add_edge(boost::shared_ptr<dfgEdge>, const std::string&, const std::string&);
    void add_edge(boost::shared_ptr<dfgEdge>, const vertex_descriptor&, const vertex_descriptor&);
    boost::shared_ptr<dfgEdge> add_edge(const std::string&, dfgEdge::edge_type_t, const std::string&, const std::string&);
    bool remove_node(boost::shared_ptr<dfgNode>);
    bool remove_node(const std::string&);
    bool remove_node(const vertex_descriptor&);
    bool remove_edge(boost::shared_ptr<dfgEdge>);
    bool remove_edge(boost::shared_ptr<dfgNode>, boost::shared_ptr<dfgNode>); // !! remove all edge between these two nodes
    bool remove_edge(const std::string&, const std::string&); // !! remove all edge between these two nodes
    bool remove_edge(const vertex_descriptor&, const vertex_descriptor&); // !! remove all edge between these two nodes
    bool remove_edge(boost::shared_ptr<dfgNode>, boost::shared_ptr<dfgNode>, dfgEdge::edge_type_t);
    bool remove_edge(const std::string&, const std::string&, dfgEdge::edge_type_t);
    bool remove_edge(const vertex_descriptor&, const vertex_descriptor&, dfgEdge::edge_type_t);
    bool remove_edge(const edge_descriptor&);
    boost::shared_ptr<dfgEdge> get_edge(const edge_descriptor&) const;
    boost::shared_ptr<dfgEdge> get_edge(const std::string&, const std::string&) const;   // return a random one if multiple
    boost::shared_ptr<dfgEdge> get_edge(const vertex_descriptor&, const vertex_descriptor&) const;   // return a random one if multiple
    boost::shared_ptr<dfgEdge> get_edge(const std::string&, const std::string&, dfgEdge::edge_type_t) const;
    boost::shared_ptr<dfgEdge> get_edge(const vertex_descriptor&, const vertex_descriptor&, dfgEdge::edge_type_t) const;
    boost::shared_ptr<dfgNode> get_node(const vertex_descriptor&) const;
    boost::shared_ptr<dfgNode> get_node(const std::string&) const;
    boost::shared_ptr<dfgNode> get_source(const edge_descriptor&) const;
    boost::shared_ptr<dfgNode> get_source(boost::shared_ptr<dfgEdge>) const;
    boost::shared_ptr<dfgNode> get_target(const edge_descriptor&) const;
    boost::shared_ptr<dfgNode> get_target(boost::shared_ptr<dfgEdge>) const;
    

    // existance check
    bool exist(const std::string&, const std::string&) const;   // edge
    bool exist(const std::string&, const std::string&, dfgEdge::edge_type_t) const; // edge 
    bool exist(const vertex_descriptor&, const vertex_descriptor&) const; // edge 
    bool exist(const vertex_descriptor&, const vertex_descriptor&, dfgEdge::edge_type_t) const; // edge 
    bool exist(const edge_descriptor&) const;
    bool exist(const std::string&) const;   // node   

    // graphic
    bool layout();
    bool layout(ogdf::Graph*, ogdf::GraphAttributes *);

    // graphic formats
    void write(std::ostream&) const;
    void write(pugi::xml_node&, std::list<boost::shared_ptr<dfgGraph> >&) const;
    void write(ogdf::Graph*, ogdf::GraphAttributes*);
    bool read(const pugi::xml_node&);
    bool read(ogdf::Graph* const, ogdf::GraphAttributes* const);

    // analyse functions
    void simplify(std::list<boost::shared_ptr<dfgNode> >&); // remove unused node and edges
  };

  boost::shared_ptr<dfgGraph> read(std::istream&);
}

#endif /* _SDFG_H_ */
