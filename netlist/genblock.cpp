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
 * Generate block definition
 * A block may be embedded in another one.
 * 12/04/2012   Wei Song
 *
 *
 */

#include <algorithm>
#include "component.h"
#include "shell/env.h"

using namespace netlist;
using std::ostream;
using std::endl;
using std::string;
using boost::shared_ptr;
using boost::static_pointer_cast;
using std::list;
using std::pair;
using shell::location;
using std::for_each;

netlist::GenBlock::GenBlock(const Block& body)
  : Block(body)
{
  ctype = tGenBlock;
  elab_inparse();
}

netlist::GenBlock::GenBlock(const location& lloc, const Block& body)
  : Block(body)
{
  ctype = tGenBlock;
  loc = lloc;
  elab_inparse();
}

ostream& netlist::GenBlock::streamout(ostream& os, unsigned int indent) const {
  streamout(os, indent, false);
  return os;
}

ostream& netlist::GenBlock::streamout(ostream& os, unsigned int indent, bool fl_prefix) const {
  
  if(!fl_prefix) os << string(indent, ' ');

  os << "generate " << endl;
  
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
  }

  os << string(indent, ' ') << "endgenerate" << endl;

  return os;
}

void netlist::GenBlock::elab_inparse() {
  
  list<shared_ptr<NetComp> >::iterator it, end;
  for(it=statements.begin(), end=statements.end(); it!=end; it++) {
    if(elab_inparse_item(*it)) {
      // the item should be removed
      it = statements.erase(it);
      it--;
      end = statements.end();
    }
  }
  
  blocked = true;               // generate block is always blocked

  // set the father pointers
  set_father();
}

bool netlist::GenBlock::elab_inparse_item( const shared_ptr<NetComp>& it) {
  // return true when this item should be removed from the statement list
  
  switch(it->get_type()) {
  case tAssign: {
    SP_CAST(m, Assign, it);
    m->set_name(new_BId());
    db_other.insert(m->name, m);
    return false;
  }
  case tGenBlock:
  case tSeqBlock: {
    SP_CAST(m, Block, it);
    if(!m->is_named()) {
      m->set_default_name(new_BId());
      db_other.insert(m->name, m);
    } else {
      shared_ptr<NetComp> item = find_item(m->name);
      if(item.use_count() != 0) { // name conflicts
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
          elab_inparse_item(item);
        }
      } else {
        db_other.insert(m->name, m);
      }
    }
    return false;
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
          elab_inparse_item(mm);
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

GenBlock* netlist::GenBlock::deep_copy() const {
  GenBlock* rv = new GenBlock();
  rv->loc = loc;
  rv->name = name;
  rv->named = named;
  
  // data in Block
  // lambda expression, need C++0x support
  for_each(statements.begin(), statements.end(),
           [rv](const shared_ptr<NetComp>& comp) { 
             rv->statements.push_back(shared_ptr<NetComp>(comp->deep_copy())); 
           });
  
  DATABASE_DEEP_COPY_ORDER_FUN(db_var,      VIdentifier, Variable,  rv->db_var       );
  rv->unnamed_block = unnamed_block;
  rv->unnamed_instance = unnamed_instance;
  rv->unnamed_var = unnamed_var;
  rv->blocked = blocked;

  rv->elab_inparse();
  return rv;
}

void netlist::GenBlock::db_register(int iod) {
  // the item in statements are duplicated in db_instance and db_other, therefore, only statements are executed
  // initialization of the variables in ablock are ignored as they are wire, reg and integers
  for_each(db_var.begin_order(), db_var.end_order(), [](pair<VIdentifier, shared_ptr<Variable> >& m) {
      m.second->db_register(1);
    });
  for_each(statements.begin(), statements.end(), [](shared_ptr<NetComp>& m) {m->db_register(1);});
}

void netlist::GenBlock::db_expunge() {
  for_each(db_var.begin_order(), db_var.end_order(), [](pair<VIdentifier, shared_ptr<Variable> >& m) {
      m.second->db_expunge();
    });
  for_each(statements.begin(), statements.end(), [](shared_ptr<NetComp>& m) {m->db_expunge();});
}
