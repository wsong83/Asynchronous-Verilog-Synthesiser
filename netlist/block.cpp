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
#include "shell/env.h"
#include <algorithm>
#include <boost/foreach.hpp>

using namespace netlist;
using std::ostream;
using std::endl;
using std::string;
using std::vector;
using boost::shared_ptr;
using boost::static_pointer_cast;
using std::list;
using std::pair;
using shell::location;
using std::for_each;

bool netlist::Block::add(const shared_ptr<NetComp>& dd) {
  statements.push_back(dd);
  return true;
}

bool netlist::Block::add_case(const shared_ptr<Expression>& exp, const list<shared_ptr<CaseItem> >& citems, const shared_ptr<CaseItem>& ditem, bool m_casex) {
  statements.push_back(shared_ptr<CaseState>( new CaseState(exp, citems, ditem, m_casex)));
  return true;
}

bool netlist::Block::add_case(const location& lloc, const shared_ptr<Expression>& exp, const list<shared_ptr<CaseItem> >& citems, const shared_ptr<CaseItem>& ditem, bool m_casex) {
  statements.push_back(shared_ptr<CaseState>( new CaseState(lloc, exp, citems, ditem, m_casex)));
  return true;
}

bool netlist::Block::add_case(const shared_ptr<Expression>& exp, const list<shared_ptr<CaseItem> >& citems, bool m_casex) {
  statements.push_back(shared_ptr<CaseState>( new CaseState(exp, citems, m_casex)));
  return true;
}

bool netlist::Block::add_case(const location& lloc, const shared_ptr<Expression>& exp, const list<shared_ptr<CaseItem> >& citems, bool m_casex) {
  statements.push_back(shared_ptr<CaseState>( new CaseState(lloc, exp, citems, m_casex)));
  return true;
}

bool netlist::Block::add_case(const shared_ptr<Expression>& exp, const shared_ptr<CaseItem>& ditem, bool m_casex) {
  statements.push_back(shared_ptr<CaseState>( new CaseState(exp, ditem, m_casex)));
  return true;
}

bool netlist::Block::add_case(const location& lloc, const shared_ptr<Expression>& exp, const shared_ptr<CaseItem>& ditem, bool m_casex) {
  statements.push_back(shared_ptr<CaseState>( new CaseState(lloc, exp, ditem, m_casex)));
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
    if(elab_inparse_item(*it, it)) {
      it = statements.erase(it);
      it--;
      end = statements.end();
    }
  }

  // double check the size
  if(statements.size() + db_var.size() > 1)
    blocked = true;             // indicating multiple variable defintions (may happen when it is module or genblock)

  // set the father pointers
  set_father();
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

/* find a variable in current block*/
shared_ptr<Variable> netlist::Block::find_var(const VIdentifier& key) const {
  return db_var.find(key);
}

/* find a variable in the global environment, up to the module level */
shared_ptr<Variable> netlist::Block::gfind_var(const VIdentifier& key) const {
  shared_ptr<Variable> rv = db_var.find(key);
  if(rv.use_count() == 0 && father != NULL) return father->gfind_var(key);
  return rv;
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
    ctype_t mt = tUnknown;
    BOOST_FOREACH(const shared_ptr<NetComp>& it, statements) {
      ctype_t mt_nxt = it->get_type();
      if(mt != mt_nxt || mt != tAssign) {
        if(mt != tUnknown) os << endl;
        mt = mt_nxt;
      } 
      it->streamout(os, indent+2);
      if(it->get_type() == NetComp::tAssign &&
         !(static_pointer_cast<Assign>(it)->is_continuous()))
        os << ";" << endl;
    }
    os << string(indent, ' ') << "end" << endl;
  }   
  return os;
}

Block* netlist::Block::deep_copy() const {
  Block* rv = new Block();
  rv->loc = loc;
  rv->name = name;
  rv->named = named;
  
  // data in Block
  // lambda expression, need C++0x support
  for_each(statements.begin(), statements.end(),
           [rv](const shared_ptr<NetComp>& comp) { 
             rv->statements.push_back(shared_ptr<NetComp>(comp->deep_copy())); 
           });
  
  DATABASE_DEEP_COPY_FUN(db_var,      VIdentifier, Variable,  rv->db_var       );
  rv->unnamed_block = unnamed_block;
  rv->unnamed_instance = unnamed_instance;
  rv->unnamed_var = unnamed_var;
  rv->blocked = blocked;
  
  rv->set_father();
  rv->elab_inparse();
  return rv;
}

bool netlist::Block::check_inparse() {
  bool rv = true;
  // macros defined in database.h
  DATABASE_CHECK_INPARSE_FUN(db_var, VIdentifier, Variable, rv);
  DATABASE_CHECK_INPARSE_FUN(db_instance, IIdentifier, Instance, rv);
  DATABASE_CHECK_INPARSE_FUN(db_other, BIdentifier, NetComp, rv);
  return rv;
}

void netlist::Block::set_father() {
  // macros defined in database.h
  DATABASE_SET_FATHER_FUN(db_var, VIdentifier, Variable, this);
  DATABASE_SET_FATHER_FUN(db_instance, IIdentifier, Instance, this);
  DATABASE_SET_FATHER_FUN(db_other, BIdentifier, NetComp, this);
}

void netlist::Block::db_register(int) {
  // the item in statements are duplicated in db_instance and db_other, therefore, only statements are executed
  // initialization of the variables in ablock are ignored as they are wire, reg and integers
  for_each(db_var.begin_order(), db_var.end_order(), [](pair<VIdentifier, shared_ptr<Variable> >& m) {
      m.second->db_register(1);
    });
  for_each(statements.begin(), statements.end(), [](shared_ptr<NetComp>& m) {m->db_register(1);});
}

void netlist::Block::db_expunge() {
  for_each(db_var.begin_order(), db_var.end_order(), [](pair<VIdentifier, shared_ptr<Variable> >& m) {
      m.second->db_expunge();
    });
  for_each(statements.begin(), statements.end(), [](shared_ptr<NetComp>& m) {m->db_expunge();});
}

bool netlist::Block::elaborate(elab_result_t &result, const ctype_t mctype, const vector<NetComp *>& fp) {
  bool rv = true;
  result = ELAB_Normal;

  // check the father component
  if(!(
       mctype == tGenBlock ||   // a general block can be defined in a generate block
       mctype == tSeqBlock      // a general block can be defined in a generate block
       )) {
    G_ENV->error(loc, "ELAB-BLOCK-1");
    return false;
  }
  
  // elaborate all internal items
  // check all variables
  list<pair<VIdentifier, shared_ptr<Variable> > >::iterator vit, vend;
  vit = db_var.begin_order();
  vend = db_var.end_order();
  while(vit != vend) {
    rv &= vit->second->elaborate(result, mctype, fp);
    if(result == ELAB_Empty) {
      VIdentifier tname = vit->first;
      vit++;
      db_var.erase(tname);
      vend = db_var.end_order();
      result = ELAB_Normal;
    } else
      vit++;
  }
  if(!rv) return rv;

  // elaborate the internals
  list<shared_ptr<NetComp> >::iterator cit, cend;
  cit = statements.begin();
  cend = statements.end();
  while(cit != cend) {
    rv &= (*cit)->elaborate(result, mctype, fp);
    if(rv) {
      switch(result) {          // need test
      case ELAB_Empty: {
        (*cit).reset();
        cit = statements.erase(cit);
        cend = statements.end();
        break;
      }
      case ELAB_Const_If: {     // need test
        SP_CAST(mif, IfState, *cit);
        for_each(mif->ifcase->db_var.begin_order(), mif->ifcase->db_var.end_order(),
                 [&](pair<VIdentifier, shared_ptr<Variable> >& m) {
                     bool v = db_var.insert(m.first, m.second);
                     assert(v);
                   } );
        statements.insert(cit, mif->ifcase->statements.begin(), mif->ifcase->statements.end());
        (*cit).reset();
        cit = statements.erase(cit);
        cend = statements.end();
        break;
      }
      default:
        cit++;
      }
    } else {
      cit++;
    }
  }
  if(!rv) return rv;

  // check the db_var again as some variables may be removed
  vit = db_var.begin_order();
  vend = db_var.end_order();
  while(vit != vend) {
    rv &= vit->second->elaborate(result, mctype, fp);
    if(result == ELAB_Empty) {
      VIdentifier tname = vit->first;
      vit++;
      db_var.erase(tname);
      vend = db_var.end_order();
    } else
      vit++;
  }

  // final check
  // to do what?

  return rv;
}

void netlist::Block::set_always_pointer(SeqBlock *p) {
  for_each(db_other.begin(), db_other.end(), [&](pair<const BIdentifier, shared_ptr<NetComp> >& m) {
      m.second->set_always_pointer(p);
    });
}

void netlist::Block::gen_sdfg(shared_ptr<SDFG::dfgGraph> G, 
                              std::set<string>& target,
                              std::set<string>& dsrc,
                              std::set<string>& csrc) {
  assert(db_var.empty());
  assert(db_instance.empty());

  BOOST_FOREACH(shared_ptr<NetComp>& m, statements) {
    m->gen_sdfg(G, target, dsrc, csrc);
  }

}

bool netlist::Block::elab_inparse_item(
                                       const shared_ptr<NetComp>& it,
                                       list<shared_ptr<NetComp> >::iterator& itor
                                       ) 
{
  switch(it->get_type()) {
  case tAssign: {
    SP_CAST(m, Assign, it);
    m->set_name(new_BId());
    db_other.insert(m->name, m);
    return false;
  }
  case tGenBlock:
  case tSeqBlock:
  case tBlock: {
    SP_CAST(m, Block, it);
    if(m->is_blocked()) {
      if(!m->is_named()) {
        m->set_default_name(new_BId());
        db_other.insert(m->name, m);
      } else {
        shared_ptr<NetComp> item = find_item(m->name);
        if(item.use_count()!=0) { // name conflict
          if((item->get_type() == tBlock || 
              item->get_type() == tSeqBlock || 
              item->get_type() == tGenBlock
              ) && 
             (static_pointer_cast<Block>(item))->is_named()
             ) { // really conflict with a named block
            G_ENV->error(m->loc, "SYN-BLOCK-0", m->name.name, toString(item->loc));
            while(find_item(++(m->name)).use_count() != 0) {}
            db_other.insert(m->name, m);
          } else { // conflict with an unnamed block
            item = db_other.swap(m->name, m);
            elab_inparse_item(item, itor);
          }
        } else {
          db_other.insert(m->name, m);
        }
      }
      return false;
    } else { // not blocked, move all items in the sub-block to this level
      if(itor == statements.begin()) { // the first one
        statements.splice(itor, m->statements);
        statements.erase(itor);
        itor = statements.begin();
      } else {
        list<shared_ptr<NetComp> >::iterator pre = itor;
        --pre;
        statements.splice(itor, m->statements);
        statements.erase(itor);
        itor = pre;
      }
      return false;
    }
  }
  case tCase: {
    SP_CAST(m, CaseState, it);
    m->set_name(new_BId());
    db_other.insert(m->name, m);
    return false;
  }
  case tFor: {
    SP_CAST(m, ForState, it);
    m->set_name(new_BId());
    db_other.insert(m->name, m);
    return false;
  }
  case tIf: {
    SP_CAST(m, IfState, it);
    m->set_name(new_BId());
    db_other.insert(m->name, m);
    return false;
  }
  case tInstance: {
    SP_CAST(m, Instance, it);
    if(!m->is_named()) {
      G_ENV->error(m->loc, "SYN-INST-1");
      m->set_default_name(new_IId());
      db_instance.insert(m->name, m);
    } else {
      shared_ptr<Instance> mm = db_instance.find(m->name);
      if(mm.use_count() != 0) {
        if(mm->is_named()) {
          G_ENV->error(m->loc, "SYN-INST-0", m->name.name, toString(mm->loc));
          while(db_instance.find(++(m->name)).use_count() != 0) {}
          db_instance.insert(m->name, m);
        } else {                  // conflict with an unnamed instance
          mm = db_instance.swap(m->name, m);
          elab_inparse_item(mm, itor);
        }
      } else {
        db_instance.insert(m->name, m);
      }
    }                 
    return false;
  }
  case tWhile: {
    SP_CAST(m, WhileState, it);
    m->set_name(new_BId());
    db_other.insert(m->name, m);
    return false;
  }
  case tVariable: {
    SP_CAST(m, Variable, it);
    shared_ptr<Variable> mm = find_var(m->name);
    if(mm.use_count() != 0) {
      G_ENV->error(m->loc, "SYN-VAR-1", m->name.name, toString(mm->loc));
    } else {
      switch(m->get_vtype()) {
      case Variable::TWire:
      case Variable::TReg: 
      case Variable::TParam: {
        db_var.insert(m->name, m);
        break;
      }
      default:
        G_ENV->error(m->loc, "SYN-VAR-0", m->name.name);
      }
    }
    return true;
  }
  default:
    G_ENV->error(it->loc, "SYN-MODULE-1");
    return true;
  }
}


