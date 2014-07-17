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
#include <boost/tuple/tuple.hpp>
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
  if(slist_level.size() == 1 && slist_level.front()->is_variable() && slist_level.front()->get_variable().get_name() == "*") {
    // automatic sensitive list filling
    slist_level.clear();

    // right now disable this to use the new rtree
    //std::set<string> cset = get_rtree()->get_all();
    std::set<string> cset;

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

SeqBlock* netlist::SeqBlock::deep_copy(NetComp* bp) const {
  bool base_call = true;
  SeqBlock *rv;
  if(!bp) {
    rv = new SeqBlock();
    base_call = false;
  } else
    rv = static_cast<SeqBlock *>(bp); // C++ does not support multiple dispatch

  Block::deep_copy(rv);

  // data in SeqBlock
  rv->sensitive = sensitive;
  for_each(slist_pulse.begin(), slist_pulse.end(), [&](const pair<bool, shared_ptr<Expression> > m) {
      rv->slist_pulse.push_back(pair<bool, shared_ptr<Expression> >(m.first, shared_ptr<Expression>(m.second->deep_copy(NULL))));
    });
  BOOST_FOREACH(const shared_ptr<Expression>& m, slist_level)
    rv->slist_level.push_back(shared_ptr<Expression>(m->deep_copy(NULL)));

  if(!base_call) {
    rv->set_father();
  }
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


SDFG::RForest netlist::SeqBlock::get_rforest() const {
  return Block::get_rforest();
}

shared_ptr<Block> netlist::SeqBlock::unfold() {
  Block::unfold();
  return shared_ptr<Block>();
}

void netlist::SeqBlock::gen_sdfg(shared_ptr<SDFG::dfgGraph> G) {
  assert(db_var.empty());
  assert(db_instance.empty());

  SDFG::RForest rt = get_rforest();
  SDFG::sig_map cset = rt.get_control_signals();        // to store all control signals
  SDFG::plain_map rmap = rt.get_plain_map();

  SDFG::plain_map::iterator rm_it;
  for(rm_it = rmap.begin(); rm_it != rmap.end(); ++rm_it) {
    SDFG::plain_map_item::iterator rm_item_it;
    for(rm_item_it = rm_it->second.begin(); rm_item_it != rm_it->second.end(); ++rm_it) {
      SDFG::plain_relation::iterator rm_r_it;
      for(rm_r_it = rm_item_it->second.begin(); rm_r_it != rm_item_it->second.end(); ++rm_r_it) {
        string tar(rm_it->first), src(rm_item_it->first);
        list<SDFG::dfgRange> tarRanges = boost::get<0>(*rm_r_it).toRange();
        list<SDFG::dfgRange> srcRanges = boost::get<1>(*rm_r_it).toRange();
        unsigned int rtype = boost::get<2>(*rm_r_it);
        if(slist_pulse.empty() && rtype == SDFG::dfgEdge::SDFG_DDP) // assume no self-loop in combi
          continue;
        BOOST_FOREACH(SDFG::dfgRange& tr, tarRanges) {
          shared_ptr<SDFG::dfgNode> ptar;
          if(!G->exist(std::pair<string, SDFG::dfgRange>(tar, tr)))
            ptar = G->add_node(tar+tr.toString(), SDFG::dfgNode::SDFG_DF);
          else
            ptar = G->get_node(std::pair<string, SDFG::dfgRange>(tar, tr));
          ptar->ptr.insert(get_sp());
          BOOST_FOREACH(SDFG::dfgRange& sr, srcRanges) {
            shared_ptr<SDFG::dfgNode> psrc;
            if(!G->exist(std::pair<string, SDFG::dfgRange>(src, sr)))
              psrc = G->add_node(src+sr.toString(), SDFG::dfgNode::SDFG_DF);
            else
              psrc = G->get_node(std::pair<string, SDFG::dfgRange>(src, sr));
            G->add_edge_multi(src, rtype, psrc, ptar);
          }
        }
      }
    }
  }

  if(!slist_pulse.empty()) {    // ff
    assert(slist_pulse.size() <= 2 && slist_pulse.size() >= 1);
    SDFG::RTree sltree;
    typedef pair<bool, shared_ptr<Expression> > slist_type;
    BOOST_FOREACH(slist_type& sl, slist_pulse)
      sltree.combine(sl.second->get_rtree());

    std::set<string> clks, rsts;
    SDFG::sig_map slmap = sltree.get_all_signals();
    for(SDFG::sig_map::iterator it = slmap.begin(); it != slmap.end(); ++it) {
      if(cset.count(it->first))
        rsts.insert(it->first);
      else
        clks.insert(it->first);
    }

    assert(clks.size() == 1);
    assert(rsts.size() <= 1);
    
    // handle the nodes
    for(SDFG::tree_map::iterator it = rt.begin(); it!=rt.end(); ++it) {
      list<SDFG::dfgRange> rlist = it->second.get_select().toRange();
      BOOST_FOREACH(SDFG::dfgRange r, rlist) {
        // change node type
        shared_ptr<SDFG::dfgNode> tar = 
          G->get_node(pair<string, SDFG::dfgRange>(it->second.get_name(), r));
        tar->type = SDFG::dfgNode::SDFG_FF;
        
        // handle clock
        string clk_name = *(clks.begin());
        if(!G->exist(clk_name))
          G->add_node(clk_name, SDFG::dfgNode::SDFG_DF);
        G->add_edge(clk_name, SDFG::dfgEdge::SDFG_CLK, clk_name, tar);

        // handle the reset signals
        if(rsts.size() > 0) {
          BOOST_FOREACH(shared_ptr<SDFG::dfgEdge> e, tar->get_in_edges()) {
            if(rsts.count(SDFG::divide_signal_name(G->get_source(e)->name).first))
              e->type = SDFG::dfgEdge::SDFG_RST;
          }
        }
      }
    }
  } else {                      // combinational
    // handle the nodes
    for(SDFG::tree_map::iterator it = rt.begin(); it!=rt.end(); ++it) {
      list<SDFG::dfgRange> rlist = it->second.get_select().toRange();
      BOOST_FOREACH(SDFG::dfgRange r, rlist)
        G->get_node(pair<string, SDFG::dfgRange>(it->second.get_name(), r))->type = 
        SDFG::dfgNode::SDFG_COMB;
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

void netlist::SeqBlock::replace_variable(const VIdentifier& var, const VIdentifier& nvar) {
  BOOST_FOREACH(shared_ptr<Expression> sl, slist_level) {
    sl->replace_variable(var, nvar);
  }
  typedef pair<bool, shared_ptr<Expression> > sp_type;
  BOOST_FOREACH(sp_type sp, slist_pulse) {
    sp.second->replace_variable(var, nvar);
  }
  Block::replace_variable(var, nvar);
}

void netlist::SeqBlock::ssa_analysis(const VIdentifier& sname) {
  //std::cout << *this << std::endl;
  std::set<string> m_set;
  shared_ptr<Expression> combi_exp = Block::get_combined_expression(sname, m_set);
  //std::cout << *combi_exp << std::endl;
  combi_exp->extract_ssa_condition(sname);
}
