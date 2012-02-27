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
#include "averilog/av_token.h"

// Env is a parameter to the parser, which is not declared yet
namespace shell {
  class Env;
}
#include "averilog/averilog.hh"

using namespace netlist;

netlist::Range::Range(const mpz_class& sel)
  : c(sel), type(TConst) {  }

netlist::Range::Range(const mpz_class& rl, const mpz_class& rr)
  : cr(new Range_Const(rl,rr)), type(TCRange) {  }

netlist::Range::Range(const shared_ptr<Expression>& sel)
  : type(TVar) 
{ 
  if(sel->is_valuable()) {	// constant expression, value it now
    c = sel->get_value().get_value();
    type = TConst;
  } else {
    v = sel;
  }
 };

netlist::Range::Range(const Range_Exp& sel)
  : type(TRange) 
{  
  if(*(sel.first) == *(sel.second)) {	// only one bit is selected
    if(sel.first->is_valuable()) { // constant expression, value it now
      c = sel.first->get_value().get_value();
      type = TConst;
    } else {			// variable expression
      v = sel.first;
      type = TVar;
    }
  } else {
    if(sel.first->is_valuable() && sel.second->is_valuable()) {
      type = TCRange;
      cr.reset(new Range_Const(sel.first->get_value().get_value(), sel.second->get_value().get_value()));
    } else {
      r = sel;
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
      c = m_sel.first.get_value().get_value();
      type = TConst;
    } else {			// variable expression
      v.reset(new Expression(m_sel.first));
      type = TVar;
    } 
  } else {
    if(m_sel.first.is_valuable() && m_sel.second.is_valuable()) {
      type = TCRange;
      cr.reset(new Range_Const(m_sel.first.get_value().get_value(), m_sel.second.get_value().get_value()));
    } else {
      r.reset(new Range_Exp(m_sel));
    } 
  }
}
  
ostream& netlist::Range::streamout(ostream& os) const {
  switch(type) {
  case TConst: os << c; break;
  case TVar: os << *v; break;
  case TRange: os << r->first << ":" << r->second; break;
  case TCRange: os << cr->first << ":" << cr->second; break;
  default: // should not go here
    assert(0 == "Wrong range type");
  }
  return os;
}

Range netlist::Range::deep_copy() const {
  Range rv;
  rv.c = c;
  if(0 != this->v.use_count())
    rv.v = v.deep_copy();
  if(0 != this->r.use_count())
    rv.r =  Range_Exp(r.first->deep_copy(), r.second->deep_copy());
  if(0 != this->cr.use_count())
    rv.cr = cr;
  rv.type = type;
  return rv;
}
