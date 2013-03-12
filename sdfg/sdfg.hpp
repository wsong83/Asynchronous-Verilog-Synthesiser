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

  typedef boost::adjacency_list<boost::multisetS, boost::listS, boost::bidirectionalS> GType;
  typedef boost::graph_traits<GType> GraphTraits;
  typedef typename GraphTraits::edge_descriptor edge_descriptor;
  typedef typename GraphTraits::vertex_descriptor vertex_descriptor;
  typedef typename GraphTraits::vertex_iterator vertex_iterator;
  typedef typename GraphTraits::edge_iterator edge_iterator;

  class dfgGraph;
  class dfgPath;

  class dfgNode {
  public:

    boost::shared_ptr<netlist::NetComp> ptr;   // pointer to the netlist component
    boost::shared_ptr<dfgGraph> child;         // when it is a module entity, it should has a child
    std::string child_name;                    // when it is a module entity, this is the module name of the module
    std::map<std::string, std::list<std::string> > sig2port;   // remember the port connection if it is a module entity
    std::map<std::string, std::string> port2sig;        // remember the port connection if it is a module entity
    dfgGraph* pg;                                       // a pointer pointing to the father Graph
    std::string name;                                   // description of this node
    std::list<std::string> hier;                        // hierarchy prefix (name of flattened modules) 
    vertex_descriptor id;                               // node id
    unsigned int node_index;   // when nodes are stored in listS, vertext_descriptors are no longer
                                // integers, thereofer, separated indices must be generated and stored 
    enum node_type_t {          // node type
      SDFG_DF             = 0x00001, // default, unknown yet
      SDFG_COMB           = 0x00010, // combinational assign or always
      SDFG_FF             = 0x00020, // flip-flop
      SDFG_LATCH          = 0x00040, // latch ?!
      SDFG_MODULE         = 0x00080, // module entity
      SDFG_GATE           = 0x00100, // gate
      SDFG_IPORT          = 0x00a00, // input port
      SDFG_OPORT          = 0x00c00, // output port
      SDFG_PORT           = 0x00800  // all ports
    } type;

    // only available in register graph
    std::list<boost::shared_ptr<dfgPath> > opath, ipath; // record all output/input paths to avoid recalculation
    std::map<boost::shared_ptr<dfgNode>, int> opath_f, ipath_f, self_f; // record all output/input paths get from fast algorithm to avoid recalculation


    dfgNode(): pg(NULL), node_index(0), type(SDFG_DF), position(0,0), bbox(0,0) {}
    dfgNode(const std::string& n, node_type_t t = SDFG_DF) : 
      pg(NULL), name(n), node_index(0), type(t), position(0,0), bbox(0,0) {}
    dfgNode* copy() const;      // copy content, not deep copy, orphan node generation
    void write(pugi::xml_node&, std::list<boost::shared_ptr<dfgGraph> >&) const;
    void write(void *, ogdf::GraphAttributes *);
    bool read(const pugi::xml_node&);
    bool read(void * const, ogdf::GraphAttributes * const);
    boost::shared_ptr<dfgNode> flatten() const;   // move this node to one module higher
    std::string get_hier_name() const;            // get the hierarchical name of the name
    std::string get_full_name() const;            // get the hierarchical name of the name
    void set_hier_name(const std::string&);       // set the hierarchical name
    void remove_port_sig(const std::string&, int); // remove a certain port signal
    void add_port_sig(const std::string&, const std::string&); // add a certain port connection
    std::list<boost::shared_ptr<dfgPath> >& get_out_paths_cb(); // return all output paths from this register/port
    std::list<boost::shared_ptr<dfgPath> >& get_in_paths_cb(); // return all input paths to this register/port
    // return all output paths from this register/port, faster algorithm
    std::list<boost::shared_ptr<dfgPath> > get_out_paths_fast_cb(); 
    // return all input paths to this register/port, fast algorithm
    std::list<boost::shared_ptr<dfgPath> > get_in_paths_fast_cb(); 
    // return all self control paths that inside this module
    std::list<boost::shared_ptr<dfgPath> > get_self_path_cb();

    std::pair<double, double> position; // graphic position
    std::pair<double, double> bbox;     // bounding box
    void graphic_init();                // set initial graphic info.

  private:
    void out_path_type_update_cb(std::list<boost::shared_ptr<dfgPath> >&,
                                 boost::shared_ptr<dfgPath>&,
                                 std::map<boost::shared_ptr<dfgNode>, 
                                          std::map<boost::shared_ptr<dfgNode>, int> >&,
                                 std::set<boost::shared_ptr<dfgNode> >&); // helper for get_out_paths()

    void in_path_type_update_cb(std::list<boost::shared_ptr<dfgPath> >&,
                                boost::shared_ptr<dfgPath>&,
                                std::map<boost::shared_ptr<dfgNode>, std::map<boost::shared_ptr<dfgNode>, int> >&,
                                std::set<boost::shared_ptr<dfgNode> >&); // helper for get_in_paths()
    void out_path_type_update_fast_cb(std::map<boost::shared_ptr<dfgNode>, int>&,
                                      boost::shared_ptr<dfgPath>&,
                                      std::map<boost::shared_ptr<dfgNode>, 
                                               std::map<boost::shared_ptr<dfgNode>, int> >&);
    void self_path_update_cb(std::map<boost::shared_ptr<dfgNode>, int>&,
                             boost::shared_ptr<dfgPath>&,
                             std::map<boost::shared_ptr<dfgNode>, 
                                      std::map<boost::shared_ptr<dfgNode>, int> >&,
                             unsigned int level);
    void in_path_type_update_fast_cb(std::map<boost::shared_ptr<dfgNode>, int>&,
                                     boost::shared_ptr<dfgPath>&,
                                     std::map<boost::shared_ptr<dfgNode>, 
                                              std::map<boost::shared_ptr<dfgNode>, int> >&);
  };

  class dfgEdge {
  public:

    dfgGraph* pg;               // a pointer pointing to the father Graph
    std::string name;           // edge name
    edge_descriptor id;         // edge id
    unsigned int edge_index;   // when nodes are stored in listS, vertext_descriptors are no longer
                                // integers, thereofer, separated indices must be generated and stored 
    enum edge_type_t {
      SDFG_DF             = 0x00000, // default, unknown yet
      SDFG_DDP            = 0x00001, // default data loop
      SDFG_DP             = 0x00010, // data path
      SDFG_CTL            = 0x00080, // control path
      SDFG_CLK            = 0x000a0, // clk
      SDFG_RST            = 0x000c0  // reset
    } type;

    dfgEdge() : pg(NULL), type(SDFG_DF) {}
    dfgEdge(const std::string& n, edge_type_t t = SDFG_DF) : pg(NULL), name(n), type(t) {}
    void write(pugi::xml_node&) const;
    void write(void *, ogdf::GraphAttributes *);
    bool read(const pugi::xml_node&);
    bool read(void * const, ogdf::GraphAttributes * const);

    std::list<std::pair<double, double> > bend; // bending points of the edge

  };

  class dfgPath {
  public:
    boost::shared_ptr<dfgNode> src;
    boost::shared_ptr<dfgNode> tar;
    int type;
    typedef std::pair<boost::shared_ptr<dfgNode>, int> path_type;
    std::list<path_type> path;
    std::set<boost::shared_ptr<dfgNode> > node_set; // remember the nodes in this path; to avoid combi loop

    dfgPath() : type(0) {}
    
    // add sub-paths
    void push_back(boost::shared_ptr<dfgNode>, int);
    void push_front(boost::shared_ptr<dfgNode>, int);
    void combine(boost::shared_ptr<dfgPath>);
    static int cal_type(const int&, const int&); // calculate the type

    // stream out
    std::ostream& streamout(std::ostream&) const;
  };

  inline std::ostream& operator<< (std::ostream& os, const dfgPath& p) {
    return p.streamout(os);
  }

#define DFGG_FH_F1(func_name, bconst)                     \
template<typename T>                                      \
void func_name(T d) bconst { func_name(to_id(d)); }              

#define DFGG_FH_RF1(rtype, func_name, bconst)             \
template<typename T>                                      \
rtype func_name(T d) bconst { return func_name(to_id(d)); }              
  
#define DFGG_FH_F2(func_name, bconst)                     \
template<typename T1, typename T2>                        \
void func_name(T1 d1, T2 d2) bconst { func_name(to_id(d1), to_id(d2)); }              

#define DFGG_FH_RF2(rtype, func_name, bconst)             \
template<typename T1, typename T2>                        \
rtype func_name(T1 d1, T2 d2) bconst { return func_name(to_id(d1), to_id(d2)); }              

  class dfgGraph{
  public:
    GType bg_;                  // BGL graph
    dfgNode* father;            // father when it is a entity of another module
    std::string name;           // description of this node
    
    std::map<edge_descriptor, boost::shared_ptr<dfgEdge> > edges;
    std::map<vertex_descriptor, boost::shared_ptr<dfgNode> > nodes;

    std::map<std::string, vertex_descriptor> node_map;
    std::map<unsigned int, vertex_descriptor> index_map;
    std::map<unsigned int, edge_descriptor> edge_map;

    dfgGraph() : father(NULL) {}
    dfgGraph(const std::string& n) : father(NULL), name(n) {}
    
    // add nodes and edges
    void add_node(boost::shared_ptr<dfgNode>);
    boost::shared_ptr<dfgNode> add_node(const std::string&, dfgNode::node_type_t);
    template<typename T1, typename T2>
    void add_edge(boost::shared_ptr<dfgEdge> pedge, T1 n1, T2 n2) { 
      add_edge(pedge, to_id(n1), to_id(n2));
    }
    void add_edge(boost::shared_ptr<dfgEdge>, vertex_descriptor, vertex_descriptor);
    boost::shared_ptr<dfgEdge> add_edge(const std::string&, dfgEdge::edge_type_t, vertex_descriptor, vertex_descriptor);
    template<typename T1, typename T2>
    boost::shared_ptr<dfgEdge> add_edge(const std::string& ename, dfgEdge::edge_type_t etype, T1 n1, T2 n2) {
      return add_edge(ename, etype, to_id(n1), to_id(n2));
    }

    // remove nodes and edges
    DFGG_FH_F1(remove_node,);
    void remove_node(vertex_descriptor);
    DFGG_FH_F2(remove_edge,);    // !! remove all edge between n1 and n2
    void remove_edge(vertex_descriptor, vertex_descriptor);
    template<typename T1, typename T2>
    void remove_edge(T1 n1, T2 n2, dfgEdge::edge_type_t etype) {
      remove_edge(to_id(n1), to_id(n2), etype);
    }
    void remove_edge(vertex_descriptor, vertex_descriptor, dfgEdge::edge_type_t);
    DFGG_FH_F1(remove_edge,); 
    void remove_edge(edge_descriptor);
    void remove_port(const std::string&);
    
    // other modifications
    std::list<boost::shared_ptr<dfgNode> > flatten() const;             // move all internal nodes to upper layer

    // get nodes and edges
    DFGG_FH_RF1(boost::shared_ptr<dfgEdge>, get_edge, const);
    boost::shared_ptr<dfgEdge> get_edge(edge_descriptor) const;
    DFGG_FH_RF2(boost::shared_ptr<dfgEdge>, get_edge, const);   // return a random one if multiple
    boost::shared_ptr<dfgEdge> get_edge(vertex_descriptor, vertex_descriptor) const;   // return a random one if multiple
    template<typename T1, typename T2>
    boost::shared_ptr<dfgEdge> get_edge(T1 n1, T2 n2, dfgEdge::edge_type_t etype) const {
      return get_edge(to_id(n1), to_id(n2), etype);
    }  
    boost::shared_ptr<dfgEdge> get_edge(vertex_descriptor, vertex_descriptor, dfgEdge::edge_type_t) const;
    DFGG_FH_RF1(boost::shared_ptr<dfgNode>, get_node, const);
    boost::shared_ptr<dfgNode> get_node(vertex_descriptor) const;
    DFGG_FH_RF1(boost::shared_ptr<dfgNode>, get_source, const);
    boost::shared_ptr<dfgNode> get_source(edge_descriptor) const;
    DFGG_FH_RF1(boost::shared_ptr<dfgNode>, get_source_cb, const);
    boost::shared_ptr<dfgNode> get_source_cb(edge_descriptor) const;
    DFGG_FH_RF1(boost::shared_ptr<dfgNode>, get_target, const);
    boost::shared_ptr<dfgNode> get_target(edge_descriptor) const;
    DFGG_FH_RF1(std::list<boost::shared_ptr<dfgNode> >, get_target_cb, const);
    std::list<boost::shared_ptr<dfgNode> > get_target_cb(edge_descriptor) const;
    vertex_descriptor get_source_id(const edge_descriptor&) const;
    vertex_descriptor get_target_id(const edge_descriptor&) const;

    // hierarchical search
    boost::shared_ptr<dfgNode> search_node(const std::string&) const;

    // existance check
    DFGG_FH_RF2(bool, exist, const);  //edge
    bool exist(vertex_descriptor, vertex_descriptor) const; // edge 
    template<typename T1, typename T2>
    bool exist(T1 n1, T2 n2, dfgEdge::edge_type_t etype) const {
        return exist(to_id(n1), to_id(n2), etype);
    }
    bool exist(vertex_descriptor, vertex_descriptor, dfgEdge::edge_type_t) const; // edge 
    DFGG_FH_RF1(bool, exist, const);
    bool exist(vertex_descriptor) const;
    bool exist(edge_descriptor) const;

    // traverse
    DFGG_FH_RF1(unsigned int, size_out_edges, const);
    DFGG_FH_RF1(unsigned int, size_out_edges_cb, const);
    unsigned int size_out_edges(vertex_descriptor) const;
    unsigned int size_out_edges_cb(vertex_descriptor) const;
    DFGG_FH_RF1(unsigned int, size_in_edges, const);
    DFGG_FH_RF1(unsigned int, size_in_edges_cb, const);
    unsigned int size_in_edges(vertex_descriptor) const;
    unsigned int size_in_edges_cb(vertex_descriptor) const;
    DFGG_FH_RF1(std::list<boost::shared_ptr<dfgNode> >, get_out_nodes, const);
    DFGG_FH_RF1(std::list<boost::shared_ptr<dfgNode> >, get_out_nodes_cb, const);
    std::list<boost::shared_ptr<dfgNode> > get_out_nodes(vertex_descriptor) const;
    std::list<boost::shared_ptr<dfgNode> > get_out_nodes_cb(vertex_descriptor) const;
    DFGG_FH_RF1(std::list<boost::shared_ptr<dfgNode> >, get_in_nodes, const);
    DFGG_FH_RF1(std::list<boost::shared_ptr<dfgNode> >, get_in_nodes_cb, const);
    std::list<boost::shared_ptr<dfgNode> > get_in_nodes(vertex_descriptor) const;
    std::list<boost::shared_ptr<dfgNode> > get_in_nodes_cb(vertex_descriptor) const;
    DFGG_FH_RF1(std::list<boost::shared_ptr<dfgEdge> >, get_out_edges, const);
    DFGG_FH_RF1(std::list<boost::shared_ptr<dfgEdge> >, get_out_edges_cb, const);
    std::list<boost::shared_ptr<dfgEdge> > get_out_edges(vertex_descriptor) const;
    std::list<boost::shared_ptr<dfgEdge> > get_out_edges_cb(vertex_descriptor) const;
    DFGG_FH_RF1(std::list<boost::shared_ptr<dfgEdge> >, get_in_edges, const);
    DFGG_FH_RF1(std::list<boost::shared_ptr<dfgEdge> >, get_in_edges_cb, const);
    std::list<boost::shared_ptr<dfgEdge> > get_in_edges(vertex_descriptor) const;
    std::list<boost::shared_ptr<dfgEdge> > get_in_edges_cb(vertex_descriptor) const;

    // graphic property
    unsigned int size_of_nodes() const;     // number of nodes in this graph
    unsigned int size_of_regs() const;      // number of FFs and Latchs
    unsigned int size_of_combs() const;     // number of ports, combi, and deafult nodes
    unsigned int size_of_modules() const;   // number of sub-modules 

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
    boost::shared_ptr<dfgGraph> get_datapath() const; // extract all datapaths from an SDFG
    boost::shared_ptr<dfgGraph> get_hier_RRG() const; // get a hierarchical RRG from any SDFG
    boost::shared_ptr<dfgGraph> get_RRG() const; // extract the register relation graph from a signal level DFG
    boost::shared_ptr<dfgGraph> build_reg_graph(const std::set<boost::shared_ptr<dfgNode> >& ) const; // build up a reg connection graph for certain registers 
    std::set<boost::shared_ptr<dfgNode> > get_fsm_groups(bool, boost::shared_ptr<dfgGraph>) const; // extract fsms from RRG and DFG
    void fsm_simplify();  // simplify the FSM connection graph

    // other
    std::string get_full_name() const;
    
  private:
    // convert types to id
    vertex_descriptor to_id(const std::string&) const; 
    vertex_descriptor to_id(boost::shared_ptr<dfgNode>) const;
    vertex_descriptor to_id(const vertex_descriptor&) const; 
    edge_descriptor to_id(boost::shared_ptr<dfgEdge>) const;
    edge_descriptor to_id(const edge_descriptor&) const;
    boost::shared_ptr<dfgNode> fsm_simplify_node(boost::shared_ptr<dfgNode>);  // simply the connection for a single FSM register
    std::list<boost::shared_ptr<dfgNode> > get_list_of_nodes(unsigned int) const; // get a list of nodes of certain types
    std::list<boost::shared_ptr<dfgNode> > get_list_of_nodes(unsigned int, const dfgGraph&) const; // get a list of nodes of certain types

  };

#undef DFGG_FH_F1  
#undef DFGG_FH_RF1
#undef DFGG_FH_F2  
#undef DFGG_FH_RF2

  boost::shared_ptr<dfgGraph> read(std::istream&);
}

#endif /* _SDFG_H_ */
