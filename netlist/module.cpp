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
  
  // insert ports
  list<PoIdentifier>::const_iterator it, end;
  for(it=port_list.begin(),end=port_list.end(); it!=end; it++)
    db_port.insert(*it, shared_ptr<Port>(new Port(it->loc, *it)));

  elab_inparse();
}

netlist::Module::Module(const location& lloc, const MIdentifier& nm, const list<PoIdentifier>& port_list, const shared_ptr<Block>& body)
  : Block(*body), name(nm) {
  
  named=true;
  loc = lloc;
  
  // insert ports
  list<PoIdentifier>::const_iterator it, end;
  for(it=port_list.begin(),end=port_list.end(); it!=end; it++)
    db_port.insert(*it, shared_ptr<Port>(new Port(it->loc, *it)));

  elab_inparse();
}

netlist::Module::Module(const MIdentifier& nm, const list<shared_ptr<Variable> >& para_list,
                        const list<PoIdentifier>& port_list, const shared_ptr<Block>& body)
  : Block(*body), name(nm) {
  
  named=true;
  
  // insert parameters
  {
    list<shared_ptr<Variable> >::const_iterator it, end;
    for(it=para_list.begin(),end=para_list.end(); it!=end; it++)
      db_param.insert((*it)->name, *it);  
  }

  // insert ports
  {
    list<PoIdentifier>::const_iterator it, end;
    for(it=port_list.begin(),end=port_list.end(); it!=end; it++)
      db_port.insert(*it, shared_ptr<Port>(new Port(it->loc, *it)));
  }

  elab_inparse();
}

netlist::Module::Module(const location& lloc, const MIdentifier& nm, 
                        const list<shared_ptr<Variable> >& para_list,
                        const list<PoIdentifier>& port_list, const shared_ptr<Block>& body)
  : Block(*body), name(nm) {
  
  named = true;
  loc = lloc;
  
  // insert parameters
  {
    list<shared_ptr<Variable> >::const_iterator it, end;
    for(it=para_list.begin(),end=para_list.end(); it!=end; it++)
      db_param.insert((*it)->name, *it);  
  }

  // insert ports
  {
    list<PoIdentifier>::const_iterator it, end;
    for(it=port_list.begin(),end=port_list.end(); it!=end; it++)
      db_port.insert(*it, shared_ptr<Port>(new Port(it->loc, *it)));
  }

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

void netlist::Module::elab_inparse() {
  list<shared_ptr<NetComp> >::iterator it, end;
  for(it=statements.begin(), end=statements.end(); it!=end; it++) {
    switch((*it)->get_type()) {
    case tAssign: {
      SP_CAST(m, Assign, *it);
      m->set_name(new_BId());
      db_assign.insert(m->name, m);
      break;
    }
    case tSeqBlock: {
      SP_CAST(m, SeqBlock, *it);
      if(!m->is_named()) {
        m->set_default_name(new_BId());
        db_seqblock.insert(m->name, m);
      } else {
        m = db_seqblock.swap(m->name, m);
        if(m.use_count() != 0) {
          m->set_default_name(new_BId());
          db_seqblock.insert(m->name, m);
        }
      }         
      break;
    }
    case tGenBlock: {
      SP_CAST(m, GenBlock, *it);
      if(!m->is_named()) {
        m->set_default_name(new_BId());
        db_genblock.insert(m->name, m);
      } else {
        m = db_genblock.swap(m->name, m);
        if(m.use_count() != 0) {
          m->set_default_name(new_BId());
          db_genblock.insert(m->name, m);
        }
      }         
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
          m->set_default_name(new_IId());
          db_instance.insert(m->name, m);
        }
      }                 
      break;
    }
    case tPort: {
      SP_CAST(m, Port, *it);
      // should check first
      /// if multiple definitions exist for the same parameter, the last one take effect
      shared_ptr<Port> mm = db_port.swap(m->name, m);
      if(mm.use_count() != 0) {
        G_ENV->error(m->loc, "SYN-PORT-1", toString(m), toString(mm->loc));
      }
      
      // port declarations are not statements
      it = statements.erase(it);
      it--;
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
      case Variable::TParam: {
        /// if multiple definitions exist for the same parameter, the last one take effect
        db_param.swap(m->name, m); 
        break;
      }
      case Variable::TGenvar: {
        db_genvar.insert(m->name, m);
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

  blocked = true;               // module is always blocked
}
