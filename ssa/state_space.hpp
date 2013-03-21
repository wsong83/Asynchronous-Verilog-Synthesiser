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

#ifndef _SSA_STATE_SPACE_
#define _SSA_STATE_SPACE_

#include "state_transfer.hpp"

// namespace State Space Analysis (SSA)
namespace SSA {
  
  class StateSpace {
  public:
    typedef std::pair<boost::shared_ptr<netlist::Expression>, boost::shared_ptr<StateTransfer> > SSA_STATE_TYPE;
    
    // constructors
    StateSpace();
    StateSpace(const std::string&);

    // helpers

    std::ostream& streamout (std::ostream& os) const; 

  private:
    std::string cname;
    std::list<SSA_STATE_TYPE> sspace; // state space

  };

  inline std::ostream& operator<< ( std::ostream& os, const StateSpace& rhs) { 
    return rhs.streamout(os);
  }
}

#endif
