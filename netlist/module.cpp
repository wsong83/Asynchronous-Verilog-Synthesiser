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
 * Definition of netlist components.
 * 15/02/2012   Wei Song
 *
 *
 */

#include "component.h"

using namespace netlist;

netlist::Module::Module(const MIdentifier& nm, const shared_ptr<Block>& body)
  : Block(*body), name(nm) {
  named=true; 
  elab_inparse();
}

netlist::Module::Module(const location& lloc, const MIdentifier& nm, const shared_ptr<Block>& body)
  : Block(*body), name(nm) {
  named=true; 
  loc = lloc;
  elab_inparse();
}

netlist::Module::Module(const MIdentifier& nm, const list<PoIdentifier>& port_list, const shared_ptr<Block>& body)
  : Block(*body), name(nm) {
  
  named=true;
  init_port_list(port_list);
  elab_inparse();
}

netlist::Module::Module(const location& lloc, const MIdentifier& nm, const list<PoIdentifier>& port_list, const shared_ptr<Block>& body)
  : Block(*body), name(nm) {
  
  named=true;
  loc = lloc;
  init_port_list(port_list);
  elab_inparse();
}

netlist::Module::Module(const MIdentifier& nm, const list<shared_ptr<Variable> >& para_list,
                        const list<PoIdentifier>& port_list, const shared_ptr<Block>& body)
  : Block(*body), name(nm) {
  
  named=true;
  init_param_list(para_list);
  init_port_list(port_list);
  elab_inparse();
}

netlist::Module::Module(const location& lloc, const MIdentifier& nm, 
                        const list<shared_ptr<Variable> >& para_list,
                        const list<PoIdentifier>& port_list, const shared_ptr<Block>& body)
  : Block(*body), name(nm) {
  
  named = true;
  loc = lloc;
  init_param_list(para_list);
  init_port_list(port_list);
  elab_inparse();
}


ostream& netlist::Module::streamout(ostream& os, unsigned int indent) const {
  os << string(indent, ' ') << name;
  if(db_port.empty()) os << ";" << endl;
  else {
    os << "(";
    list<pair<PoIdentifier, shared_ptr<Port> > >::const_iterator it, end;
    it = db_port.begin_order();
    end = db_port.end_order();
    while(it != end){
      os << it->second->name.name;
      it++;
      if(it != end)
        os << ", ";
      else
        os << ");" << endl;
    }
  }

  // parameters
  if(db_param.size() > 0) {
    os << endl;
    db_param.streamout(os, indent+2);
  }
  // ports
  if(db_port.size() > 0) {
    os << endl;
    db_port.streamout(os, indent+2);
  }
  // variables
  if(db_var.size() > 0) {
    os << endl;
    db_var.streamout(os, indent+2);
  }
  // generate variables
  if(db_genvar.size() > 0) {
    os << endl;
    db_genvar.streamout(os, indent+2);
  }

  // statements
  ctype_t mt = tUnkown;
  list<shared_ptr<NetComp> >::const_iterator it, end;
  for(it=statements.begin(), end=statements.end(); it!=end; it++) {
    ctype_t mt_nxt = (*it)->get_type();
    if(mt != mt_nxt || mt != tAssign) { 
      os << endl; mt = mt_nxt; 
    }
    (*it)->streamout(os, indent+2);
  }

  os << endl << string(indent, ' ') << "endmodule" << endl << endl;
  return os;
}


VIdentifier& netlist::Module::new_VId() {
  while(db_var.find(unnamed_var).use_count() +
        db_param.find(unnamed_var).use_count() +
        db_genvar.find(unnamed_var).use_count() != 0)
    ++unnamed_var;
  return unnamed_var;
}

BIdentifier& netlist::Module::new_BId() {
  while(db_seqblock.find(unnamed_block).use_count() +
        db_assign.find(unnamed_block).use_count() +
        db_genblock.find(unnamed_block).use_count() != 0)
    ++unnamed_block;
  return unnamed_block;
}

/* find a variable in current block*/
shared_ptr<Variable> netlist::Module::find_var(const VIdentifier& key) const {
  shared_ptr<Variable>     rv = db_param.find(key);
  if(rv.use_count() == 0)  rv = db_genvar.find(key);
  if(rv.use_count() == 0)  rv = db_var.find(key);
  return rv;
}

/* find a variable in the global environment, up to the module level */
shared_ptr<Variable> netlist::Module::gfind_var(const VIdentifier& key) const {
  return find_var(key);         // for a module, it is the highest level
}

shared_ptr<Block> netlist::Module::find_block(const BIdentifier& key) const {
  shared_ptr<Block>        rv = db_seqblock.find(key);
  if(rv.use_count() == 0)  rv = db_genblock.find(key);
  return rv;
}

shared_ptr<NetComp> netlist::Module::find_item(const BIdentifier& key) const {
  shared_ptr<NetComp>      rv = db_seqblock.find(key);
  if(rv.use_count() == 0)  rv = db_genblock.find(key);
  if(rv.use_count() == 0)  rv = db_assign.find(key);
  return rv;
}

void netlist::Module::elab_inparse() {
  list<shared_ptr<NetComp> >::iterator it, end;
  for(it=statements.begin(), end=statements.end(); it!=end; it++) {
    if(elab_inparse_item(*it)) {
      // the item should be removed
      it = statements.erase(it);
      it--;
      end = statements.end();
    }
  }

  blocked = true;               // module is always blocked
  
  if(statements.size() == 0)
    G_ENV->error(loc, "SYN-MODULE-2", name.name);


  // set the father pointers
  set_father();
  check_inparse();
}

bool netlist::Module::check_inparse() {
  bool rv = true;
  rv &= Block::check_inparse();

  // macros defined in database.h
  DATABASE_CHECK_INPARSE_ORDER_FUN(db_port, PoIdentifier, Port, rv);
  DATABASE_CHECK_INPARSE_ORDER_FUN(db_param, VIdentifier, Variable, rv);
  DATABASE_CHECK_INPARSE_ORDER_FUN(db_genvar, VIdentifier, Variable, rv);
  DATABASE_CHECK_INPARSE_FUN(db_seqblock, BIdentifier, SeqBlock, rv);
  DATABASE_CHECK_INPARSE_FUN(db_assign, BIdentifier, Assign, rv);
  DATABASE_CHECK_INPARSE_FUN(db_genblock, BIdentifier, GenBlock, rv);

  return rv;
}

void netlist::Module::set_father() {
  // macros defined in database.h
  DATABASE_SET_FATHER_ORDER_FUN(db_port, PoIdentifier, Port, this);
  DATABASE_SET_FATHER_ORDER_FUN(db_param, VIdentifier, Variable, this);
  DATABASE_SET_FATHER_ORDER_FUN(db_genvar, VIdentifier, Variable, this);
  DATABASE_SET_FATHER_FUN(db_seqblock, BIdentifier, SeqBlock, this);
  DATABASE_SET_FATHER_FUN(db_assign, BIdentifier, Assign, this);
  DATABASE_SET_FATHER_FUN(db_genblock, BIdentifier, GenBlock, this);
  Block::set_father();
}

void netlist::Module::init_port_list(const list<PoIdentifier>& port_list) {
  list<PoIdentifier>::const_iterator it, end;
  for(it=port_list.begin(),end=port_list.end(); it!=end; it++) {
    shared_ptr<Port> m = db_port.swap(*it, shared_ptr<Port>(new Port(it->loc, *it)));
    if(m.use_count() != 0) {
      // duplicated declaration
      G_ENV->error(it->loc, "SYN-PORT-1", it->name, toString(m->loc));
    }
  }
}  

void netlist::Module::init_param_list(const list<shared_ptr<Variable> >& para_list) {
  list<shared_ptr<Variable> >::const_iterator it, end;
  for(it=para_list.begin(),end=para_list.end(); it!=end; it++) {
    shared_ptr<Variable> m = db_param.swap((*it)->name, *it);  
    if(m.use_count() != 0) {
      // duplicated declaration
      G_ENV->error((*it)->loc, "SYN-PARA-1", m->name.name, toString(m->loc));
    }
  }    
}

bool netlist::Module::elab_inparse_item(const shared_ptr<NetComp>& it) {
  // return true when this item should be removed from the statement list
  
  switch(it->get_type()) {
  case tAssign: {
    SP_CAST(m, Assign, it);
    m->set_name(new_BId());
    db_assign.insert(m->name, m);
    return false;
  }
  case tSeqBlock: {
    SP_CAST(m, SeqBlock, it);
    if(!m->is_named()) {
      m->set_default_name(new_BId());
      db_seqblock.insert(m->name, m);
    } else {
      shared_ptr<NetComp> item = find_item(m->name);
      if(item.use_count() != 0) { // name conflicts
        shared_ptr<Block> blk = find_block(m->name);
        if(blk.use_count() != 0 && blk->is_named()) { // conflict with a named block
          G_ENV->error(m->loc, "SYN-BLOCK-0", m->name.name, toString(blk->loc));
          // rename and insert
          while(find_item(++(m->name)).use_count() != 0) {}
          db_seqblock.insert(m->name, m);
        } else {              // conflict with a unnamed block
          //fatch the item
          switch(item->get_type()) {
          case tSeqBlock: 
            item = db_seqblock.swap(m->name, m); 
            break;
          case tGenBlock: 
            item = db_genblock.fetch(m->name); 
            db_seqblock.insert(m->name, m); 
            break;
          default: 
            item = db_assign.fetch(m->name); 
            db_seqblock.insert(m->name, m); 
            break;
          }
          // reinsert the unnamed item
          elab_inparse_item(item);
        }
      } else { // no name conflicts
        db_seqblock.insert(m->name, m); 
      }
    }
    return false;
  }
  case tGenBlock: {
    SP_CAST(m, GenBlock, it);
    if(!m->is_named()) {
      m->set_default_name(new_BId());
      db_genblock.insert(m->name, m);
    } else {
      shared_ptr<NetComp> item = find_item(m->name);
      if(item.use_count() != 0) { // name conflicts
        shared_ptr<Block> blk = find_block(m->name);
        if(blk.use_count() != 0 && blk->is_named()) { // conflict with a named block
          G_ENV->error(m->loc, "SYN-BLOCK-0", m->name.name, toString(blk->loc));
          // rename and insert
          while(find_item(++(m->name)).use_count() != 0) {}
          db_genblock.insert(m->name, m);
        } else {              // conflict with a unnamed block
          //fatch the item
          switch(item->get_type()) {
          case tSeqBlock: 
            item = db_seqblock.fetch(m->name); 
            db_genblock.insert(m->name, m); 
            break;
          case tGenBlock: 
            item = db_genblock.swap(m->name, m); 
            break;
          default: 
            item = db_assign.fetch(m->name); 
            db_genblock.insert(m->name, m); 
            break;
          }
          // reinsert the unnamed item
          elab_inparse_item(item);
        }
      } else { // no name conflicts
        db_genblock.insert(m->name, m); 
      }
    }
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
  case tPort: {
    SP_CAST(m, Port, it);
    shared_ptr<Port> mm = db_port.find(m->name);
    if(mm.use_count() != 0) {
      db_port.swap(m->name, m);
    } else {
      G_ENV->error(m->loc, "SYN-PORT-0", toString(m), name.name);
      db_port.insert(m->name, m);
    }
    return true;
  }
  case tVariable: {
    SP_CAST(m, Variable, it);
    shared_ptr<Variable> mm = find_var(m->name);
    if(mm.use_count() != 0) {
      G_ENV->error(m->loc, "SYN-VAR-1", m->name.name, toString(mm->loc));
    } else {
      switch(m->get_vtype()) {
      case Variable::TWire:
      case Variable::TReg: {
        db_var.insert(m->name, m);
        break;
      }
      case Variable::TParam: {
        /// if multiple definitions exist for the same parameter, the last one take effect
        db_param.insert(m->name, m); 
        break;
      }
      case Variable::TGenvar: {
        db_genvar.insert(m->name, m);
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
