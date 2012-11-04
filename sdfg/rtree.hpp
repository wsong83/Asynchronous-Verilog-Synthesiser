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
 * relation tree and forest needed in SDFG generation
 * 02/11/2012   Wei Song
 *
 *
 */

#ifndef _SDFG_RTREE_H_
#define _SDFG_RTREE_H_

#include <list>
#include <string>
#include <utility>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace SDFG {

  // forward declaration
  class RTee;
  class RForest;

  class RTree : public boost::enable_shared_from_this<RTree> {
  public:
    enum node_type_t {
      RT_DF                  = 0x00001, // default, unknown type
      RT_DATA                = 0x00002, // data
      RT_CTL                 = 0x00004, // control
      RT_TARGET              = 0x80000  // root
    } type;

    std::list<boost::shared_ptr<RTree> > child; // child nodes
    std::set<std::string> sig;  // signals in this node
    
    // constructor
    RTree();
    RTree(node_type_t);

    //helpers
    boost::shared_ptr<RTree> get_sp();
  };

  class RForest {
  public:
    std::map<std::string, boost::shared_ptr<RTree> > tree;

    // constructore
    RForest(bool d_init = false);
  };

}

#endif
