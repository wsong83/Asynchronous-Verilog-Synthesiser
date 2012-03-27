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
 * General block definition
 * A block may be embedded in another one.
 * 23/03/2012   Wei Song
 *
 *
 */

#include "component.h"

using namespace netlist;

void netlist::SeqBlock::clear() {
  named = false;
  statements.clear();
  sensitive = false;
  slist_pulse.clear();
  slist_level.clear();
}

ostream& netlist::SeqBlock::streamout(ostream& os, unsigned int indent) const {
  return streamout(os, indent, false);
}

ostream& netlist::SeqBlock::streamout(ostream& os, unsigned int indent, bool fl_prefix) const {
  if(!fl_prefix) os << string(indent, ' ');
  if(sensitive) {
    os << "@(";
    if(slist_pulse.size() > 0) {
      list<pair<bool, Expression> >::const_iterator it, end;
      it = slist_pulse.begin();
      while(true) {
        if(it->first)
          os << "posedge " << it->second;
        else
          os << "negedge " << it->second;
        it++;
        if(it != end) os << " or ";
        else break;
      }
    } else {
      list<Expression>::const_iterator it, end;
      it = slist_level.begin();
      while(true) {
        os << *it;
        it++;
        if(it != end) os << " or ";
        else break;
      }
    }
    os << ")";
  }
  if(statements.size() == 0)
    os << ";" << endl;
  else if(statements.size() == 1) {
    if(statements.front().get_type == NetComp::tSeqBlock) {
      ///////////////////////////////////
    }
  }
  
  return os;
      
}

bool netlist::SeqBlock::add_assignment(Assign& dd) {
  statements.push_back(dd);
  return true;
}

bool netlist::SeqBlock::add_case(Expression& exp, list<CaseItem>& citems, CaseItem& ditem) {
  statements.push_back(CaseState(exp, citems, ditem));
  return true;
}

bool netlist::SeqBlock::add_case(Expression& exp, list<CaseItem>& citems) {
  statements.push_back(CaseState(exp, citems));
  return true;
}

bool netlist::SeqBlock::add_case(Expression& exp, CaseItem& ditem) {
  statements.push_back(CaseState(exp, ditem));
  return true;
}

bool netlist::SeqBlock::add_if(Expression& exp, SeqBlock& ifcase, SeqBlock& elsecase) {
  statements.push_back(IfState(exp, ifcase, elsecase));
  return true;
}

bool netlist::SeqBlock::add_while(Expression& exp, SeqBlock& body) {
  statements.push_back(WhileState(exp, body));
  return true;
}

bool netlist::SeqBlock::add_for(Assign& init, Expression& cond, Assign& incr, SeqBlock& body) {
  statements.push_back(ForState(init, cond, incr, body));
  return true;
}

bool netlist::SeqBlock::add_seq_block(list<pair<int, Expression> >& slist, SeqBlock& body) {
  list<pair<int, Expression> >::iterator it, end;
  for(it=slist.begin(), end=slist.end(); it!=end; it++) {
    if(it->first > 0)
      slist_pulse.push_back(pair<bool, Expression>(true, it->second));
    else if(it->first < 0)
      slist_pulse.push_back(pair<bool, Expression>(false, it->second));
    else
      slist_level.push_back(it->second);
  }
  
  sensitive = true; 
  named = body.named;
  if(named) name = body.name;
  statements = body.statements;
  
  if(slist_pulse.empty() || slist_level.empty() == false)
    return false;
  else
    return true;
}

bool netlist::SeqBlock::add_block(SeqBlock& body) {
  statements.push_back(body);
  return true;
}

bool netlist::SeqBlock::add_statements(SeqBlock& body) {
  if(body.is_named()) {
    statements.push_back(body);
  } else {
    statements.splice(statements.end(), body.statements);
  }
  return true;
}

      
