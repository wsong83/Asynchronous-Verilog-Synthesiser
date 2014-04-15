/*
 * Copyright (c) 2012-2014 Wei Song <songw@cs.man.ac.uk> 
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
#include "shell/env.h"
#include <algorithm>
#include <boost/foreach.hpp>

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

netlist::GenBlock::GenBlock() 
  : Block(tGenBlock) {}

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
  ctype_t mt = tUnknown;
  BOOST_FOREACH(const shared_ptr<NetComp>& it, statements) {
    ctype_t mt_nxt = it->get_type();
    if(mt != mt_nxt || mt != tAssign) {
      if(mt != tUnknown) os << endl;
      mt = mt_nxt;
    } 
    it->streamout(os, indent+2);
  }
  os << string(indent, ' ') << "endgenerate" << endl;
  return os;
}

GenBlock* netlist::GenBlock::deep_copy() const {
  GenBlock* rv = new GenBlock();
  rv->loc = loc;
  rv->name = name;
  rv->named = named;
  
  // data in Block
  BOOST_FOREACH(const shared_ptr<NetComp>& comp, statements)
    rv->statements.push_back(shared_ptr<NetComp>(comp->deep_copy())); 
  
  DATABASE_DEEP_COPY_FUN(db_var,      VIdentifier, Variable,  rv->db_var       );
  rv->unnamed_block = unnamed_block;
  rv->unnamed_instance = unnamed_instance;
  rv->unnamed_var = unnamed_var;

  // special components in Blocks
  DATABASE_DEEP_COPY_FUN(db_instance,  IIdentifier,  Instance,  rv->db_instance );
  DATABASE_DEEP_COPY_FUN(db_func,      FIdentifier,  Function,  rv->db_func     );

  rv->set_father();
  rv->elab_inparse();
  return rv;
}

void netlist::GenBlock::db_register(int) {
  // the item in statements are duplicated in db_instance and db_other, therefore, only statements are executed
  // initialization of the variables in ablock are ignored as they are wire, reg and integers
  for_each(db_var.begin_order(), db_var.end_order(), [](pair<const VIdentifier, shared_ptr<Variable> >& m) {
      m.second->db_register(1);
    });
  BOOST_FOREACH(shared_ptr<NetComp>& m, statements) m->db_register(1);
  for_each(db_instance.begin(), db_instance.end(), [](pair<const IIdentifier, shared_ptr<Instance> >& m) {
	m.second->db_register(1);
      });
  for_each(db_func.begin(), db_func.end(), [](pair<const FIdentifier, shared_ptr<Function> >& m) {
	m.second->db_register(1);
      });
}

void netlist::GenBlock::db_expunge() {
  for_each(db_var.begin_order(), db_var.end_order(), [](pair<const VIdentifier, shared_ptr<Variable> >& m) {
      m.second->db_expunge();
    });
  BOOST_FOREACH(shared_ptr<NetComp>& m, statements) m->db_expunge();
  for_each(db_instance.begin(), db_instance.end(), [](pair<const IIdentifier, shared_ptr<Instance> >& m) {
	m.second->db_expunge();
      });
  for_each(db_func.begin(), db_func.end(), [](pair<const FIdentifier, shared_ptr<Function> >& m) {
	m.second->db_expunge();
      });
}
