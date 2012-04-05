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
  db_wire.clear();
  db_reg.clear();
  db_instance.clear();
  db_block.clear();
  unnamed_block = BIdentifier();
  unnamed_instance = IIdentifier();
  unnamed_var = VIdentifier();
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

bool netlist::Block::add_if(const shared_ptr<Expression>& exp, const shared_ptr<Block>& ifcase, const shared_ptr<Block>& elsecase) {
  statements.push_back(shared_ptr<IfState>( new IfState(exp, ifcase, elsecase)));
  return true;
}

bool netlist::Block::add_if(const shared_ptr<Expression>& exp, const shared_ptr<Block>& ifcase) {
  statements.push_back(shared_ptr<IfState>( new IfState(exp, ifcase)));
  return true;
}

bool netlist::Block::add_while(const shared_ptr<Expression>& exp, const shared_ptr<Block>& body) {
  statements.push_back(shared_ptr<WhileState>( new WhileState(exp, body)));
  return true;
}

bool netlist::Block::add_for(const shared_ptr<Assign>& init, const shared_ptr<Expression>& cond, const shared_ptr<Assign>& incr, const shared_ptr<Block>& body) {
  statements.push_back(shared_ptr<ForState>( new ForState(init, cond, incr, body)));
  return true;
}


bool netlist::Block::add_block(const shared_ptr<Block>& body) {
  statements.push_back(body);
  return true;
}

bool netlist::Block::add_statements(const shared_ptr<Block>& body) {
  if(body->is_named() || (body->db_reg.size() != 0)) {
    statements.push_back(body);
  } else {
    statements.splice(statements.end(), body->statements);
  }
  return true;
}

BIdentifier& netlist::Block::new_BId() {
  while(db_block.find(unnamed_block).use_count() != 0)
    ++unnamed_block;
  return unnamed_block;
}

IIdentifier& netlist::Block::new_IId() {
  while(db_instance.find(unnamed_instance).use_count() != 0)
    ++unnamed_instance;
  return unnamed_instance;
}

VIdentifier& netlist::Block::new_VId() {
  while(db_wire.find(unnamed_var).use_count() +
        db_reg.find(unnamed_var).use_count() != 0)
    ++unnamed_var;
  return unnamed_var;
}

ostream& netlist::Block::streamout(ostream& os, unsigned int indent) const {
  streamout(os, indent, false);
  return os;
}

ostream& netlist::Block::streamout(ostream& os, unsigned int indent, bool fl_prefix) const {

  if(!fl_prefix) os << string(indent, ' ');

  // the body part
  if((statements.size() + 
      db_reg.size() +
      db_wire.size() +
      db_instance.size() == 1) &&
     (!named))  {
    if(statements.front()->get_type() == NetComp::tBlock)
      static_pointer_cast<Block>(statements.front())->streamout(os, indent, true);
    else {
      os << endl;

      if(statements.size() > 0) {
        statements.front()->streamout(os, indent+2);
        if(statements.front()->get_type() == NetComp::tAssign) os << ";" << endl;
      }

      if(db_wire.size() > 0)
        os << string(indent+2, ' ') << "wire" << *(db_wire.begin()->second) << ";" << endl;

      if(db_reg.size() > 0)
        os << string(indent+2, ' ') << "reg" << *(db_wire.begin()->second) << ";" << endl;

      if(db_instance.size() > 0)
        db_instance.streamout(os, indent+2);
    }
  } else {
    os << "begin";
    if(named) os << ": " << name.name;
    os << endl;
    // show local variables if any
    {
      map<VIdentifier, shared_ptr<Variable> >::const_iterator it, end;
      for(it = db_wire.begin(), end = db_wire.end(); it != end; it++)
        os << string(indent+2, ' ') << "wire " << *(it->second) << ";" << endl;
      for(it = db_reg.begin(), end = db_reg.end(); it != end; it++)
        os << string(indent+2, ' ') << "reg " << *(it->second) << ";" << endl;
      db_instance.streamout(os, indent+2);
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
  
