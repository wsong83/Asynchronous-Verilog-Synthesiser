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
    boost::shared_ptr<dfgNode> flatten() const;   // move this node to one module higher, not sure how this work, do not use it
    std::string get_hier_name() const;            // get the hierarchical name of the name
    std::string get_full_name() const;            // get the hierarchical name of the name
    void set_hier_name(const std::string&);       // set the hierarchical name
    void remove_port_sig(const std::string&, int); // remove a certain port signal
    void add_port_sig(const std::string&, const std::string&); // add a certain port connection
    std::list<boost::shared_ptr<dfgPath> >& get_out_paths_cb(); // return all output paths from this register/port
    std::list<boost::shared_ptr<dfgPath> >& get_in_paths_cb(); // return all input paths to this register/port
    // return all output paths from this register/port, faster algorithm
    std::list<boost::shared_ptr<dfgPath> > get_out_paths_fast_cb(); 
    // return all output paths from this register/port, inside the current module, faster algorithm
    std::list<boost::shared_ptr<dfgPath> > get_out_paths_fast_im(); 
    // return all output paths inside this module
    std::list<boost::shared_ptr<dfgPath> > get_out_paths_fast();
    // return all input paths to this register/port, fast algorithm
    std::list<boost::shared_ptr<dfgPath> > get_in_paths_fast_cb(); 
    // return all input paths from this register/port, inside the current module, faster algorithm
    std::list<boost::shared_ptr<dfgPath> > get_in_paths_fast_im(); 
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
    void out_path_type_update_fast(std::map<boost::shared_ptr<dfgNode>, int>&,
                                   boost::shared_ptr<dfgPath>&,
                                   std::map<boost::shared_ptr<dfgNode>, 
                                            std::map<boost::shared_ptr<dfgNode>, int> >&);
    void out_path_type_update_fast_cb(std::map<boost::shared_ptr<dfgNode>, int>&,
                                      boost::shared_ptr<dfgPath>&,
                                      std::map<boost::shared_ptr<dfgNode>, 
                                               std::map<boost::shared_ptr<dfgNode>, int> >&);
    void out_path_type_update_fast_im(std::map<boost::shared_ptr<dfgNode>, int>&,
                                      boost::shared_ptr<dfgPath>&,
                                      std::map<boost::shared_ptr<dfgNode>, 
                                               std::map<boost::shared_ptr<dfgNode>, int> >&,
                                      unsigned int);
    void self_path_update_cb(std::map<boost::shared_ptr<dfgNode>, int>&,
                             boost::shared_ptr<dfgPath>&,
                             std::map<boost::shared_ptr<dfgNode>, 
                                      std::map<boost::shared_ptr<dfgNode>, int> >&,
                             unsigned int);
    void in_path_type_update_fast_cb(std::map<boost::shared_ptr<dfgNode>, int>&,
                                     boost::shared_ptr<dfgPath>&,
                                     std::map<boost::shared_ptr<dfgNode>, 
                                              std::map<boost::shared_ptr<dfgNode>, int> >&);
    void in_path_type_update_fast_im(std::map<boost::shared_ptr<dfgNode>, int>&,
                                     boost::shared_ptr<dfgPath>&,
                                     std::map<boost::shared_ptr<dfgNode>, 
                                              std::map<boost::shared_ptr<dfgNode>, int> >&,
                                     unsigned int);
  };
}

#endif