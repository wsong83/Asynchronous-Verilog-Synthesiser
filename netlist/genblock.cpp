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

ostream& netlist::GenBlock::streamout(ostream& os, unsigned int indent) const {
  streamout(os, indent, false);
  return os;
}

ostream& netlist::GenBlock::streamout(ostream& os, unsigned int indent, bool fl_prefix) const {
  
  if(!fl_prefix) os << string(indent, ' ');

  os << "generate " << endl;
  
  list<shared_ptr<NetComp> >::const_iterator it, end;
  for(it=statements.begin(), end=statements.end(); it!=end; it++) {
    if((*it)->get_type() == NetComp::tBlock)
      static_pointer_cast<Block>(*it)->streamout(os, indent+2);
    else {
      os << endl;
      (*it)->streamout(os, indent+2);
      if((*it)->get_type() == NetComp::tAssign &&
         !(static_pointer_cast<Assign>(*it)->is_continuous()))
        os << ";" << endl;
    }
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
      if(!m->is_named()) m->name = new_BId();
      db_other.insert(m->name, m);
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
      if(!m->is_named()) m->set_name(new_IId());
      db_instance.insert(m->name, m);
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

