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
 * relation tree and forest needed in SDFG generation
 * 02/11/2012   Wei Song
 *
 *
 */

#ifndef _SDFG_RTREE_H_
#define _SDFG_RTREE_H_

#include <list>
#include <string>
#include <set>
#include <map>
#include <boost/shared_ptr.hpp>

// pugixml library
#include "pugixml/pugixml.hpp"

#include "dfg_edge.hpp"

namespace SDFG {

  class RTree {
  public:
    static const std::string DTarget;
    RTree(bool default_case = true);
    RTree(const std::string&, int etype = dfgEdge::SDFG_ASS);
    RTree(boost::shared_ptr<RTree>, int);
    RTree(boost::shared_ptr<RTree>, boost::shared_ptr<RTree>, int);
    RTree(boost::shared_ptr<RTree>, boost::shared_ptr<RTree>, boost::shared_ptr<RTree>);
    
    std::map<std::string, std::map<std::string, int> > tree; 
    typedef std::pair<const std::string, std::map<std::string, int> > sub_tree_type;
    typedef std::pair<const std::string, int> rtree_edge_type;


    // helpers
    RTree* add_edge(const std::string&, int etype = dfgEdge::SDFG_ASS); // add an signal to the tree
    RTree* add_edge(const std::string&, const std::string&, int etype = dfgEdge::SDFG_ASS); // add an signal to the tree
    RTree* add_tree(boost::shared_ptr<RTree>, int etype = dfgEdge::SDFG_ASS); // add a parallel tree to this tree
    RTree* add_tree(boost::shared_ptr<RTree>, const std::string&, int etype = dfgEdge::SDFG_ASS); // add a tree and set a root for the default targeted sub-tree
    RTree* combine(boost::shared_ptr<RTree>, int etype = dfgEdge::SDFG_ASS); // assign the default sub-tree to the named sub-trees of this tree

    std::set<std::string> get_control(const std::string& = "") const;
    std::set<std::string> get_data(const std::string& = "") const;
    std::set<std::string> get_all(const std::string& = "") const;
    std::set<std::string> get_signals(int, const std::string& = "") const;


    // debug
    std::ostream& streamout (std::ostream& os) const;
    
  private:
    void copy_subtree(const std::string&, const std::map<std::string, int>&, int);
  };

  inline std::ostream& operator<< ( std::ostream& os, const RTree& rhs) {
    return rhs.streamout(os);
  }

}

#endif
