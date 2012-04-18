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
 * 14/02/2012   Wei Song
 *
 *
 */

#ifndef _H_MODULE_
#define _H_MODULE_

namespace netlist {
  
  class Module : public Block {
  public:
    Module()
      : Block(tModule) {}
    Module(const MIdentifier& nm)
      : Block(tModule), name(nm) { named=true; }
  Module(const location& lloc, const MIdentifier& nm)
    : Block(tModule, lloc), name(nm) { named=true; }
    Module(const MIdentifier& nm, const shared_ptr<Block>& body);
    Module(const location& lloc, const MIdentifier& nm, const shared_ptr<Block>& body);
    Module(const MIdentifier& nm, const list<PoIdentifier>& port_list, const shared_ptr<Block>& body);
    Module(const location& lloc, const MIdentifier& nm, const list<PoIdentifier>& port_list, const shared_ptr<Block>& body);
    Module(const MIdentifier& nm, const list<shared_ptr<Variable> >& para_list, 
           const list<PoIdentifier>& port_list, const shared_ptr<Block>& body);
    Module(const location& lloc, const MIdentifier& nm, 
           const list<shared_ptr<Variable> >& para_list, 
           const list<PoIdentifier>& port_list, const shared_ptr<Block>& body);

    // inherit from NetComp
    NETLIST_STREAMOUT_DECL;

    // helpers
    virtual void set_name(const MIdentifier& nm) { name = nm; named=true;}
    VIdentifier& new_VId();
    BIdentifier& new_BId();
    shared_ptr<Variable>  find_var       (const VIdentifier&) const; /* find a variable */
    shared_ptr<Block>     find_block     (const BIdentifier&) const; /* find a block */
    shared_ptr<NetComp>   find_item      (const BIdentifier&) const; /* find an item in db_other */
    virtual void elab_inparse();                           /* resolve the content in statements during parsing */
    
    // data
    MIdentifier name;
    DataBase<PoIdentifier, Port, true>     db_port;      /* input and output ports, ordered */
    DataBase<VIdentifier, Variable, true>  db_param;     /* parameters, ordered */
    DataBase<VIdentifier, Variable, true>  db_genvar;    /* generate variable, ordered */
    DataBase<BIdentifier, SeqBlock>        db_seqblock;  /* always blocks */
    DataBase<BIdentifier, Assign>          db_assign;    /* continuous assignments */
    DataBase<BIdentifier, GenBlock>        db_genblock;  /* generation blocks */

  private:
    // only used in constructors
    void init_port_list(const list<PoIdentifier>&);
    void init_param_list(const list<shared_ptr<Variable> >&);
    // helper in elab_inparse
    bool elab_inparse_item( const shared_ptr<NetComp>&);

  };

  NETLIST_STREAMOUT(Module);
}

#endif
