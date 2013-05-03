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

}

#endif