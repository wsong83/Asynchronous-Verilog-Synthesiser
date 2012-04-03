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

void netlist::Block::clear() {
  named = false;
  statements.clear();
}

bool netlist::Block::add_assignment(const shared_ptr<Assign>& dd) {
  statements.push_back(dd);
  return true;
}

bool netlist::Block::add_case(const shared_ptr<Expression>& exp, const list<shared_ptr<CaseItem> >& citems, const shared_ptr<CaseItem>& ditem) {
  statements.push_back(shared_ptr<CaseState>( new CaseState(exp, citems, ditem)));
  return true;
}

bool netlist::Block::add_case(const shared_ptr<Expression>& exp, const list<shared_ptr<CaseItem> >& citems) {
  statements.push_back(shared_ptr<CaseState>( new CaseState(exp, citems)));
  return true;
}

bool netlist::Block::add_case(const shared_ptr<Expression>& exp, const shared_ptr<CaseItem>& ditem) {
  statements.push_back(shared_ptr<CaseState>( new CaseState(exp, ditem)));
  return true;
}

bool netlist::Block::add_if(const shared_ptr<Expression>& exp, const shared_ptr<SeqBlock>& ifcase, const shared_ptr<SeqBlock>& elsecase) {
  statements.push_back(shared_ptr<IfState>( new IfState(exp, ifcase, elsecase)));
  return true;
}

bool netlist::Block::add_if(const shared_ptr<Expression>& exp, const shared_ptr<SeqBlock>& ifcase) {
  statements.push_back(shared_ptr<IfState>( new IfState(exp, ifcase)));
  return true;
}

bool netlist::Block::add_while(const shared_ptr<Expression>& exp, const shared_ptr<SeqBlock>& body) {
  statements.push_back(shared_ptr<WhileState>( new WhileState(exp, body)));
  return true;
}

bool netlist::Block::add_for(const shared_ptr<Assign>& init, const shared_ptr<Expression>& cond, const shared_ptr<Assign>& incr, const shared_ptr<SeqBlock>& body) {
  statements.push_back(shared_ptr<ForState>( new ForState(init, cond, incr, body)));
  return true;
}


bool netlist::Block::add_block(const shared_ptr<SeqBlock>& body) {
  statements.push_back(body);
  return true;
}

bool netlist::Block::add_statements(const shared_ptr<SeqBlock>& body) {
  if(body->is_named()) {
    statements.push_back(body);
  } else {
    statements.splice(statements.end(), body->statements);
  }
  return true;
}

///////////////////////////////////////////////////////////////////////

BIdentifier& netlist::GenBlock::new_BId() {
  while(db_block.find(unnamed_block).use_count() != 0)
    ++unnamed_block;
  return unnamed_block;
}

IIdentifier& netlist::GenBlock::new_IId() {
  while(db_instance.find(unnamed_instance).use_count() != 0)
    ++unnamed_instance;
  return unnamed_instance;
}

VIdentifier& netlist::GenBlock::new_VId() {
  while(db_wire.find(unnamed_var).use_count() +
        db_reg.find(unnamed_var).use_count() != 0)
    ++unnamed_var;
  return unnamed_var;
}

//////////////////////////////////////////////////////////////
void netlist::SeqBlock::clear() {
  Block::clear();
  sensitive = false;
  slist_pulse.clear();
  slist_level.clear();
}

ostream& netlist::SeqBlock::streamout(ostream& os, unsigned int indent) const {
  streamout(os, indent, false);
  return os;
}

ostream& netlist::SeqBlock::streamout(ostream& os, unsigned int indent, bool fl_prefix) const {
  
  if(!fl_prefix) os << string(indent, ' ');
  
  if(sensitive) {
    os << "@(";
    if(slist_pulse.size() > 0) {
      list<pair<bool, shared_ptr<Expression> > >::const_iterator it, end;
      it = slist_pulse.begin();
      end = slist_pulse.end();
      while(true) {
        if(it->first) {
          os << "posedge ";
          it->second->streamout(os, 0);
        } else {
          os << "negedge ";
          it->second->streamout(os, 0);
        }
        it++;
        if(it != end) os << " or ";
        else break;
      }
    } else {
      list<shared_ptr<Expression> >::const_iterator it, end;
      it = slist_level.begin();
      end = slist_level.end();
      while(true) {
        os << **it;
        it++;
        if(it != end) os << " or ";
        else break;
      }
    }
    os << ") ";
  }

  // the body part
  if((statements.size() == 0) && (db_var.size() == 0))
    os << ";" << endl;
  else if((statements.size() == 1) && (db_var.size() == 0))  {
    if(statements.front()->get_type() == NetComp::tSeqBlock)
      static_pointer_cast<SeqBlock>(statements.front())->streamout(os, indent, true);
    else {
      os << endl;
      statements.front()->streamout(os, indent+2);
      if(statements.front()->get_type() == NetComp::tAssign) os << ";" << endl;
    }
  } else {
    os << "begin" << endl;
    // show local variables if any
    {
      map<VIdentifier, shared_ptr<Variable> >::const_iterator it, end;
      for(it = db_var.begin(), end = db_var.end(); it != end; it++)
        os << string(indent+2, ' ') << "reg " << *(it->second) << ";" << endl;
    }
    // statements
    {
      list<shared_ptr<NetComp> >::const_iterator it, end;
      for(it=statements.begin(), end=statements.end(); it!=end; it++) {
        (*it)->streamout(os, indent+2);
        if((*it)->get_type() == NetComp::tAssign) os << ";" << endl;
      }
    }
    os << string(indent, ' ') << "end" << endl;
  }
  
  return os; 
}

bool netlist::SeqBlock::add_seq_block(list<pair<int, shared_ptr<Expression> > >& slist, const shared_ptr<SeqBlock>& body) {
  list<pair<int, shared_ptr<Expression> > >::iterator it, end;
  for(it=slist.begin(), end=slist.end(); it!=end; it++) {
    if(it->first > 0)
      slist_pulse.push_back(pair<bool, shared_ptr<Expression> >(true, it->second));
    else if(it->first < 0)
      slist_pulse.push_back(pair<bool, shared_ptr<Expression> >(false, it->second));
    else
      slist_level.push_back(it->second);
  }
  
  sensitive = true; 
  named = body->named;
  if(named) name = body->name;
  statements = body->statements;
  
  if(slist_pulse.empty() || slist_level.empty() == false)
    return false;
  else
    return true;
}
      
