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

#include "component.h"

using namespace netlist;

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
    switch((*it)->get_type()) {
    case tAssign: {
      SP_CAST(m, Assign, *it);
      m->set_name(new_BId());
      db_other.insert(m->name, m);
      break;
    }
    case tSeqBlock: {
      SP_CAST(m, SeqBlock, *it);
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
          case tSeqBlock:
            {SP_CAST(mm, SeqBlock, tmp); mm->set_default_name(new_BId()); db_other.insert(mm->name, mm);break;}
          default:       assert(0 == "wrong type!");
          }
        } 
      }         
      break;
    }
    case tGenBlock: {
      SP_CAST(m, GenBlock, *it);
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
          case tSeqBlock:
            {SP_CAST(mm, SeqBlock, tmp); mm->set_default_name(new_BId()); db_other.insert(mm->name, mm);break;}
          case tGenBlock:
            {SP_CAST(mm, SeqBlock, tmp); mm->set_default_name(new_BId()); db_other.insert(mm->name, mm);break;}
          default:       assert(0 == "wrong type!");
          }
        } 
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
        assert(0 == "wrong type of variable in general block!");
      }
      break;
    }
    default:
      assert(0 == "wrong type os statement in general block!");
    }
  }
  
  blocked = true;               // generate block is always blocked

}

