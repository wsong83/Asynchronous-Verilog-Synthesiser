/*
 * Copyright (c) 2013-2014 Wei Song <songw@cs.man.ac.uk> 
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
 * A function call in an expression
 * 01/03/2013   Wei Song
 *
 *
 */

#include "component.h"
#include "shell/env.h"
#include <algorithm>
#include <boost/foreach.hpp>
#include "sdfg/rtree.hpp"

using namespace netlist;
using std::ostream;
using std::endl;
using std::string;
using std::vector;
using boost::shared_ptr;
using std::list;
using std::pair;
using std::map;
using shell::location;

netlist::FuncCall::FuncCall()
  : NetComp(tFuncCall), valuable(false) {}

netlist::FuncCall::FuncCall(const location& mloc, const FIdentifier& mfn, const list<shared_ptr<Expression> >& margs)
  : NetComp(tFuncCall, mloc), fname(mfn), args(margs), valuable(false) {} 

void netlist::FuncCall::set_father(Block *pf) {
  if(father == pf) return;
  father = pf;
  fname.set_father(pf);
  BOOST_FOREACH(shared_ptr<Expression> arg, args)
    arg->set_father(pf);
}

ostream& netlist::FuncCall::streamout(ostream& os, unsigned int indent) const {
  os << string(indent, ' ');
  os << fname.get_name() << "(";
  int i = 0;
  int s = args.size() - 1;
  BOOST_FOREACH(shared_ptr<Expression> arg, args) {
    arg->streamout(os, 0);
    if(i++ < s) os << ", "; 
  }
  os << ")";
  return os;
}

FuncCall* netlist::FuncCall::deep_copy(NetComp* bp) const {
  FuncCall *rv;
  if(!bp) rv = new FuncCall();
  else    rv = static_cast<FuncCall *>(bp); // C++ does not support multiple dispatch
  NetComp::deep_copy(rv);
  rv->fname = fname;
  rv->valuable = valuable;
  BOOST_FOREACH(shared_ptr<Expression> arg, args) 
    rv->args.push_back(shared_ptr<Expression>(arg->deep_copy(NULL)));
  return rv;
}

void netlist::FuncCall::db_register(int iod) {
  BOOST_FOREACH(shared_ptr<Expression> arg, args) 
    arg->db_register(iod);
}


void netlist::FuncCall::db_expunge() {
  BOOST_FOREACH(shared_ptr<Expression> arg, args) 
    arg->db_expunge();
}

void netlist::FuncCall::reduce() {
  valuable = true;
  BOOST_FOREACH(shared_ptr<Expression> exp, args) {
    exp->reduce();
    valuable &= exp->is_valuable();
  }
}

bool netlist::FuncCall::is_valuable() const {
  return valuable;
}

Number netlist::FuncCall::get_value() const {
  // find out the function
  // execute the fucntion
  // return the value
  assert(0 == "right now get the value of a function call is not available!");
  return Number(0);
}

void netlist::FuncCall::replace_variable(const VIdentifier& var, const Number& num) {
  BOOST_FOREACH(shared_ptr<Expression> exp, args) {
    exp->replace_variable(var, num);
  }
}

void netlist::FuncCall::replace_variable(const VIdentifier& var, const VIdentifier& nvar) {
  BOOST_FOREACH(shared_ptr<Expression> exp, args) {
    exp->replace_variable(var, nvar);
  }
}

SDFG::RTree netlist::FuncCall::get_rtree() const {
  SDFG::RTree rv;
  BOOST_FOREACH(shared_ptr<Expression> exp, args) {
    rv.combine(exp->get_rtree());
  }
  return rv;
}

unsigned int netlist::FuncCall::get_width() const {
  assert(0 == "get_width() of function_call is not implemented yet!");
  return 0;
}
