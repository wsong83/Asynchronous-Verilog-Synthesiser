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

bool netlist::Assign::check_inparse() {
  bool rv = true;
  rv &= lval->check_inparse();
  rv &= rexp->check_inparse();
  return rv;
}

void netlist::Assign::set_father(Block *pf) {
  if(father == pf) return;
  father = pf;
  name.set_father(pf);
  lval->set_father(pf);
  rexp->set_father(pf);
}

bool netlist::Assign::elaborate(elab_result_t &result, const ctype_t mctype, const vector<NetComp *>& fp) {
  bool rv = true;
  result = ELAB_Normal;

  if(continuous && !( mctype == tModule ||
                      mctype == tGenBlock
                      )) {
    G_ENV->error(loc, "ELAB-ASSIGN-0");
    return false;
  }

  if(!continuous && !( mctype == tSeqBlock
                       )) {
    G_ENV->error(loc, "ELAB-ASSIGN-1");
    return false;
  }

  assert(lval.use_count() != 0);
  assert(rexp.use_count() != 0);

  // check internals
  rv &= lval->elaborate(result, mctype, fp);
  rv &= rexp->elaborate(result, mctype, fp);

  return rv;
}


void netlist::Assign::set_always_pointer(SeqBlock *p) {
  if(lval.use_count() != 0) lval->set_always_pointer(p);
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
