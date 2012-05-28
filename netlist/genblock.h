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
 * 04/04/2012   Wei Song
 *
 *
 */

#ifndef AV_H_AV_GEN_BLOCK_
#define AV_H_AV_GEN_BLOCK_

namespace netlist{
  class GenBlock : public Block {
  public:
    // constructors
    GenBlock() : Block(tGenBlock) {}
    GenBlock(const Block& body);
    GenBlock(const shell::location& lloc, const Block& body);
    
    // helpers
    virtual std::ostream& streamout(std::ostream&, unsigned int, bool) const;    
    virtual void elab_inparse();        /* resolve the content during parsing */
    using Block::set_father;

    // inherit from NetComp
    NETLIST_STREAMOUT_DECL;
    virtual GenBlock* deep_copy() const;

  private:
    bool elab_inparse_item( const boost::shared_ptr<NetComp>&);

  };
  NETLIST_STREAMOUT(GenBlock);

}

#endif
