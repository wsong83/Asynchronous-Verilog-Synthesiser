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
 * A path in the SDFG library
 * 17/09/2012   Wei Song
 *
 *
 */

#ifndef _SDFG_PATH_H_
#define _SDFG_PATH_H_

#include <boost/shared_ptr.hpp>
#include <string>
#include <map>
#include <list>
#include "dfg_common.hpp"

// the Synchronous Data-Flow Graph (SDFG) library
namespace SDFG {
  
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
    static int cal_type(int, int); // calculate the type

    // stream out
    std::ostream& streamout(std::ostream&) const;
    
  private:
    static std::string get_stype(int);
    static int cal_type_data(int, int);
    static int cal_type_control(int, int);
    static int cal_type_data2control(int, int);
    
  };

  inline std::ostream& operator<< (std::ostream& os, const dfgPath& p) {
    return p.streamout(os);
  }

}

#endif
