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
 * 22/03/2012   Wei Song
 *
 *
 */

#ifndef _H_AV_BLOCK_
#define _H_AV_BLOCK_

namespace netlist {

  class Block : public NetComp {
  public:
    // constructors
    NETLIST_DEFAULT_CON(Block, tBlock);
    Block(NetComp::ctype_t t, const BIdentifier& nm) 
      : NetComp(t), name(nm), named(true), blocked(true) {}
    Block(const BIdentifier& nm) 
      : NetComp(NetComp::tBlock), name(nm), named(true), blocked(true) {}
    Block(NetComp::ctype_t t) 
      : NetComp(t) {}

    // helpers
    virtual void set_name(const BIdentifier& nm) {name = nm; named=true; blocked = true;}
    virtual void set_blocked() { blocked = true; }
    virtual bool is_named() const { return named; }
    virtual bool is_blocked() const { return blocked; }
    virtual void clear();               /* clear all statements */
    virtual bool add(const shared_ptr<NetComp>&); /* add a general statement to this block */
    
    /* a template funtion to add all sorts of netlist components */
    template<typename T>
      bool add_list(const list<shared_ptr<T> >& ll) {
      typename list<shared_ptr<T> >::const_iterator it, end;
      for(it=ll.begin(), end=ll.end(); it!=end; it++)
        add(*it);
      return true;
    }

    virtual bool add_case(const shared_ptr<Expression>&, const list<shared_ptr<CaseItem> >&, const shared_ptr<CaseItem>&); /* add a general case statement */
    virtual bool add_case(const shared_ptr<Expression>&, const list<shared_ptr<CaseItem> >&); /* add a case statement without default */
    virtual bool add_case(const shared_ptr<Expression>&, const shared_ptr<CaseItem>&); /* add a case statement only with a default case, odd! */
    virtual bool add_if(const shared_ptr<Expression>&, const shared_ptr<Block>&, const shared_ptr<Block>&); /* add an if statement with else case */
    virtual bool add_if(const shared_ptr<Expression>&, const shared_ptr<Block>&); /* add an if statement without else case */
    virtual bool add_while(const shared_ptr<Expression>&, const shared_ptr<Block>&); /* add a while statement */
    virtual bool add_for(const shared_ptr<Assign>&, const shared_ptr<Expression>&, const shared_ptr<Assign>&, const shared_ptr<Block>&); /* add a for statement */
    virtual bool add_statements(const shared_ptr<Block>&); /* add several statements */
    virtual void elab_inparse();                           /* resolve the content in statements during parsing */

    // helpers
    BIdentifier& new_BId();     /* generate an unused block id */
    IIdentifier& new_IId();     /* generate an unused instance id*/
    VIdentifier& new_VId();     /* generate an unused variable id */
    virtual ostream& streamout(ostream& os, unsigned int indent, bool fl_prefix) const;
    
    // inherit from NetComp
    NETLIST_STREAMOUT_FUN_DECL;

    // data
    BIdentifier name;
    list<shared_ptr<NetComp> >             statements;   /* a general list to stor the statements */
    DataBase<VIdentifier, Variable>        db_wire;      /* wires */
    DataBase<VIdentifier, Variable>        db_reg;       /* registers */
    DataBase<IIdentifier, Instance>        db_instance;  /* module instances */
    DataBase<BIdentifier, Block>           db_block;     /* sub-blocks */
    DataBase<BIdentifier, NetComp>         db_other;     /* non-block statements, including assignements, if, etc. */
    

  protected:

    //name for unnamed items
    BIdentifier unnamed_block;
    IIdentifier unnamed_instance;
    VIdentifier unnamed_var;

    bool named;                 /* true when named */
    bool blocked;               /* user used begin and end for this block */

  };

  NETLIST_STREAMOUT(Block);
 
}

#endif
