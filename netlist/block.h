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
    Block(NetComp::ctype_t t, const BIdentifier& nm) 
      : NetComp(t), name(nm), named(true) {}
    Block(NetComp::ctype_t t) 
      : NetComp(t), named(false) {}

    // helpers
    void set_name(const BIdentifier& nm) {name = nm; named=true;}
    bool is_named() const { return named; }
    virtual void clear();               /* clear all statements */
    virtual bool add_assignment(const shared_ptr<Assign>&); /* add a blocking or non-blocking assignment into the block */
    virtual bool add_case(const shared_ptr<Expression>&, const list<shared_ptr<CaseItem> >&, const shared_ptr<CaseItem>&); /* add a general case statement */
    virtual bool add_case(const shared_ptr<Expression>&, const list<shared_ptr<CaseItem> >&); /* add a case statement without default */
    virtual bool add_case(const shared_ptr<Expression>&, const shared_ptr<CaseItem>&); /* add a case statement only with a default case, odd! */
    virtual bool add_if(const shared_ptr<Expression>&, const shared_ptr<SeqBlock>&, const shared_ptr<SeqBlock>&); /* add an if statement with else case */
    virtual bool add_if(const shared_ptr<Expression>&, const shared_ptr<SeqBlock>&); /* add an if statement without else case */
    virtual bool add_while(const shared_ptr<Expression>&, const shared_ptr<SeqBlock>&); /* add a while statement */
    virtual bool add_for(const shared_ptr<Assign>&, const shared_ptr<Expression>&, const shared_ptr<Assign>&, const shared_ptr<SeqBlock>&); /* add a for statement */
    virtual bool add_block(const shared_ptr<SeqBlock>&); /* add a statement block */
    virtual bool add_statements(const shared_ptr<SeqBlock>&); /* add several statements */

    // data
    BIdentifier name;
    bool named;
    list<shared_ptr<NetComp> > statements;   /* a general list to stor the statements */
    DataBase<VIdentifier, Variable> db_var;  /* local variables */

  };

  class GenBlock : public Block {
  public:
    // constructors
    GenBlock()
      : Block(NetComp::tGenBlock) {}
    GenBlock(const BIdentifier& nm)
      : Block(NetComp::tGenBlock, nm) {}

    // helpers
    BIdentifier& new_BId();     /* generate an unused block id */
    IIdentifier& new_IId();     /* generate an unused instance id*/
    VIdentifier& new_VId();     /* generate an unused variable id */
    
    // helpersinherit from NetComp
    NETLIST_STREAMOUT_FUN_DECL;

    // data
    MIdentifier name;
    DataBase<VIdentifier, Variable>        db_wire;      /* wires */
    DataBase<VIdentifier, Variable>        db_reg;       /* registers */
    DataBase<IIdentifier, Instance>        db_instance;  /* module instances */

    DataBase<string, Assign>               db_assign;    /* continueous assignments */
    DataBase<BIdentifier, SeqBlock>        db_block;     /* always blocks */

  private:
    //name for unnamed items
    BIdentifier unnamed_block;
    IIdentifier unnamed_instance;
    VIdentifier unnamed_var;
  };
  NETLIST_STREAMOUT(GenBlock);

  class SeqBlock : public Block {
  public:
    // constructors
    SeqBlock()
      : Block(NetComp::tSeqBlock), sensitive(false) {}
    SeqBlock(const BIdentifier& nm)
      : Block(NetComp::tSeqBlock, nm), sensitive(false) {}

    // helpers
    virtual void clear();               /* clear all statements */
    virtual bool add_seq_block(list<pair<int, shared_ptr<Expression> > >&, const shared_ptr<SeqBlock>&); /* add a sequential block */
    NETLIST_STREAMOUT_FUN_DECL;
    virtual ostream& streamout(ostream& os, unsigned int indent, bool fl_prefix) const;

    // data
    bool sensitive;                                         /* whether this is a sensitive block, top level block */
    list<pair<bool, shared_ptr<Expression> > > slist_pulse; /* pulse sensitive list */
    list<shared_ptr<Expression> > slist_level;              /* level sensitive list */
    

  };
  NETLIST_STREAMOUT(SeqBlock);

  
}

#endif
