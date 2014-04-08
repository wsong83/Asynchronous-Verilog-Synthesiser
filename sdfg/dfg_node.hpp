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

#ifndef _SDFG_NODE_H_
#define _SDFG_NODE_H_

#include <boost/shared_ptr.hpp>
#include <string>
#include <map>
#include <list>
#include "dfg_common.hpp"

// the Synchronous Data-Flow Graph (SDFG) library
namespace SDFG {
  
  class dfgNode {
  public:

    std::set<boost::shared_ptr<netlist::NetComp> > ptr;   // pointer to the netlist components
    boost::shared_ptr<dfgGraph> child;         // when it is a module entity, it should has a child
    std::string child_name;                    // when it is a module entity, this is the module name of the module
    std::map<std::string, std::set<std::string> > sig2port;   // remember the port connection if it is a module entity
    typedef std::pair<const std::string, std::set<std::string> > sig2port_type;
    std::map<std::string, std::string> port2sig;        // remember the port connection if it is a module entity
    typedef std::pair<const std::string, std::string> port2sig_type;
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

    enum fsm_type_t {           // FSM type
      SDFG_FSM_NONE       = 0x00000, // not a fsm
      SDFG_FSM_FSM        = 0x00001, // state machine
      SDFG_FSM_CNT        = 0x00002, // counter used as FSM
      SDFG_FSM_ADR        = 0x00004, // counter used as address
      SDFG_FSM_FLAG       = 0x00008, // control flag
      SDFG_FSM_OTHER      = 0x00100  // probably false-active
    };

    enum datapath_type_t {      // node type in control/data path division
      SDFG_DP_NONE        = 0x00000, // unknown yet
      SDFG_DP_DATA        = 0x00010, // elements on data paths
      SDFG_DP_CTL         = 0x00100, // elements on control paths
      SDFG_DP_FSM         = 0x00300  // elements on control paths
    } dp_type;

    // only available in register graph
    std::list<boost::shared_ptr<dfgPath> > opath, ipath; // record all output/input paths to avoid recalculation
    std::map<boost::shared_ptr<dfgNode>, int> opath_f, ipath_f, self_f; // record all output/input paths get from fast algorithm to avoid recalculation

    // toggle rate
    bool   is_annotated;
    double toggle_min;          // minimal toggle rate, in unit of MHz
    double toggle_max;          // maximal toggle rate, in unit of MHz
    double toggle_rate_min;     // minimal raltive toggle rate compared with its clock driver
    double toggle_rate_max;     // maximal raltive toggle rate compared with its clock driver
    

    dfgNode(): pg(NULL), node_index(0), type(SDFG_DF), dp_type(SDFG_DP_NONE), is_annotated(false), position(0,0), bbox(0,0) {}
    dfgNode(const std::string& n, node_type_t t = SDFG_DF) : 
      pg(NULL), name(n), node_index(0), type(t), dp_type(SDFG_DP_NONE), is_annotated(false), position(0,0), bbox(0,0) {}
    dfgNode* copy() const;      // copy content, not deep copy, orphan node generation
    void write(pugi::xml_node&, std::list<boost::shared_ptr<dfgGraph> >&) const;
    void write(void *, ogdf::GraphAttributes *);
    bool read(const pugi::xml_node&);
    bool read(void * const, ogdf::GraphAttributes * const);

    unsigned int size_out_edges(bool bself = true) const;
    unsigned int size_out_edges_cb(bool bself = true) const; // cb: cross-boundar
    unsigned int size_in_edges(bool bself = true) const; 
    unsigned int size_in_edges_cb(bool bself = true) const; // cb: cross-boundar
    std::list<boost::shared_ptr<dfgNode> > get_out_nodes(bool bself = true) const;
    std::list<boost::shared_ptr<dfgNode> > get_out_nodes_cb(bool bself = true) const;
    std::list<boost::shared_ptr<dfgNode> > get_in_nodes(bool bself = true) const;
    std::list<boost::shared_ptr<dfgNode> > get_in_nodes_cb(bool bself = true) const;
    std::list<boost::shared_ptr<dfgEdge> > get_out_edges(bool bself = true) const;
    std::list<boost::shared_ptr<dfgEdge> > get_out_edges_cb(bool bself = true) const;
    std::list<boost::shared_ptr<dfgEdge> > get_in_edges(bool bself = true) const;
    std::list<boost::shared_ptr<dfgEdge> > get_in_edges_cb(bool bself = true) const;
    int get_out_edges_type(bool bself = true) const;
    int get_out_edges_type_cb(bool bself = true) const;
    int get_in_edges_type(bool bself = true) const;
    int get_in_edges_type_cb(bool bself = true) const;

    boost::shared_ptr<dfgNode> flatten() const;   // move this node to one module higher, not sure how this work, do not use it
    std::string get_hier_name() const;            // get the hierarchical name of the name
    std::string get_full_name() const;            // get the hierarchical name of the name
    void set_hier_name(const std::string&);       // set the hierarchical name
    void remove_port_sig(const std::string&, int); // remove a certain port signal
    void add_port_sig(const std::string&, const std::string&); // add a certain port connection
    void remap_ports();                           // remove the port maps when a new sub-figure is linked
    void set_new_child(boost::shared_ptr<dfgGraph>);  // set a new child

    std::list<boost::shared_ptr<dfgPath> >& get_out_paths_cb(); // return all output paths from this register/port
    std::list<boost::shared_ptr<dfgPath> >& get_in_paths_cb(); // return all input paths to this register/port
    // return all output paths from this register/port, faster algorithm
    std::list<boost::shared_ptr<dfgPath> > get_out_paths_fast_cb(); 
    // return all output paths from this register/port, inside the current module, faster algorithm
    std::list<boost::shared_ptr<dfgPath> > get_out_paths_fast_im(); 
    // return all output paths from this register/port, inside certain top module, faster algorithm
    std::list<boost::shared_ptr<dfgPath> > get_out_paths_fast_in(dfgGraph*);
    // return all output paths inside this module
    std::list<boost::shared_ptr<dfgPath> > get_out_paths_fast();
    // return all input paths to this register/port, fast algorithm
    std::list<boost::shared_ptr<dfgPath> > get_in_paths_fast_cb(); 
    // return all input paths inside this module
    std::list<boost::shared_ptr<dfgPath> > get_in_paths_fast();
    // return all input paths from this register/port, inside the current module, faster algorithm
    std::list<boost::shared_ptr<dfgPath> > get_in_paths_fast_im(); 
    // return all input paths from this register/port, inside certain top module, faster algorithm
    std::list<boost::shared_ptr<dfgPath> > get_in_paths_fast_in(dfgGraph*);
    // return all self control paths that inside this module
    std::list<boost::shared_ptr<dfgPath> > get_self_path_cb();

    std::pair<double, double> position; // graphic position
    std::pair<double, double> bbox;     // bounding box
    void graphic_init();                // set initial graphic info.

    // hierarchy
    bool belong_to(dfgGraph*) const;
    boost::shared_ptr<dfgGraph> get_connected_module(boost::shared_ptr<dfgNode>) const;

    // remove open and static ports
    bool remove_useless_ports();

    // check functionalities
    bool is_const();            // is const or const after reset
    int is_fsm() const;
    boost::shared_ptr<dfgNode> get_synonym(dfgGraph *) const;
    std::string get_fsm_type() const;
    static std::string get_fsm_type(int);

    // for debug usage
    std::ostream& streamout(std::ostream&) const;
    bool check_integrity() const; // check whether there is any illegal connections

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
    void self_path_update_cb(std::map<boost::shared_ptr<dfgNode>, int>&,
                             boost::shared_ptr<dfgPath>&,
                             std::map<boost::shared_ptr<dfgNode>, 
                             std::map<boost::shared_ptr<dfgNode>, int> >&,
                             unsigned int);
    std::map<boost::shared_ptr<dfgNode>, int>
    path_search_base_fast(bool,      // cross boundary 
                          dfgGraph*, // top module
                          bool       // input 0, or output 1
                          );

    void path_update_fast(std::map<boost::shared_ptr<dfgNode>, int>&, // path map
                          boost::shared_ptr<dfgPath>,                 // main path
                          std::map<boost::shared_ptr<dfgNode>, 
                          std::map<boost::shared_ptr<dfgNode>, int > >&, // path cache
                          bool, dfgGraph*, bool);

    void update_search_map(std::map<boost::shared_ptr<dfgNode>, int>&, // target map
                           bool, dfgGraph*, bool);
  };

  inline std::ostream& operator<< (std::ostream& os, const dfgNode& node) {
    return node.streamout(os);
  }
    
}

#endif
