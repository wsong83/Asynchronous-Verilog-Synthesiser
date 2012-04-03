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
 * If statements
 * 02/04/2012   Wei Song
 *
 *
 */

#include "component.h"

using namespace netlist;

netlist::IfState::IfState(
                          const shared_ptr<Expression>& exp, 
                          const shared_ptr<SeqBlock>& m_ifcase, 
                          const shared_ptr<SeqBlock>& m_elsecase
                          )
  : NetComp(NetComp::tIf), exp(exp) 
{
  if(m_ifcase->is_named()) {
    ifcase.push_back(static_pointer_cast<NetComp>(m_ifcase));
  } else {
    ifcase = m_ifcase->statements;
  }

  if(m_elsecase->is_named()) {
    elsecase.push_back(static_pointer_cast<NetComp>(m_elsecase));
  } else {
    elsecase = m_elsecase->statements;
  }
}

netlist::IfState::IfState(
                          const shared_ptr<Expression>& exp, 
                          const shared_ptr<SeqBlock>& m_ifcase
                          )
  : NetComp(NetComp::tIf), exp(exp) 
{
  if(m_ifcase->is_named()) {
    ifcase.push_back(static_pointer_cast<NetComp>(m_ifcase));
  } else {
    ifcase = m_ifcase->statements;
  }
}

ostream& netlist::IfState::streamout(ostream& os, unsigned int indent) const {
  return streamout(os, indent, false);
}

ostream& netlist::IfState::streamout(ostream& os, unsigned int indent, bool fl_prefix) const {
  assert(exp.use_count() != 0);

  if(!fl_prefix) os << string(indent, ' ');
  
  os << "if (" << *exp << ") ";

  if(ifcase.size() == 1) {
    if(ifcase.front()->get_type() == NetComp::tSeqBlock)
      static_pointer_cast<SeqBlock>(ifcase.front())->streamout(os, indent, true);
    else {
      os << endl;
      ifcase.front()->streamout(os, indent+2);
      if(ifcase.front()->get_type() == NetComp::tAssign) os << ";" << endl;
    }
  } else {
    os << "begin" << endl;
    list<shared_ptr<NetComp> >::const_iterator it, end;
    for(it=ifcase.begin(), end=ifcase.end(); it!=end; it++) {
      (*it)->streamout(os, indent+2);
      if((*it)->get_type() == NetComp::tAssign) os << ";" << endl;
    }
    os << "end" << endl;
  }

  if(elsecase.size() == 0) return os;

  os << string(indent, ' ') << "else ";
  
  if(elsecase.size() == 1) {
    if(elsecase.front()->get_type() == NetComp::tSeqBlock)
      static_pointer_cast<SeqBlock>(elsecase.front())->streamout(os, indent, true);
    else if(elsecase.front()->get_type() == NetComp::tIf)
      static_pointer_cast<IfState>(elsecase.front())->streamout(os, indent, true);
    else {
      os << endl;
      elsecase.front()->streamout(os, indent+2);
      if(elsecase.front()->get_type() == NetComp::tAssign) os << ";" << endl;
    }
  } else {
    os << "begin" << endl;
    list<shared_ptr<NetComp> >::const_iterator it, end;
    for(it=elsecase.begin(), end=elsecase.end(); it!=end; it++) {
      (*it)->streamout(os, indent+2);
      if((*it)->get_type() == NetComp::tAssign) os << ";" << endl;
    }
    os << "end" << endl;
  }
  
  return os;

}
