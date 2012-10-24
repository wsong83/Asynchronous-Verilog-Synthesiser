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
 * Block/non-block assignments
 * 23/02/2012   Wei Song
 *
 *
 */

#include "component.h"
#include "shell/env.h"
#include "sdfg/sdfg.hpp"
#include <boost/foreach.hpp>

using namespace netlist;
using std::ostream;
using std::endl;
using shell::location;
using std::string;
using std::vector;
using boost::shared_ptr;


netlist::Assign::Assign( const shared_ptr<LConcatenation>& lhs,
                         const shared_ptr<Expression>& rhs,
                         bool b
                         )
  : NetComp(tAssign), lval(lhs), rexp(rhs), blocking(b), continuous(false), named(false)
{}

netlist::Assign::Assign( const location& lloc,
                         const shared_ptr<LConcatenation>& lhs,
                         const shared_ptr<Expression>& rhs,
                         bool b
                         )
  : NetComp(tAssign, lloc), lval(lhs), rexp(rhs), blocking(b), continuous(false), named(false)
{}

ostream& netlist::Assign::streamout(ostream& os, unsigned int indent) const {
  os << string(indent, ' ');
  if(continuous) os << "assign ";
  os << *lval;
  if(blocking) os << " = ";
  else os << " <= ";
  os << *rexp;
  if(continuous) os << ";" << endl;
  return os;
}

void netlist::Assign::set_father(Block *pf) {
  if(father == pf) return;
  father = pf;
  name.set_father(pf);
  lval->set_father(pf);
  rexp->set_father(pf);
}

void netlist::Assign::db_register(int) { 
  lval->db_register(0); 
  rexp->db_register(1); 
}

void netlist::Assign::db_expunge() { 
  lval->db_expunge(); 
  rexp->db_expunge(); 
}

bool netlist::Assign::elaborate(set<shared_ptr<Variable> >& to_del,
                                map<shared_ptr<NetComp>, list<shared_ptr<Variable> > >& to_add) {
  bool rv = true;

  // check internals
  rv &= lval->elaborate(to_del, to_add);
  rv &= rexp->elaborate(to_del, to_add);

  return rv;
}


void netlist::Assign::scan_vars(std::set<string>& target,
                                std::set<string>& dsrc,
                                std::set<string>& csrc,
                                bool ctl) const {
  lval->scan_vars(target, dsrc, csrc, ctl);
  rexp->scan_vars(target, dsrc, csrc, ctl);
}

Assign* netlist::Assign::deep_copy() const {
  Assign* rv = new Assign( loc,
                           shared_ptr<LConcatenation>(lval->deep_copy()),
                           shared_ptr<Expression>(rexp->deep_copy()),
                           blocking
                           );
  rv->name = name;
  rv->named = named;
  rv->continuous = continuous;
  return rv;
}

void netlist::Assign::gen_sdfg(shared_ptr<SDFG::dfgGraph> G, 
                               const std::set<string>&,
                               const std::set<string>&,
                               const std::set<string>&) {
  std::set<string> t, d, c;     // local version
  scan_vars(t, d, c, false);

  BOOST_FOREACH(const string& m, t) {
    BOOST_FOREACH(const string& sig, d) {
      if(!G->exist(sig, m, SDFG::dfgEdge::SDFG_DP)) 
        G->add_edge(sig, SDFG::dfgEdge::SDFG_DP, sig, m);
    }
    
    BOOST_FOREACH(const string& sig, c) {
      if(!G->exist(sig, m, SDFG::dfgEdge::SDFG_CTL)) 
        G->add_edge(sig, SDFG::dfgEdge::SDFG_CTL, sig, m);
    }
  }
}

void netlist::Assign::replace_variable(const VIdentifier& var, const Number& num) {
  lval->replace_variable(var, num);
  rexp->replace_variable(var, num);
}
