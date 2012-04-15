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

#include "component.h"

using namespace netlist;

void netlist::SeqBlock::clear() {
  Block::clear();
  sensitive = false;
  slist_pulse.clear();
  slist_level.clear();
}

ostream& netlist::SeqBlock::streamout(ostream& os, unsigned int indent) const {
  streamout(os, indent, false);
  return os;
}

ostream& netlist::SeqBlock::streamout(ostream& os, unsigned int indent, bool fl_prefix) const {
  
  if(!fl_prefix) os << string(indent, ' ');
  
  os << "always ";
  
  if(sensitive) {
    os << "@(";
    if(slist_pulse.size() > 0) {
      list<pair<bool, shared_ptr<Expression> > >::const_iterator it, end;
      it = slist_pulse.begin();
      end = slist_pulse.end();
      while(true) {
        if(it->first) {
          os << "posedge ";
          it->second->streamout(os, 0);
        } else {
          os << "negedge ";
          it->second->streamout(os, 0);
        }
        it++;
        if(it != end) os << " or ";
        else break;
      }
    } else {
      list<shared_ptr<Expression> >::const_iterator it, end;
      it = slist_level.begin();
      end = slist_level.end();
      while(true) {
        os << **it;
        it++;
        if(it != end) os << " or ";
        else break;
      }
    }
    os << ") ";
  }

  Block::streamout(os, indent, true);

  return os; 
}

netlist::SeqBlock::SeqBlock(list<pair<int, shared_ptr<Expression> > >& slist, const shared_ptr<Block>& body) 
  : Block(*body)
{
  ctype = tSeqBlock;
  
  list<pair<int, shared_ptr<Expression> > >::iterator it, end;
  for(it=slist.begin(), end=slist.end(); it!=end; it++) {
    if(it->first > 0)
      slist_pulse.push_back(pair<bool, shared_ptr<Expression> >(true, it->second));
    else if(it->first < 0)
      slist_pulse.push_back(pair<bool, shared_ptr<Expression> >(false, it->second));
    else
      slist_level.push_back(it->second);
  }
  
  sensitive = true; 
  named = body->is_named();
  if(named) name = body->name;
  statements = body->statements;

  // TODO: checking sensitive list, only one type is used
  elab_inparse();

}
      
netlist::SeqBlock::SeqBlock(const location& lloc, list<pair<int, shared_ptr<Expression> > >& slist, const shared_ptr<Block>& body) 
  : Block(*body)
{
  ctype = tSeqBlock;
  loc = lloc;
  
  list<pair<int, shared_ptr<Expression> > >::iterator it, end;
  for(it=slist.begin(), end=slist.end(); it!=end; it++) {
    if(it->first > 0)
      slist_pulse.push_back(pair<bool, shared_ptr<Expression> >(true, it->second));
    else if(it->first < 0)
      slist_pulse.push_back(pair<bool, shared_ptr<Expression> >(false, it->second));
    else
      slist_level.push_back(it->second);
  }
  
  sensitive = true; 
  named = body->is_named();
  if(named) name = body->name;
  statements = body->statements;

  // TODO: checking sensitive list, only one type is used
  elab_inparse();

}
      
netlist::SeqBlock::SeqBlock(const Block& body) 
  : Block(body), sensitive(false)
{
  ctype = tSeqBlock;
  elab_inparse();
}

netlist::SeqBlock::SeqBlock(const location& lloc, const Block& body) 
  : Block(body), sensitive(false)
{
  ctype = tSeqBlock;
  loc = lloc;
  elab_inparse();
}

void netlist::SeqBlock::elab_inparse() {

  // TODO: checking sensitive list, only one type is used
  //       right now nothing is checked

  // just use the elab_inparse of Block is fine for always blocks
  Block::elab_inparse();

  // make sure it is blocked
  blocked = true;

}
      
