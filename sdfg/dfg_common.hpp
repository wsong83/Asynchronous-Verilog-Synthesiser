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
 * Common definitions in the SDFG library
 * 17/09/2012   Wei Song
 *
 *
 */

#ifndef _SDFG_COMMON_H_
#define _SDFG_COMMON_H_

// pugixml library
#include "pugixml/pugixml.hpp"

// forward declaration
namespace ogdf {
  class Graph;
  class GraphAttributes;
}

// the BGL library
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>

// the Synchronous Data-Flow Graph (SDFG) library
namespace SDFG {
  
  typedef boost::adjacency_list<boost::multisetS, boost::listS, boost::bidirectionalS> GType;
  typedef boost::graph_traits<GType> GraphTraits;
  typedef typename GraphTraits::edge_descriptor edge_descriptor;
  typedef typename GraphTraits::vertex_descriptor vertex_descriptor;
  typedef typename GraphTraits::vertex_iterator vertex_iterator;
  typedef typename GraphTraits::edge_iterator edge_iterator;

  class dfgNode;
  class dfgEdge;
  class dfgPath;
  class dfgGraph;

  // local functions, get a hash value of a string
  unsigned long shash(const std::string&);
  // show the hash value of a string
  void show_hash(const std::string&);

  // format the output of double numbers
  std::string format_double(double, unsigned int);

}

// forward decalration
namespace netlist {
  class NetComp;
  class Module; 
} 

#include "dfg_range.hpp"

#endif
