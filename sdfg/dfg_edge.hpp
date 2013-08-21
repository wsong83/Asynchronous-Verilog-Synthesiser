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
 * An edge in the SDFG library
 * 17/09/2012   Wei Song
 *
 *
 */

#ifndef _SDFG_EDGE_H_
#define _SDFG_EDGE_H_

#include <boost/shared_ptr.hpp>
#include <string>
#include <map>
#include <list>
#include "dfg_common.hpp"

// the Synchronous Data-Flow Graph (SDFG) library
namespace SDFG {
  
  class dfgEdge {
  public:

    dfgGraph* pg;               // a pointer pointing to the father Graph
    std::string name;           // edge name
    edge_descriptor id;         // edge id
    unsigned int edge_index;    // when nodes are stored in listS, vertext_descriptors are no longer
                                // integers, thereofer, separated indices must be generated and stored 
    enum edge_type_t {
      SDFG_DF             = 0x00000, // default, unknown yet
      SDFG_DDP            = 0x00001, // default data loop
      SDFG_CAL            = 0x00002, // mathmatical calculation datapath
      SDFG_ASS            = 0x00004, // direct assignment
      SDFG_DAT            = 0x00008, // other type of data
      SDFG_CMP            = 0x00010, // compare
      SDFG_EQU            = 0x00020, // equal
      SDFG_LOG            = 0x00040, // logic
      SDFG_ADR            = 0x00080, // address
      SDFG_CTL            = 0x00100, // normal control
      SDFG_CLK            = 0x02000, // clk
      SDFG_RST            = 0x04000, // reset
      SDFG_DAT_MASK       = 0x0000F,
      SDFG_CTL_MASK       = 0x00FF0,
      SDFG_CR_MASK        = 0x0F000
    } type;

    dfgEdge() : pg(NULL), type(SDFG_DF) {}
    dfgEdge(const std::string& n, edge_type_t t = SDFG_DF) : pg(NULL), name(n), type(t) {}
    void write(pugi::xml_node&) const;
    void write(void *, ogdf::GraphAttributes *);
    bool read(const pugi::xml_node&);
    bool read(void * const, ogdf::GraphAttributes * const);
    void push_bend(double, double, bool relative = false);

    boost::shared_ptr<dfgNode> get_source() const
    { return pg->get_source(id); }
    boost::shared_ptr<dfgNode> get_source_cb() const
    { return pg->get_source_cb(id); }
    boost::shared_ptr<dfgNode> get_target() const
    { return pg->get_target(id); }
    std::list<boost::shared_ptr<dfgNode> > get_target_cb() const
    { return pg->get_target_cb(id); }
    vertex_descriptor get_source_id() const
    { return pg->get_source_id(id); }
    vertex_descriptor get_target_id() const
    { return pg->get_target_id(id); }

    std::list<std::pair<double, double> > bend; // bending points of the edge
    
    // for debug usage
    bool check_integrity() const; // check whether there is any illegal connections
  };

}

#endif
