/*
 * Copyright (c) 2012-2014 Wei Song <songw@cs.man.ac.uk> 
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
 * A node in the SDFG library
 * 17/09/2012   Wei Song
 *
 *
 */

#ifndef _SDFG_GRAPH_H_
#define _SDFG_GRAPH_H_

#include <boost/shared_ptr.hpp>
#include <string>
#include <map>
#include <list>
#include "dfg_common.hpp"

// forward declaration
namespace shell {
  class Env;
}

// the Synchronous Data-Flow Graph (SDFG) library
namespace SDFG {

#define DFGG_FH_F1(func_name, bconst)                     \
template<typename T>                                      \
void func_name(T d) bconst { func_name(to_id(d)); }              

#define DFGG_FH_RF1(rtype, func_name, bconst)             \
template<typename T>                                      \
rtype func_name(T d) bconst { return func_name(to_id(d)); }              
  
#define DFGG_FH_F1B(func_name, bconst)                    \
template<typename T>                                      \
 void func_name(T d, bool bself = true) bconst {          \
  func_name(to_id(d), bself); }              

#define DFGG_FH_RF1B(rtype, func_name, bconst)            \
template<typename T>                                      \
rtype func_name(T d, bool bself = true) bconst {          \
  return func_name(to_id(d), bself); }              

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
    netlist::Module* pModule;   // a pointer to link back to the netlist Module represented by this DFG
    
    std::map<edge_descriptor, boost::shared_ptr<dfgEdge> > edges;
    typedef std::pair<const edge_descriptor, boost::shared_ptr<dfgEdge> > edges_type;
    std::map<vertex_descriptor, boost::shared_ptr<dfgNode> > nodes;
    typedef std::pair<const vertex_descriptor, boost::shared_ptr<dfgNode> > nodes_type;

    typedef std::set<boost::shared_ptr<dfgNode> > named_nodes_type;
    std::map<std::string, named_nodes_type> node_map;
    typedef std::pair<const std::string, named_nodes_type> node_map_type;
    std::map<unsigned int, vertex_descriptor> index_map;
    typedef std::pair<const unsigned int, vertex_descriptor> index_map_type;
    std::map<unsigned int, edge_descriptor> edge_map;
    typedef std::pair<const unsigned int, edge_descriptor> edge_map_type;

    dfgGraph() : father(NULL), pModule(NULL) {}
    dfgGraph(const std::string& n) : father(NULL), name(n), pModule(NULL) {}

    // copy
    dfgGraph* deep_copy() const;
    
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
    void add_edge_multi(const std::string&, int, vertex_descriptor, vertex_descriptor);
    template<typename T1, typename T2>
    void add_edge_multi(const std::string& ename, int etype, T1 n1, T2 n2) {
      add_edge_multi(ename, etype, to_id(n1), to_id(n2));
    }    
    void add_path(boost::shared_ptr<dfgPath>);

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
    void remove_port(const dfgNode&);
    
    // other modifications
    std::list<boost::shared_ptr<dfgNode> > flatten() const;             // move all internal nodes to upper layer, not sure how it works, do not use it

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
    std::set<boost::shared_ptr<dfgNode> > get_node(const std::string&) const; // get a group of node whose ranges cover the asked node
    boost::shared_ptr<dfgNode> get_node(std::pair<std::string, dfgRange>) const; // get a specific node with range
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
    vertex_descriptor get_source_id_cb(const edge_descriptor&) const;
    vertex_descriptor get_target_id(const edge_descriptor&) const;

    // hierarchical search
    std::set<boost::shared_ptr<dfgNode> > search_node(const std::string&) const;

    // clear up the graph
    void remove_useless_nodes();
    void remove_unlisted_nodes(const std::set<boost::shared_ptr<dfgNode> >&, bool);

    // existance check
    DFGG_FH_RF2(bool, exist, const);  //edge
    bool exist(vertex_descriptor, vertex_descriptor) const; // edge 
    template<typename T1, typename T2>
    bool exist(T1 n1, T2 n2, dfgEdge::edge_type_t etype) const {
        return exist(to_id(n1), to_id(n2), etype);
    }
    bool exist(vertex_descriptor, vertex_descriptor, dfgEdge::edge_type_t) const; // edge 
    bool exist(std::pair<std::string, dfgRange>) const; // check a specific name with a range
    bool exist(const std::string&) const;
    bool exist(boost::shared_ptr<dfgNode>) const;
    bool exist(vertex_descriptor) const;
    bool exist(edge_descriptor) const;

    // traverse
    DFGG_FH_RF1B(unsigned int, size_out_edges, const);
    DFGG_FH_RF1B(unsigned int, size_out_edges_cb, const);
    unsigned int size_out_edges(vertex_descriptor, bool bself = true) const;
    unsigned int size_out_edges_cb(vertex_descriptor, bool bself = true) const;
    DFGG_FH_RF1B(unsigned int, size_in_edges, const);
    DFGG_FH_RF1B(unsigned int, size_in_edges_cb, const);
    unsigned int size_in_edges(vertex_descriptor, bool bself = true) const;
    unsigned int size_in_edges_cb(vertex_descriptor, bool bself = true) const;
    DFGG_FH_RF1B(std::list<boost::shared_ptr<dfgNode> >, get_out_nodes, const);
    DFGG_FH_RF1B(std::list<boost::shared_ptr<dfgNode> >, get_out_nodes_cb, const);
    std::list<boost::shared_ptr<dfgNode> > get_out_nodes(vertex_descriptor, bool bself = true) const;
    std::list<boost::shared_ptr<dfgNode> > get_out_nodes_cb(vertex_descriptor, bool bself = true) const;
    DFGG_FH_RF1B(std::list<boost::shared_ptr<dfgNode> >, get_in_nodes, const);
    DFGG_FH_RF1B(std::list<boost::shared_ptr<dfgNode> >, get_in_nodes_cb, const);
    std::list<boost::shared_ptr<dfgNode> > get_in_nodes(vertex_descriptor, bool bself = true) const;
    std::list<boost::shared_ptr<dfgNode> > get_in_nodes_cb(vertex_descriptor, bool bself = true) const;
    DFGG_FH_RF1B(std::list<boost::shared_ptr<dfgEdge> >, get_out_edges, const);
    DFGG_FH_RF1B(std::list<boost::shared_ptr<dfgEdge> >, get_out_edges_cb, const);
    std::list<boost::shared_ptr<dfgEdge> > get_out_edges(vertex_descriptor, bool bself = true) const;
    std::list<boost::shared_ptr<dfgEdge> > get_out_edges_cb(vertex_descriptor, bool bself = true) const;
    DFGG_FH_RF1B(std::list<boost::shared_ptr<dfgEdge> >, get_in_edges, const);
    DFGG_FH_RF1B(std::list<boost::shared_ptr<dfgEdge> >, get_in_edges_cb, const);
    std::list<boost::shared_ptr<dfgEdge> > get_in_edges(vertex_descriptor, bool bself = true) const;
    std::list<boost::shared_ptr<dfgEdge> > get_in_edges_cb(vertex_descriptor, bool bself = true) const;
    DFGG_FH_RF1B(int, get_out_edges_type, const);
    int get_out_edges_type(vertex_descriptor, bool bself = true) const;
    DFGG_FH_RF1B(int, get_out_edges_type_cb, const);
    int get_out_edges_type_cb(vertex_descriptor, bool bself = true) const;
    DFGG_FH_RF1B(int, get_in_edges_type, const);
    int get_in_edges_type(vertex_descriptor, bool bself = true) const;
    DFGG_FH_RF1B(int, get_in_edges_type_cb, const);
    int get_in_edges_type_cb(vertex_descriptor, bool bself = true) const;

    // graphic property
    unsigned int size_of_nodes(bool hier = false) const;     // number of nodes in this graph
    unsigned int size_of_regs(bool hier = false) const;      // number of FFs and Latchs
    unsigned int size_of_combs(bool hier = false) const;     // number of ports, combi, and deafult nodes
    unsigned int size_of_modules(bool hier = false) const;   // number of sub-modules 

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
    void edge_type_propagate();                       // propagating the edge types
    boost::shared_ptr<dfgGraph> extract_datapath_new(bool, bool, bool) const; // extract datapath form outputs
    boost::shared_ptr<dfgGraph> get_hier_RRG(bool hier = true) const; // get a hierarchical RRG from any SDFG
    boost::shared_ptr<dfgGraph> get_RRG() const; // extract the register relation graph from a signal level DFG
    boost::shared_ptr<dfgGraph> build_reg_graph(const std::set<boost::shared_ptr<dfgNode> >& ) const; // build up a reg connection graph for certain registers 
    std::set<boost::shared_ptr<dfgNode> > get_fsms(
                                                   bool verbose, 
                                                   boost::shared_ptr<dfgGraph> RRG,
                                                   unsigned int& num_of_nodes,
                                                   unsigned int& num_of_regs,
                                                   unsigned int& num_of_p_fsms
                                                   ) const; // extract fsms from RRG and DFG

    std::map<boost::shared_ptr<dfgNode>, int > get_fsms_new() const; // extract fsms from RRG and DFG
    void fsm_simplify();  // simplify the FSM connection graph
    void annotate_toggle(shell::Env *, netlist::Module*); // annotate the switching activities in Module into the SDFG
    void annotate_rate();                                 // calculate the relative rate of each node

    void connect_partial_nodes(); // connect the nodes with partial ranges with the full range nodes by assigna arcs
    void correct_conections();    // correct the error of redundent connection node for FFs
    
    // other
    std::string get_full_name() const;
    bool check_integrity() const; // check whether there is any illegal connections or mismatches in various maps
    
    std::list<boost::shared_ptr<dfgNode> > get_list_of_nodes(unsigned int, bool strict = false) const; // get a list of nodes of certain types
    std::list<boost::shared_ptr<dfgNode> > get_list_of_nodes(unsigned int, const dfgGraph&, bool strict = false) const; // get a list of nodes of certain types

  private:
    // convert types to id
    vertex_descriptor to_id(const std::string&) const; 
    vertex_descriptor to_id(boost::shared_ptr<dfgNode>) const;
    vertex_descriptor to_id(const vertex_descriptor&) const; 
    edge_descriptor to_id(boost::shared_ptr<dfgEdge>) const;
    edge_descriptor to_id(const edge_descriptor&) const;
    boost::shared_ptr<dfgNode> fsm_simplify_node(boost::shared_ptr<dfgNode>);  // simply the connection for a single FSM register
    boost::shared_ptr<dfgNode> copy_a_node(boost::shared_ptr<dfgGraph>, boost::shared_ptr<dfgNode>, bool use_full_name = false) const; // copy a node from this graph to a new graph and return the pointer of the new node
    void edge_type_propagate_combi(boost::shared_ptr<dfgNode>, 
                                   std::list<boost::shared_ptr<dfgNode> >&,
                                   std::set<boost::shared_ptr<dfgNode> >&,
                                   std::set<boost::shared_ptr<dfgNode> >&
                                   );
    void edge_type_propagate_reg(boost::shared_ptr<dfgNode>, 
                                 std::list<boost::shared_ptr<dfgNode> >&,
                                 std::set<boost::shared_ptr<dfgNode> >&,
                                 std::set<boost::shared_ptr<dfgNode> >&
                                 );
    void remove_control_nodes(bool hier = true);
    void remove_disconnected_nodes();
  };

#undef DFGG_FH_F1  
#undef DFGG_FH_RF1
#undef DFGG_FH_F1B  
#undef DFGG_FH_RF1B
#undef DFGG_FH_F2  
#undef DFGG_FH_RF2

  boost::shared_ptr<dfgGraph> read(std::istream&);

}

#endif
