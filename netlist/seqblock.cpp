/*
 * Copyright (c) 2011-2013 Wei Song <songw@cs.man.ac.uk> 
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
#include "shell/env.h"
#include <algorithm>
#include <boost/foreach.hpp>
#include "sdfg/rtree.hpp"
#include "sdfg/sdfg.hpp"

using namespace netlist;
using std::ostream;
using std::string;
using std::vector;
using boost::shared_ptr;
using std::list;
using std::map;
using std::pair;
using shell::location;
using std::for_each;

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
  Block::elab_inparse();

  // handle sensitive list
  if(slist_level.size() == 1 && slist_level.front()->is_variable() && slist_level.front()->get_variable().name == "*") {
    // automatic sensitive list filling
    slist_level.clear();

    std::set<string> cset = get_rtree()->get_all();

    BOOST_FOREACH(const string& s, cset) {
      VIdentifier signal(s);
      slist_level.push_back(shared_ptr<Expression>(new Expression(signal)));
    }
  }
}

void netlist::SeqBlock::set_father(Block *pf) {
  if(father == pf) return;
  father = pf;
  {
    list<pair<bool, shared_ptr<Expression> > >::iterator it, end;
    for(it=slist_pulse.begin(), end=slist_pulse.end(); it!=end; it++)
      it->second->set_father(pf);
  }

  BOOST_FOREACH(shared_ptr<Expression>& it, slist_level)
    it->set_father(pf);
}

SeqBlock* netlist::SeqBlock::deep_copy() const {
  SeqBlock* rv = new SeqBlock();
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

  // data in SeqBlock
  rv->sensitive = sensitive;
  for_each(slist_pulse.begin(), slist_pulse.end(), [&](const pair<bool, shared_ptr<Expression> > m) {
      rv->slist_pulse.push_back(pair<bool, shared_ptr<Expression> >(m.first, shared_ptr<Expression>(m.second->deep_copy())));
    });
  BOOST_FOREACH(const shared_ptr<Expression>& m, slist_level)
    rv->slist_level.push_back(shared_ptr<Expression>(m->deep_copy()));

  rv->set_father();
  return rv;
}

void netlist::SeqBlock::db_register(int) {
  // the item in statements are duplicated in db_instance and db_other, therefore, only statements are executed
  // initialization of the variables in ablock are ignored as they are wire, reg and integers
  for_each(db_var.begin_order(), db_var.end_order(), [](pair<const VIdentifier, shared_ptr<Variable> >& m) {
      m.second->db_register(1);
    });
  BOOST_FOREACH(shared_ptr<NetComp>& m, statements) m->db_register(1);
  for_each(slist_pulse.begin(), slist_pulse.end(), [](pair<bool, shared_ptr<Expression> >& m) {
      m.second->db_register(1);
    });
  BOOST_FOREACH(shared_ptr<Expression>& m, slist_level) m->db_register(1);
}

void netlist::SeqBlock::db_expunge() {
  for_each(db_var.begin_order(), db_var.end_order(), [](pair<const VIdentifier, shared_ptr<Variable> >& m) {
      m.second->db_expunge();
    });
  BOOST_FOREACH(shared_ptr<NetComp>& m, statements) m->db_expunge();
  for_each(slist_pulse.begin(), slist_pulse.end(), [](pair<bool, shared_ptr<Expression> >& m) {
      m.second->db_expunge();
    });
  BOOST_FOREACH(shared_ptr<Expression>& m, slist_level) m->db_expunge();
}


shared_ptr<SDFG::RTree> netlist::SeqBlock::get_rtree() const {
  return Block::get_rtree();
}


void netlist::SeqBlock::gen_sdfg(shared_ptr<SDFG::dfgGraph> G) {
  assert(db_var.empty());
  assert(db_instance.empty());

  shared_ptr<SDFG::RTree> rt = get_rtree();
  std::set<string> cset = rt->get_control();        // to store all control signals
  
  BOOST_FOREACH(SDFG::RTree::sub_tree_type& t, rt->tree) {
    BOOST_FOREACH(SDFG::RTree::rtree_edge_type& e, t.second) {
      G->add_edge_multi(e.first, e.second, e.first, t.first);
    }
  }

  if(!slist_pulse.empty()) {    // ff
    assert(slist_pulse.size() <= 2 && slist_pulse.size() >= 1);
    shared_ptr<SDFG::RTree> sltree(new SDFG::RTree());
    typedef pair<bool, shared_ptr<Expression> > slist_type;
    BOOST_FOREACH(slist_type& sl, slist_pulse) {
      sltree->add_tree(sl.second->get_rtree());
    }

    std::set<string> clks, rsts;

    BOOST_FOREACH(const string& s, sltree->get_all()) {
      if(cset.count(s))
        rsts.insert(s);
      else
        clks.insert(s);
    }

    assert(clks.size() == 1);
    assert(rsts.size() <= 1);
    
    // handle the nodes
    BOOST_FOREACH(SDFG::RTree::sub_tree_type& t, rt->tree) {
      shared_ptr<SDFG::dfgNode> node = G->get_node(t.first);
      assert(node);
      node->type = SDFG::dfgNode::SDFG_FF;
      // handle the reset signals
      if(rsts.size() > 0 && G->exist(*(rsts.begin()), t.first, SDFG::dfgEdge::SDFG_CTL))
        G->get_edge(*(rsts.begin()), t.first, SDFG::dfgEdge::SDFG_CTL)->type = SDFG::dfgEdge::SDFG_RST;
      
      // handle clock
      string clk_name = *(clks.begin());
      G->add_edge(clk_name, SDFG::dfgEdge::SDFG_CLK, clk_name, t.first);
    }
  } else {                      // combinational
    // handle the nodes
    BOOST_FOREACH(SDFG::RTree::sub_tree_type& t, rt->tree) {
      if(t.second.count(t.first))
        G->get_node(t.first)->type = SDFG::dfgNode::SDFG_LATCH; // self-loop means latch
      else
        G->get_node(t.first)->type = SDFG::dfgNode::SDFG_COMB;
    }
  }
}

void netlist::SeqBlock::replace_variable(const VIdentifier& var, const Number& num) {
  BOOST_FOREACH(shared_ptr<Expression> sl, slist_level) {
    sl->replace_variable(var, num);
  }
  typedef pair<bool, shared_ptr<Expression> > sp_type;
  BOOST_FOREACH(sp_type sp, slist_pulse) {
    sp.second->replace_variable(var, num);
  }
  Block::replace_variable(var, num);
}

void netlist::SeqBlock::ssa_analysis(const VIdentifier& sname) {
  //std::cout << *this << std::endl;
  std::set<string> m_set;
  shared_ptr<Expression> combi_exp = Block::get_combined_expression(sname, m_set);
  //std::cout << *combi_exp << std::endl;
  combi_exp->extract_ssa_condition(sname);
}
