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
 * Range declaration and definition
 * 08/02/2012   Wei Song
 *
 *
 */

#include "component.h"

using namespace netlist;

netlist::Range::Range(const mpz_class& sel)
  : c(sel),  dim(false), type(TConst) {  }

netlist::Range::Range(const mpz_class& rl, const mpz_class& rr)
  : cr(rl,rr), dim(false), type(TCRange) {  }

netlist::Range::Range(const Expression& sel)
  : dim(false), type(TVar) 
{ 
  Expression m = sel;
  m.reduce();
  if(m.is_valuable()) {	// constant expression, value it now
    c = m.get_value().get_value();
    type = TConst;
  } else {
    v = m;
  }
 };

netlist::Range::Range(const Range_Exp& sel, bool dimm)
  : dim(dimm), type(TRange) 
{ 
  Range_Exp m = sel;
  m.first.reduce();
  m.second.reduce();
  if(m.first == m.second) {	// only one bit is selected
    if(m.first.is_valuable()) { // constant expression, value it now
      c = m.first.get_value().get_value();
      type = TConst;
    } else {			// variable expression
      v = m.first;
      type = TVar;
    }
  } else {
    if(m.first.is_valuable() && m.second.is_valuable()) {
      type = TCRange;
      cr.first = m.first.get_value().get_value();
      cr.second = m.second.get_value().get_value();
    } else {
      r = m;
    } 
  }
};

netlist::Range::Range(const Range_Exp& sel, int ctype)
  : type(TRange)
{
  Range_Exp m_sel;
  if(ctype == 1) { // positive colon
    m_sel.first = sel.first+sel.second;
    m_sel.second = sel.first;
  } else if(ctype == -1){ // negtive colon
    m_sel.first = sel.first;
    m_sel.second = sel.first - sel.second;
  } else {
    // error
    assert(1 == 0);
  }

  if(m_sel.first == m_sel.second) {	// only one bit is selected
    if(m_sel.first.is_valuable()) { // constant expression, value it now
      c = m_sel.first.get_value().get_value();
      type = TConst;
    } else {			// variable expression
      v = m_sel.first;
      type = TVar;
    } 
  } else {
    if(m_sel.first.is_valuable() && m_sel.second.is_valuable()) {
      type = TCRange;
      cr.first = m_sel.first.get_value().get_value();
      cr.second = m_sel.second.get_value().get_value();
    } else {
      r = m_sel;
    } 
  }
}

void netlist::Range::db_register() {
  v.db_register(1);
  r.first.db_register(1);
  r.second.db_register(1);
}
  
ostream& netlist::Range::streamout(ostream& os, unsigned int indent) const {
  os << string(indent, ' ');
  switch(type) {
  case TConst: os << c; break;
  case TVar: os << v; break;
  case TRange: os << r.first << ":" << r.second; break;
  case TCRange: os << cr.first << ":" << cr.second; break;
  default: // should not go here
    assert(0 == "Wrong range type");
  }
  return os;
}
