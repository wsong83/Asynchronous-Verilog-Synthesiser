/*
 * Copyright (c) 2011 Wei Song <songw@cs.man.ac.uk> 
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
 * Range declaration and definition
 * 08/02/2011   Wei Song
 *
 *
 */

#include <cassert>
#include "range.h"
#include "averilog/src/averilog_util.h"

using namespace netlist;

netlist::Range::Range(int sel)
  : c(sel), type(TConst) {  }

netlist::Range::Range(const Expression& sel)
  : type(TVar) 
{ 
  if(sel.is_valuable()) {	// constant expression, value it now
    c = sel.get_value();
    type = TConst;
  } else {
    v.reset(new Expression(sel));
  }
 };

netlist::Range::Range(const Range_Exp& sel)
  : type(TRange) 
{  
  if(sel.first == sel.second) {	// only one bit is selected
    if(sel.first.is_valuable()) { // constant expression, value it now
      c = sel.first.get_value();
      type = TConst;
    } else {			// variable expression
      v.reset(new Expression(sel.first));
      type = TVar;
    }
  }   
};

netlist::Range::Range(const Range_Exp& sel, int ctype)
  : type(TRange)
{
  Range_Exp m_sel;
  if(ctype == averilog::av_parser::token::oPColon) { // positive colon
    m_sel.first = sel.first+sel.second;
    m_sel.second = sel.first;
  } else if(ctype == averilog::av_parser::token::oNColon){ // negtive colon
    m_sel.first = sel.first;
    m_sel.second = sel.first - sel.second;
  } else {
    // error
    assert(1 == 0);
  }

  if(m_sel.first == m_sel.second) {	// only one bit is selected
    if(m_sel.first.is_valuable()) { // constant expression, value it now
      c = m_sel.first.get_value();
      type = TConst;
    } else {			// variable expression
      v.reset(new Expression(m_sel.first));
      type = TVar;
    } 
  } else {
    r.reset(new Range_Exp(m_sel));
  }
}

bool netlist::Range::is_valuable() {
  return type == TConst;
}

bool netlist::Range::is_single() {
  return type != TRange;
}
  
std::ostream& netlist::Range::streamout(std::ostream& os) const {
  return os;
}

