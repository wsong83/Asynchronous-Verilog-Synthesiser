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

bool netlist::Block::add(const shared_ptr<NetComp>& dd) {
  statements.push_back(dd);
  return true;
}

bool netlist::Block::add_case(const shared_ptr<Expression>& exp, const list<shared_ptr<CaseItem> >& citems, const shared_ptr<CaseItem>& ditem) {
  statements.push_back(shared_ptr<CaseState>( new CaseState(exp, citems, ditem)));
  return true;
}

bool netlist::Block::add_case(const location& lloc, const shared_ptr<Expression>& exp, const list<shared_ptr<CaseItem> >& citems, const shared_ptr<CaseItem>& ditem) {
  statements.push_back(shared_ptr<CaseState>( new CaseState(lloc, exp, citems, ditem)));
  return true;
}

bool netlist::Block::add_case(const shared_ptr<Expression>& exp, const list<shared_ptr<CaseItem> >& citems) {
  statements.push_back(shared_ptr<CaseState>( new CaseState(exp, citems)));
  return true;
}

bool netlist::Block::add_case(const location& lloc, const shared_ptr<Expression>& exp, const list<shared_ptr<CaseItem> >& citems) {
  statements.push_back(shared_ptr<CaseState>( new CaseState(lloc, exp, citems)));
  return true;
}

bool netlist::Block::add_case(const shared_ptr<Expression>& exp, const shared_ptr<CaseItem>& ditem) {
  statements.push_back(shared_ptr<CaseState>( new CaseState(exp, ditem)));
  return true;
}

bool netlist::Block::add_case(const location& lloc, const shared_ptr<Expression>& exp, const shared_ptr<CaseItem>& ditem) {
  statements.push_back(shared_ptr<CaseState>( new CaseState(lloc, exp, ditem)));
  return true;
}

bool netlist::Block::add_if(const shared_ptr<Expression>& exp, const shared_ptr<Block>& ifcase, const shared_ptr<Block>& elsecase) {
  statements.push_back(shared_ptr<IfState>( new IfState(exp, ifcase, elsecase)));
  return true;
}

bool netlist::Block::add_if(const location& lloc, const shared_ptr<Expression>& exp, const shared_ptr<Block>& ifcase, const shared_ptr<Block>& elsecase) {
  statements.push_back(shared_ptr<IfState>( new IfState(lloc, exp, ifcase, elsecase)));
  return true;
}

bool netlist::Block::add_if(const shared_ptr<Expression>& exp, const shared_ptr<Block>& ifcase) {
  statements.push_back(shared_ptr<IfState>( new IfState(exp, ifcase)));
  return true;
}

bool netlist::Block::add_if(const location& lloc, const shared_ptr<Expression>& exp, const shared_ptr<Block>& ifcase) {
  statements.push_back(shared_ptr<IfState>( new IfState(lloc, exp, ifcase)));
  return true;
}

bool netlist::Block::add_while(const shared_ptr<Expression>& exp, const shared_ptr<Block>& body) {
  statements.push_back(shared_ptr<WhileState>( new WhileState(exp, body)));
  return true;
}

bool netlist::Block::add_while(const location& lloc, const shared_ptr<Expression>& exp, const shared_ptr<Block>& body) {
  statements.push_back(shared_ptr<WhileState>( new WhileState(lloc, exp, body)));
  return true;
}

bool netlist::Block::add_for(const shared_ptr<Assign>& init, const shared_ptr<Expression>& cond, const shared_ptr<Assign>& incr, const shared_ptr<Block>& body) {
  statements.push_back(shared_ptr<ForState>( new ForState(init, cond, incr, body)));
  return true;
}

bool netlist::Block::add_for(const location& lloc, const shared_ptr<Assign>& init, const shared_ptr<Expression>& cond, const shared_ptr<Assign>& incr, const shared_ptr<Block>& body) {
  statements.push_back(shared_ptr<ForState>( new ForState(lloc, init, cond, incr, body)));
  return true;
}

bool netlist::Block::add_statements(const shared_ptr<Block>& body) {
  if(body->is_blocked()) {
    statements.push_back(body);
  } else {
    statements.splice(statements.end(), body->statements);
  }
  return true;
}

void netlist::Block::elab_inparse() {
  list<shared_ptr<NetComp> >::iterator it, end;
  for(it=statements.begin(), end=statements.end(); it!=end; it++) {
    switch((*it)->get_type()) {
    case tAssign: {
      SP_CAST(m, Assign, *it);
      m->set_name(new_BId());
      db_other.insert(m->name, m);
      break;
    }
    case tBlock: {
      SP_CAST(m, Block, *it);
      if(m->is_blocked()) {
        if(!m->is_named()) {
          m->set_default_name(new_BId());
          db_other.insert(m->name, m);
        } else {
          shared_ptr<NetComp> tmp = db_other.swap(m->name, m);
          if(tmp.use_count() != 0) {
            // an unnamed block has the same name as the named block, rename it and reinsert the block
            switch(tmp->get_type()) {
            case tAssign:
              {SP_CAST(mm, Assign, tmp); mm->set_name(new_BId()); db_other.insert(mm->name, mm);break;}
            case tCase:
              {SP_CAST(mm, CaseState, tmp); mm->set_name(new_BId()); db_other.insert(mm->name, mm);break;}
            case tFor:
              {SP_CAST(mm, ForState, tmp); mm->set_name(new_BId()); db_other.insert(mm->name, mm);break;}
            case tIf:
              {SP_CAST(mm, IfState, tmp); mm->set_name(new_BId()); db_other.insert(mm->name, mm);break;}
            case tWhile:
              {SP_CAST(mm, WhileState, tmp); mm->set_name(new_BId()); db_other.insert(mm->name, mm);break;}
            case tBlock:
              {SP_CAST(mm, Block, tmp); mm->set_default_name(new_BId()); db_other.insert(mm->name, mm);break;}
            default:       assert(0 == "wrong type!");
            }
          } 
        }
      } else { // not blocked, move all items in the sub-block to this level
        it = statements.erase(it);
        list<shared_ptr<NetComp> >::iterator it_m = it;
        it--;
        statements.splice(it_m, m->statements);
        end = statements.end();
      }    
      break;
    }
    case tCase: {
      SP_CAST(m, CaseState, *it);
      m->set_name(new_BId());
      db_other.insert(m->name, m);
      break;
    }
    case tFor: {
      SP_CAST(m, ForState, *it);
      m->set_name(new_BId());
      db_other.insert(m->name, m);
      break;
    }
    case tIf: {
      SP_CAST(m, IfState, *it);
      m->set_name(new_BId());
      db_other.insert(m->name, m);
      break;
    }
    case tInstance: {
      SP_CAST(m, Instance, *it);
      if(!m->is_named()) {
        G_ENV->error(m->loc, "SYN-INST-1");
        m->set_default_name(new_IId());
        db_instance.insert(m->name, m);
      } else {
        m = db_instance.swap(m->name, m);
        if(m.use_count() != 0) {
          // it is an unnamed instance
          m->set_default_name(new_IId());
          db_instance.insert(m->name, m);
        }
      }
      break;
    }
    case tWhile: {
      SP_CAST(m, WhileState, *it);
      m->set_name(new_BId());
      db_other.insert(m->name, m);
      break;
    }
    case tVariable: {
      SP_CAST(m, Variable, *it);
      switch(m->get_vtype()) {
      case Variable::TWire: {
        db_var.insert(m->name, m);
        break;
      }
      case Variable::TReg: {
        db_var.insert(m->name, m);
        break;
      }
      default:
        G_ENV->error(m->loc, "SYN-VAR-0", m->name.name);
      }
      // variable declarations are not statements
      it = statements.erase(it);
      it--;
      end = statements.end();
      break;
    }
    default:
      assert(0 == "wrong type os statement in general block!");
    }
 }

  // double check the size
  if(statements.size() + db_var.size() > 1)
    blocked = true;             // indicating multiple variable defintions (may happen when it is module or genblock)
}

BIdentifier& netlist::Block::new_BId() {
  while(db_other.find(unnamed_block).use_count() != 0)
    ++unnamed_block;
  return unnamed_block;
}

IIdentifier& netlist::Block::new_IId() {
  while(db_instance.find(unnamed_instance).use_count() != 0)
    ++unnamed_instance;
  return unnamed_instance;
}

VIdentifier& netlist::Block::new_VId() {
  while(db_var.find(unnamed_var).use_count() != 0)
    ++unnamed_var;
  return unnamed_var;
}

shared_ptr<NetComp> netlist::Block::find_item(const BIdentifier& key) const {
  return db_other.find(key);
}

shared_ptr<Instance> netlist::Block::find_instance(const IIdentifier& key) const {
  return db_instance.find(key);
}

shared_ptr<Variable> netlist::Block::find_var(const VIdentifier& key) const {
  return db_var.find(key);
}

ostream& netlist::Block::streamout(ostream& os, unsigned int indent) const {
  streamout(os, indent, false);
  return os;
}

ostream& netlist::Block::streamout(ostream& os, unsigned int indent, bool fl_prefix) const {

  if(!fl_prefix) os << string(indent, ' ');

  // the body part
  if(!blocked)  {
    if(statements.front()->get_type() == NetComp::tBlock)
      static_pointer_cast<Block>(statements.front())->streamout(os, indent, true);
    else {
      os << endl;
      statements.front()->streamout(os, indent+2);
      if(statements.front()->get_type() == NetComp::tAssign &&
         !(static_pointer_cast<Assign>(statements.front())->is_continuous()))
        os << ";" << endl;
    }
  } else {
    os << "begin";
    if(named) os << ": " << name;
    os << endl;
    
    db_var.streamout(os, indent+2);
    if(db_var.size() > 0) os << endl;

    // statements
    ctype_t mt = tUnkown;
    list<shared_ptr<NetComp> >::const_iterator it, end;
    for(it=statements.begin(), end=statements.end(); it!=end; it++) {
      ctype_t mt_nxt = (*it)->get_type();
      if(mt != mt_nxt || mt != tAssign) {
        if(mt != tUnkown) os << endl;
        mt = mt_nxt;
      } 
      (*it)->streamout(os, indent+2);
      if((*it)->get_type() == NetComp::tAssign &&
         !(static_pointer_cast<Assign>(*it)->is_continuous()))
        os << ";" << endl;
    }
    os << string(indent, ' ') << "end" << endl;
  }   
  return os;
}

bool netlist::Block::check_inparse() {
  return false;
}

void netlist::Block::set_father() {
  DATABASE_SET_FATHER_FUN(db_var, VIdentifier, Variable, this);
  DATABASE_SET_FATHER_FUN(db_instance, IIdentifier, Instance, this);
  DATABASE_SET_FATHER_FUN(db_other, BIdentifier, NetComp, this);
}

