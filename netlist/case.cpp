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
 * Case statements
 * 26/03/2012   Wei Song
 *
 *
 */

#include <algorithm>
#include "component.h"

using namespace netlist;
using std::ostream;
using std::endl;
using std::string;
using boost::shared_ptr;
using std::list;
using std::for_each;

ostream& netlist::CaseItem::streamout (ostream& os, unsigned int indent) const {
  // show the expressions
  if(is_default()) {
    os << string(indent, ' ');    // show the indent for each line
    os << "default: ";
  } else if(exps.size() == 1) {
    os << string(indent, ' ');    // show the indent for each line
    os << *(exps.front()) << ": ";
  } else {
    unsigned int ncase = exps.size(); // number of cases in the item
    list<shared_ptr<Expression> >::const_iterator it = exps.begin();
    for(unsigned int i=0; i< ncase-1; i++) {
      os << string(indent, ' ');    // show the indent for each line
      os << *(*it) << "," << endl;
      it++;
    }
    os << *(*it) << ": ";
  }
  
  // the body part
  body->streamout(os, indent, true);
  return os;
}

void netlist::CaseItem::set_father(Block *pf) {
  if(father == pf) return;
  father = pf;
  list<shared_ptr<Expression> >::iterator it, end;
  for(it=exps.begin(), end=exps.end(); it!=end; it++)
    (*it)->set_father(pf);

  if(body.use_count())
    body->set_father(pf);
}

bool netlist::CaseItem::check_inparse() {
  bool rv = true;
  list<shared_ptr<Expression> >::iterator it, end;
  for(it=exps.begin(), end=exps.end(); it!=end; it++)
    rv &= (*it)->check_inparse();

  if(body.use_count())
    rv &= body->check_inparse();

  return rv;
} 

CaseItem* netlist::CaseItem::deep_copy() const {
  CaseItem* rv = new CaseItem(loc);
  if(body.use_count() != 0) rv->body.reset(body->deep_copy());
  for_each(exps.begin(), exps.end(), [&rv](const shared_ptr<Expression>& m) {
      rv->exps.push_back(shared_ptr<Expression>(m->deep_copy()));
    });
  return rv;
}

void netlist::CaseItem::db_register(int iod) {
  for_each(exps.begin(), exps.end(), [](shared_ptr<Expression>& m) {m->db_register(1);});
  if(body.use_count() != 0) body->db_register(1);
}

void netlist::CaseItem::db_expunge() {
  for_each(exps.begin(), exps.end(), [](shared_ptr<Expression>& m) {m->db_expunge();});
  if(body.use_count() != 0) body->db_expunge();
}

void netlist::CaseItem::set_always_pointer(SeqBlock *p) {
  if(body.use_count() != 0) body->set_always_pointer(p);
}

ostream& netlist::CaseState::streamout (ostream& os, unsigned int indent) const {
  os << string(indent, ' ');
  if(casex) os << "casex(" << *exp << ")" << endl;
  else      os << "case("  << *exp << ")" << endl;
  list<shared_ptr<CaseItem> >::const_iterator it, end;
  for(it=cases.begin(), end=cases.end(); it!=end; it++) {
    (*it)->streamout(os, indent+2);
  }
  os << string(indent, ' ') << "endcase" << endl;
  return os;
}

void netlist::CaseState::set_father(Block *pf) {
  if(father == pf) return;
  father = pf;
  name.set_father(pf);
  exp->set_father(pf);
  list<shared_ptr<CaseItem> >::iterator it, end;
  for(it=cases.begin(), end=cases.end(); it!=end; it++)
    (*it)->set_father(pf);
}

bool netlist::CaseState::check_inparse() {
  bool rv = true;
  rv &= exp->check_inparse();
  list<shared_ptr<CaseItem> >::iterator it, end;
  for(it=cases.begin(), end=cases.end(); it!=end; it++)
    rv &= (*it)->check_inparse();

  return rv;
}

CaseState* netlist::CaseState::deep_copy() const {
  CaseState* rv = new CaseState(loc);
  rv->name = name;
  rv->named = named;
  rv->casex = casex;
  if(exp.use_count() != 0) rv->exp.reset(exp->deep_copy());
  for_each(cases.begin(), cases.end(), [&rv](const shared_ptr<CaseItem>& m) {
      rv->cases.push_back(shared_ptr<CaseItem>(m->deep_copy()));
    });
  return rv;
}

void netlist::CaseState::db_register(int iod) {
  for_each(cases.begin(), cases.end(), [](shared_ptr<CaseItem>& m) {m->db_register(1);});
  if(exp.use_count() != 0) exp->db_register(1);
}

void netlist::CaseState::db_expunge() {
  for_each(cases.begin(), cases.end(), [](shared_ptr<CaseItem>& m) {m->db_expunge();});
  if(exp.use_count() != 0) exp->db_expunge();
}

void netlist::CaseState::set_always_pointer(SeqBlock *p) {
  for_each(cases.begin(), cases.end(), [&p](shared_ptr<CaseItem>& m) {m->set_always_pointer(p);});
}
