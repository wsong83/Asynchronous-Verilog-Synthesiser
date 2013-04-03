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
 * Record of a state transfer condition and its target state
 * 19/03/2013   Wei Song
 *
 *
 */

#ifndef _SSA_STATE_TRANSFER_
#define _SSA_STATE_TRANSFER_

#include <boost/shared_ptr.hpp>
#include <string>

// kind of pair<next_state, expression>

// namespace State Space Analysis (SSA)
namespace SSA {
  class StateTransfer {
  public:
    enum st_type_t {
      SSA_CONST,                // const state, standard FSM
      SSA_DELTA,                // delta, counter
      SSA_EXP                   // expression, non-FSM
    } type;

    // constructors
    StateTransfer();            // default constructor
    StateTransfer(const std::string&);
    StateTransfer(const std::string&, st_type_t, const netlist::Number&, 
                  const boost::shared_ptr<netlist::Expression>& expr = boost::shared_ptr<netlist::Expression>());
    StateTransfer(const std::string&, boost::shared_ptr<netlist::Expression>, 
                  const boost::shared_ptr<netlist::Expression>& expr = boost::shared_ptr<netlist::Expression>());
    
    // helpers
    void set_next_state(const netlist::Number&);
    void set_next_delta(const netlist::Number&);
    void set_next_exp(boost::shared_ptr<netlist::Expression>);
    netlist::Number get_next_state() const { return next_state; }
    netlist::Number get_next_delta() const { return next_delta; }
    boost::shared_ptr<netlist::Expression> get_next_exp() const;
    
    typedef std::map<netlist::VIdentifier, netlist::Number> SSA_ENV;
    typedef std::pair<const netlist::VIdentifier, netlist::Number> SSA_ENV_TYPE;
    std::pair<bool, netlist::Number> get_next_state(const SSA_ENV&, const netlist::Number& cstate = netlist::Number(0)) const; // calculating the next state, if not met with the condition, return false in the pair
    void append_condition(const netlist::Operation&, boost::shared_ptr<netlist::Expression>); // add more clauses in the condition
    
    std::ostream& streamout (std::ostream& os) const; 

  private:
    netlist::Number next_state;          // the next state
    netlist::Number next_delta;          // delta to calculate the next state
    boost::shared_ptr<netlist::Expression> next_exp;   // the expression to calculate the next state (normally not an FSM)
    boost::shared_ptr<netlist::Expression> condition;  // the condition when this state transfer is executed 
    std::string cname;                   // the name of the controller

  };

  inline std::ostream& operator<< ( std::ostream& os, const StateTransfer& rhs) { 
    return rhs.streamout(os);
  }

}

#endif
