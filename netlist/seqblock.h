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
 * Sequential block definition
 * A block may be embedded in another one.
 * 04/04/2012   Wei Song
 *
 *
 */

#ifndef AV_H_AV_SEQ_BLOCK_
#define AV_H_AV_SEQ_BLOCK_

namespace netlist{

  class SeqBlock : public Block {
  public:
    // constructors
    SeqBlock() : Block(tSeqBlock), sensitive(false) {};
    SeqBlock(const shell::location& lloc) : Block(tSeqBlock, lloc), sensitive(false) {};
    SeqBlock(const BIdentifier& nm)
      : Block(tSeqBlock, nm), sensitive(false) {}
    SeqBlock(const shell::location& lloc, const BIdentifier& nm)
      : Block(tSeqBlock, lloc, nm), sensitive(false) {}
    SeqBlock(std::list<std::pair<int, boost::shared_ptr<Expression> > >&, const boost::shared_ptr<Block>&);
    SeqBlock(const shell::location&, std::list<std::pair<int, boost::shared_ptr<Expression> > >&, const boost::shared_ptr<Block>&);
    SeqBlock(const Block&);
    SeqBlock(const shell::location&, const Block&);
    
    // helpers
    virtual std::ostream& streamout(std::ostream&, unsigned int, bool) const;
    virtual void elab_inparse();        /* resolve the content during parsing */

    // inherit from NetComp
    NETLIST_STREAMOUT_DECL;
    NETLIST_SET_FATHER_DECL;
    NETLIST_CHECK_INPARSE_DECL;
    using Block::set_father;
    virtual SeqBlock* deep_copy() const;
    
    // data
    bool sensitive;                                         /* whether this is a sensitive block, top level block */
    std::list<std::pair<bool, boost::shared_ptr<Expression> > > slist_pulse; /* pulse sensitive list */
    std::list<boost::shared_ptr<Expression> > slist_level;                   /* level sensitive list */
        
  };

  NETLIST_STREAMOUT(SeqBlock);


}

#endif
