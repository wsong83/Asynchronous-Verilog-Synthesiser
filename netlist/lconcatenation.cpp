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
 * left-side concatenation, only to be the target of an assignment
 * 23/02/2012   Wei Song
 *
 *
 */

#include "component.h"
#include <algorithm>
#include <boost/foreach.hpp>

using namespace netlist;
using std::ostream;
using std::string;
using std::vector;
using boost::shared_ptr;
using std::list;
using shell::location;
using std::for_each;

netlist::LConcatenation::LConcatenation(shared_ptr<Concatenation>& con)
  : NetComp(tLConcatenation, con->loc), valid(false)
{
  con->reduce();
  BOOST_FOREACH(const shared_ptr<ConElem>& it, con->data) {
    if( 0 != it->con.size() ||    // the concatenation contain sub-concatenations
        !it->exp->is_singular() ||   // the expression is still complex
        it->exp->get_op().get_type() != Operation::oVar || // wrong type
        it->exp->get_op().get_var().get_type() != tVarName) return; // wrong type
    data.push_back(it->exp->get_op().get_var());
  }
  valid = true;
}

netlist::LConcatenation::LConcatenation(const location& lloc, shared_ptr<Concatenation>& con)
  : NetComp(tLConcatenation, lloc), valid(false)
{
  con->reduce();
  BOOST_FOREACH(const shared_ptr<ConElem>& it, con->data) {
    if( 0 != it->con.size() ||    // the concatenation contain sub-concatenations
        !it->exp->is_singular() ||   // the expression ia still complex
        it->exp->get_op().get_type() != Operation::oVar || // wrong type
        it->exp->get_op().get_var().get_type() != tVarName) return; // wrong type
    data.push_back(it->exp->get_op().get_var());
  }
  valid = true;
}

netlist::LConcatenation::LConcatenation(const VIdentifier& id)
  : NetComp(tLConcatenation, id.loc), valid(true) { data.push_back(id); }

netlist::LConcatenation::LConcatenation(const location& lloc, const VIdentifier& id)
  : NetComp(tLConcatenation, lloc), valid(true) { data.push_back(id); }

void netlist::LConcatenation::set_father(Block *pf) {
  if(father == pf) return;
  father = pf;
  BOOST_FOREACH(VIdentifier& it, data) it.set_father(pf);
}

bool netlist::LConcatenation::check_inparse() {
  bool rv = true;
  BOOST_FOREACH(VIdentifier& it, data) rv &= it.check_inparse();
  return rv;
}

ostream& netlist::LConcatenation::streamout(ostream& os, unsigned int indent) const {
  assert(valid);

  os << string(indent, ' ');

  if(1 == data.size()) os << data.front();
  else {
    list<VIdentifier>::const_iterator it, end;
    it=data.begin();
    end=data.end();
    os << "{";
    while(true) {
      os << *it;
      it++;
      if(it != end)
        os << ",";
      else {
        os << "}";
        break;
      }
    }
  }
  return os;
}

LConcatenation* netlist::LConcatenation::deep_copy() const {
  LConcatenation* rv = new LConcatenation();
  rv->loc = loc;
  rv->valid = valid;
  BOOST_FOREACH(const VIdentifier& m, data) {
    VIdentifier* mp = m.deep_copy();
    rv->data.push_back(*mp);
    delete mp;
  }
  return rv;
}

void netlist::LConcatenation::db_register(int iod) {
  BOOST_FOREACH(VIdentifier& m, data) m.db_register(0);
}

void netlist::LConcatenation::db_expunge() {
  BOOST_FOREACH(VIdentifier& m, data) m.db_expunge();
}

bool netlist::LConcatenation::elaborate(elab_result_t &result, const ctype_t mctype, const vector<NetComp *>& fp) {
  bool rv = true;
  result = ELAB_Normal;

  assert(valid && data.size() > 0);

  BOOST_FOREACH(VIdentifier& m, data) 
    rv &= m.elaborate(result, tLConcatenation, fp);

  return rv;
}

void netlist::LConcatenation::set_always_pointer(SeqBlock *p) {
  BOOST_FOREACH(VIdentifier& m, data) m.set_always_pointer(p);
}

unsigned int netlist::LConcatenation::get_width() {
  if(width) return width;
  BOOST_FOREACH(VIdentifier& m, data)
    width += m.get_width();
  return width;
}

void netlist::LConcatenation::set_width(const unsigned int& w) {
  if(width == w) return;
  assert(w < get_width());
  unsigned int wm = w;
  list<VIdentifier>::reverse_iterator it, end;
  for(it=data.rbegin(), end=data.rend(); it!=end; it++) {
    if(wm >= it->get_width()) 
      wm -= it->get_width();
    else {
      if(wm == 0) break;
      else {
        it->set_width(wm);
        wm = 0;
      }
    }
  }
  if(it != end) 
    data.erase(data.begin(), it.base()); // ATTN: it is a reverse_iterator
  width = w;
}
