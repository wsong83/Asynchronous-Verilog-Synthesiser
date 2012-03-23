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
    virtual ostream& streamout (ostream& ) const = 0;
    void set_name(const BIdentifier& nm) {name = nm; named=true;}

    // data
    BIdentifier name;
    bool named;
    list<NetComp> statements;   /* a general list to stor the statements */

  };

  class GenBlock : public Block {
  public:
    // constructors
    GenBlock()
      : Block(NetComp::tGenBlock, BIdentifier()) {}
    GenBlock(const BIdentifier& nm)
      : Block(NetComp::tGenBlock, nm) {}

    // helpers
    ostream& streamout (ostream& ) const;
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
    ostream& streamout (ostream& ) const;
    void clear();               /* clear all statements */
    bool add_assignment(Assign&); /* add a blocking or non-blocking assignment into the block */
    bool add_case(Expression&, list<CaseItem>&, CaseItem&); /* add a general case statement */
    bool add_case(Expression&, list<CaseItem>&);            /* add a case statement without default */
    bool add_case(Expression&, CaseItem&);                  /* add a case statement only with a default case, odd! */
    bool add_if(Expression&, SeqBlock&, SeqBlock&);         /* add an if statement with else case */
    bool add_while(Expression&, SeqBlock&);                 /* add a while statement */
    bool add_for(Assign&, Expression&, Assign&, SeqBlock&); /* add a for statement */
    bool add_seq_block(list<pair<int, Expression> >&, SeqBlock&); /* add a sequential block */
    bool add_block(SeqBlock&);                              /* add a statement block */

    // data
    bool sensitive;                            /* whether this is a sensitive block, top level block */
    list<pair<bool, Expression> > slist_pulse; /* pulse sensitive list */
    list<Expression> slist_level;              /* level sensitive list */
    

  };
  NETLIST_STREAMOUT(SeqBlock);

  
}

#endif
