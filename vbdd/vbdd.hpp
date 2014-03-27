/*
 * Copyright (c) 2014-2014 Wei Song <songw@cs.man.ac.uk> 
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
 * A variable labelled Binary Decision Diagram (BDD) lib
 * 19/03/2014   Wei Song
 *
 *
 */

#ifndef VBDD_H_
#define VBDD_H_

#include <string>
#include <map>
#include <boost/shared_ptr.hpp>

namespace vbdd {

  class BDDExp {                // BDD expression, which cannot be reduced to BDD
  };

  class BDDNode {               // a node in the BDD
  };

  class BDDLib {                // the root variable library
  public:
    static boost::shared_ptr<BDDNode> create_node(const std::string&);
  private:
    static std::map<std::string, boost::shared_ptr<BDDNode> > lib;
  };


}

#endif
