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
 * Record of the state space of a controller
 * 20/03/2013   Wei Song
 *
 *
 */

#include "netlist/component.h"
#include "state_space.hpp"
#include <boost/foreach.hpp>

using namespace SSA;
using namespace netlist;
using boost::shared_ptr;
using std::pair;
using std::map;
using std::string;

SSA::StateSpace::StateSpace() 
  : cname("state") {}

SSA::StateSpace::StateSpace(const string& cn) 
  : cname(cn) {}



std::ostream& SSA::StateSpace::streamout (std::ostream& os) const {
  os << cname << " : " << std::endl;
  BOOST_FOREACH(SSA_STATE_TYPE s, sspace) {
    os << "[" << *(s.first) << "] " << *(s.second) << std::endl;
  }
  os << std::endl;
  return os;
}
