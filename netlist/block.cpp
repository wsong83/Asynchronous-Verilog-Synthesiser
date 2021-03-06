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
 * General block definition
 * A block may be embedded in another one.
 * 23/03/2012   Wei Song
 *
 *
 */

#include "component.h"
#include "shell/env.h"
#include <algorithm>
#include <boost/foreach.hpp>
#include "sdfg/sdfg.hpp"
#include "sdfg/rtree.hpp"

using namespace netlist;
using std::ostream;
using std::endl;
using std::string;
using std::vector;
using boost::shared_ptr;
using boost::static_pointer_cast;
using std::list;
using std::pair;
using std::map;
using shell::location;
using std::for_each;

netlist::Block::Block() 
  : NetComp(tBlock), named(false) {}

netlist::Block::Block(const shell::location& lloc) 
  : NetComp(tBlock, lloc), named(false) {}

netlist::Block::Block(ctype_t t, const BIdentifier& nm) 
  : NetComp(t), name(nm), named(true) {}

netlist::Block::Block(ctype_t t, const shell::location& lloc, const BIdentifier& nm) 
  : NetComp(t, lloc), name(nm), named(true) {}

netlist::Block::Block(const BIdentifier& nm) 
  : NetComp(tBlock), name(nm), named(true) {}

netlist::Block::Block(const shell::location& lloc, const BIdentifier& nm) 
  : NetComp(tBlock, lloc), name(nm), named(true) {}

netlist::Block::Block(NetComp::ctype_t t) 
  : NetComp(t), named(false) {}

netlist::Block::Block(NetComp::ctype_t t, const shell::location& lloc) 
  : NetComp(t, lloc), named(false) {}

bool netlist::Block::add(const shared_ptr<NetComp>& dd) {
  statements.push_back(dd);
  return true;
}

bool netlist::Block::add_case(const shared_ptr<Expression>& exp, const list<shared_ptr<CaseItem> >& citems, const shared_ptr<CaseItem>& ditem, CaseState::case_type_t m_casex) {
  statements.push_back(shared_ptr<CaseState>( new CaseState(exp, citems, ditem, m_casex)));
  return true;
}

bool netlist::Block::add_case(const location& lloc, const shared_ptr<Expression>& exp, const list<shared_ptr<CaseItem> >& citems, const shared_ptr<CaseItem>& ditem, CaseState::case_type_t m_casex) {
  statements.push_back(shared_ptr<CaseState>( new CaseState(lloc, exp, citems, ditem, m_casex)));
  return true;
}

bool netlist::Block::add_case(const shared_ptr<Expression>& exp, const list<shared_ptr<CaseItem> >& citems, CaseState::case_type_t m_casex) {
  statements.push_back(shared_ptr<CaseState>( new CaseState(exp, citems, m_casex)));
  return true;
}

bool netlist::Block::add_case(const location& lloc, const shared_ptr<Expression>& exp, const list<shared_ptr<CaseItem> >& citems, CaseState::case_type_t m_casex) {
  statements.push_back(shared_ptr<CaseState>( new CaseState(lloc, exp, citems, m_casex)));
  return true;
}

bool netlist::Block::add_case(const shared_ptr<Expression>& exp, const shared_ptr<CaseItem>& ditem, CaseState::case_type_t m_casex) {
  statements.push_back(shared_ptr<CaseState>( new CaseState(exp, ditem, m_casex)));
  return true;
}

bool netlist::Block::add_case(const location& lloc, const shared_ptr<Expression>& exp, const shared_ptr<CaseItem>& ditem, CaseState::case_type_t m_casex) {
  statements.push_back(shared_ptr<CaseState>( new CaseState(lloc, exp, ditem, m_casex)));
  return true;
}

bool netlist::Block::add_if(const shared_ptr<Expression>& exp, const shared_ptr<Block>& ifcase, const shared_ptr<Block>& elsecase) {
  statements.push_back(shared_ptr<IfState>( new IfState(exp, ifcase, elsecase)));
  return true;
}

bool netlist::Block::add_if(const location& lloc, const shared_ptr<Expression>& exp, const shared_ptr<Block>& ifcase, const shared_ptr<Block>& elsecase) {
  statements.push_back(shared_ptr<IfState>( new IfState(lloc, exp, ifcase, elsecase)));
  return true;
}

bool netlist::Block::add_if(const shared_ptr<Expression>& exp, const shared_ptr<Block>& ifcase) {
  statements.push_back(shared_ptr<IfState>( new IfState(exp, ifcase)));
  return true;
}

bool netlist::Block::add_if(const location& lloc, const shared_ptr<Expression>& exp, const shared_ptr<Block>& ifcase) {
  statements.push_back(shared_ptr<IfState>( new IfState(lloc, exp, ifcase)));
  return true;
}

bool netlist::Block::add_while(const shared_ptr<Expression>& exp, const shared_ptr<Block>& body) {
  statements.push_back(shared_ptr<WhileState>( new WhileState(exp, body)));
  return true;
}

bool netlist::Block::add_while(const location& lloc, const shared_ptr<Expression>& exp, const shared_ptr<Block>& body) {
  statements.push_back(shared_ptr<WhileState>( new WhileState(lloc, exp, body)));
  return true;
}

bool netlist::Block::add_for(const shared_ptr<Assign>& init, const shared_ptr<Expression>& cond, const shared_ptr<Assign>& incr, const shared_ptr<Block>& body) {
  statements.push_back(shared_ptr<ForState>( new ForState(init, cond, incr, body)));
  return true;
}

bool netlist::Block::add_for(const location& lloc, const shared_ptr<Assign>& init, const shared_ptr<Expression>& cond, const shared_ptr<Assign>& incr, const shared_ptr<Block>& body) {
  statements.push_back(shared_ptr<ForState>( new ForState(lloc, init, cond, incr, body)));
  return true;
}

bool netlist::Block::add_statements(const shared_ptr<Block>& body) {
  if(body->is_named()) {
    statements.push_back(body);
  } else {
    statements.splice(statements.end(), body->statements);
  }
  return true;
}

bool netlist::Block::elab_add_block(const shared_ptr<Block>& b) {
  return elab_replace_statement(shared_ptr<NetComp>(), b);
}

bool netlist::Block::elab_replace_statement(const shared_ptr<NetComp>& st, const shared_ptr<Block> nb) {
  // find out the original statement
  list<shared_ptr<NetComp> >::iterator it;
  if(st)
    it = std::find(statements.begin(), statements.end(), st);
  else
    it = statements.end();

  // insert the new statements from the new block
  for_each(nb->db_func.begin(), nb->db_func.end(),
           [&](pair<const FIdentifier, shared_ptr<Function> >& item) {
             if(db_func.count(item.first)) {
               G_ENV->error(loc, "ELAB-FOR-5", item.first.get_name());
             } else {
               db_func.insert(item.first, item.second);
             }
           });
  for_each(nb->db_instance.begin(), nb->db_instance.end(),
           [&](pair<const IIdentifier, shared_ptr<Instance> >& item) {
             if(db_instance.count(item.first)) {
               G_ENV->error(loc, "ELAB-FOR-5", item.first.get_name());
             } else {
               db_instance.insert(item.first, item.second);
             }
           });
  for_each(nb->db_var.begin_order(), nb->db_var.end_order(),
           [&](pair<const VIdentifier, shared_ptr<Variable> >& item) {
             if(db_var.count(item.first)) {
               G_ENV->error(loc, "ELAB-FOR-5", item.first.get_name());
             } else {
               db_var.insert(item.first, item.second);
             }
           });
  statements.insert(it, nb->statements.begin(), nb->statements.end());

  // remove the original statement
  if(st)
    statements.erase(it);
  
  set_father();
  
  return true;
}

BIdentifier& netlist::Block::new_BId() {
  unnamed_block.suffix_increase();
  return unnamed_block;
}

IIdentifier& netlist::Block::new_IId() {
  unnamed_instance.suffix_increase();
  return unnamed_instance;
}

VIdentifier& netlist::Block::new_VId() {
  unnamed_var.suffix_increase();
  return unnamed_var;
}

shared_ptr<Instance> netlist::Block::find_instance(const IIdentifier& key) const {
  return db_instance.find(key);
}

/* find a variable in current block*/
shared_ptr<Variable> netlist::Block::find_var(const VIdentifier& key) const {
  return db_var.find(key);
}

/* find a variable in the global environment, up to the module level */
shared_ptr<Variable> netlist::Block::gfind_var(const VIdentifier& key) const {
  shared_ptr<Variable> rv = db_var.find(key);
  if(rv.use_count() == 0 && father != NULL) return father->gfind_var(key);
  return rv;
}

ostream& netlist::Block::streamout(ostream& os, unsigned int indent) const {
  streamout(os, indent, false);
  return os;
}

ostream& netlist::Block::streamout(ostream& os, unsigned int indent, bool fl_prefix, bool is_else) const {

  if(!fl_prefix) os << string(indent, ' ');

  // the body part
  if(named || (!db_var.empty()) || (!db_instance.empty()) || (statements.size() != 1)) {
    os << "begin";
    if(named) os << ": " << name;
    os << endl;
  
    db_var.streamout(os, indent+2);
    if(db_var.size() > 0) os << endl;

  } else if((!is_else) || (statements.front()->get_type() != tIf))
    os << endl;



  if(named || (!db_var.empty()) || (!db_instance.empty()) || (statements.size() != 1) || 
     (!is_else) || (statements.front()->get_type() != tIf)) {
    // statements
    ctype_t mt = tUnknown;
    BOOST_FOREACH(const shared_ptr<NetComp>& it, statements) {
      ctype_t mt_nxt = it->get_type();
      if(mt != mt_nxt || mt != tAssign) {
        if(mt != tUnknown) os << endl;
        mt = mt_nxt;
      } 
      it->streamout(os, indent+2);
      if(it->get_type() == NetComp::tAssign &&
         !(static_pointer_cast<Assign>(it)->is_continuous()))
        os << ";" << endl;
    }
  } else {                      // if following an else
    boost::static_pointer_cast<IfState>(statements.front())->streamout(os, indent, true);
  }

  if(db_instance.size() != 0)
    db_instance.streamout(os, indent+2);

  if(db_func.size() != 0)
    db_func.streamout(os, indent+2);

  if(named || (!db_var.empty()) || (!db_instance.empty()) || (statements.size() != 1))
    os << string(indent, ' ') << "end" << endl;
  return os;
}

void netlist::Block::elab_inparse() {
  // classify and sort the current block
  //std::cout << "--------------- BLOCK ELAB INPARSE() " << std::endl;
  //std::cout << *this << std::endl;
  // to add list, used when some statements are replaced
  map<shared_ptr<NetComp>, list<shared_ptr<NetComp> > > to_add;
  typedef pair<const shared_ptr<NetComp>, list<shared_ptr<NetComp> > > to_add_type;
  // to delete list, when some statements are put in databases
  std::set<shared_ptr<NetComp> > to_del;

  // find out variables and database them
  BOOST_FOREACH(shared_ptr<NetComp> st, statements) {
    if(st->get_type() == tVariable) {
      SP_CAST(m, Variable, st);
      if(find_var(m->name)) {
        // double definition
        G_ENV->error(m->loc, "SYN-VAR-1", m->name.get_name(), toString(find_var(m->name)->loc));
      } else {
        if(m->exp) {            // handle the initial assignment
          if(m->vtype == Variable::TWire) { 
            // initial assignment to a wire is a continueous assignment
            shared_ptr<LConcatenation> lhs(new LConcatenation(m->loc, m->name));
            shared_ptr<Assign> asgn(new Assign(m->loc, lhs, m->exp, true));
            asgn->set_continuous();
            to_add[m].push_back(asgn);
            m->exp.reset();
          } else if(m->vtype != Variable::TParam && m->vtype != Variable::TLParam) {
            G_ENV->error(m->loc, "SYN-VAR-4", m->name.get_name());
            m->exp.reset();
          }
        }
        db_var.insert(m->name, m);
      }
      to_del.insert(st);
    }
  }
  BOOST_FOREACH(to_add_type m, to_add)
    statements.splice(std::find(statements.begin(), statements.end(), m.first), m.second);
  to_add.clear();
  BOOST_FOREACH(shared_ptr<NetComp> m, to_del)
    statements.remove(m);
  to_del.clear();

  //std::cout << "********** BLOCK ELAB INPARSE [after variable] *******" << std::endl;
  //std::cout << *this;

  // find out all instances and functions
  BOOST_FOREACH(shared_ptr<NetComp> st, statements) {
    if(st->get_type() == tInstance) {
      SP_CAST(m, Instance, st);
      if(!m->is_named()) {
        G_ENV->error(m->loc, "SYN-INST-1");
        m->set_default_name(new_IId());
      }
      if(db_instance.find(m->name)) {
        shared_ptr<Instance> m_inst = db_instance.find(m->name);
	if(m_inst) {
	  if(m_inst->is_named() && m->is_named()) {
	    G_ENV->error(m->loc, "SYN-INST-0", m->name.get_name(), toString(m_inst->loc));
	  }
	  if(m->is_named() && !m_inst->is_named()) {
	    db_instance.erase(m_inst->name);
	    IIdentifier m_iid = m_inst->name;
	    while(db_instance.find(m_iid)) {m_iid.suffix_increase(); }
	    m_inst->set_default_name(m_iid);
	    db_instance.insert(m_inst->name, m_inst);
	  } else {
	    IIdentifier m_iid = m->name;
	    while(db_instance.find(m_iid)) {m_iid.suffix_increase(); }
	    m->set_default_name(m_iid);
	  }
	}
      }
      db_instance.insert(m->name, m);
      //std::cout << "insert a new instance " << m->name << std::endl;
      to_del.insert(st);
    } else if (st->get_type() == tFunction) {
      SP_CAST(m, Function, st);
      if(db_func.count(m->fname)) {
        G_ENV->error(m->loc, "SYN-FUNC-0", m->fname.get_name(), toString(db_func.find(m->fname)->loc));
      } else {
        m->elab_inparse();
        db_func.insert(m->fname, m);
      }
      to_del.insert(st);
    } else if (st->get_type() == tBlock) {
      SP_CAST(m, Block, st);
      if(!m->is_named()) {
        m->elab_inparse();
        if(!m->db_var.empty()) { // the embedded block has variables
          for_each(m->db_var.begin_order(), m->db_var.end_order(),
                   [&](pair<const VIdentifier, shared_ptr<Variable> >& var) {
                     if(find_var(var.first))
                       G_ENV->error(var.second->loc, "SYN-VAR-1", var.first.get_name(), toString(find_var(var.first)->loc));
                     else
                       db_var.insert(var.first, var.second);
                   });
        }
        if(!m->db_instance.empty()) {
          for_each(m->db_instance.begin(), m->db_instance.end(),
                   [&](pair<const IIdentifier, shared_ptr<Instance> >& inst) {
                     if(db_instance.find(inst.first))
                       G_ENV->error(inst.second->loc, "SYN-INST-1", inst.first.get_name());
                     else
                       db_instance.insert(inst.first, inst.second);
                   });                       
        }
        if(!m->db_func.empty()) {
          for_each(m->db_func.begin(), m->db_func.end(),
                   [&](pair<const FIdentifier, shared_ptr<Function> >& func) {
                     if(db_func.find(func.first)) 
                       G_ENV->error(func.second->loc, "SYN-FUNC-0", func.first.get_name(), toString(db_func.find(func.first)->loc));
                     else
                       db_func.insert(func.first, func.second);
                   });
        }
        to_add[m] = m->statements;
        to_del.insert(st);
      }
    }
  }
  BOOST_FOREACH(to_add_type m, to_add)
    statements.splice(std::find(statements.begin(), statements.end(), m.first), m.second);
  to_add.clear();  
  BOOST_FOREACH(shared_ptr<NetComp> m, to_del)
    statements.remove(m);
  to_del.clear();  

  //std::cout << "********** BLOCK ELAB INPARSE [after all] *******" << std::endl;
  //std::cout << *this;
  
}

Block* netlist::Block::deep_copy(NetComp* bp) const {
  bool base_call = true;
  Block *rv;
  if(!bp) {
    rv = new Block();
    base_call = false;
  } else
    rv = static_cast<Block *>(bp); // C++ does not support multiple dispatch

  NetComp::deep_copy(rv);
  rv->name = name;
  rv->named = named;
  
  // data in Block
  // lambda expression, need C++0x support
  for_each(statements.begin(), statements.end(),
           [rv](const shared_ptr<NetComp>& comp) { 
             rv->statements.push_back(shared_ptr<NetComp>(comp->deep_copy(NULL))); 
           });
  
  DATABASE_DEEP_COPY_FUN(db_var,      VIdentifier, Variable,  rv->db_var       );
  rv->unnamed_block = unnamed_block;
  rv->unnamed_instance = unnamed_instance;
  rv->unnamed_var = unnamed_var;
 
  // special components in Blocks
  DATABASE_DEEP_COPY_FUN(db_instance,  IIdentifier,  Instance,  rv->db_instance );
  DATABASE_DEEP_COPY_FUN(db_func,      FIdentifier,  Function,  rv->db_func     );

  if(!base_call) {
    rv->set_father();
    rv->elab_inparse();
  }
  return rv;
}

void netlist::Block::set_father() {
  // macros defined in database.h
  DATABASE_SET_FATHER_FUN(db_var, VIdentifier, Variable, this);
  DATABASE_SET_FATHER_FUN(db_instance, IIdentifier, Instance, this);
  BOOST_FOREACH(shared_ptr<NetComp> st, statements) {
    st->set_father(this);
  }
}

void netlist::Block::db_register(int) {
  // the item in statements are duplicated in db_instance and db_other, therefore, only statements are executed
  // initialization of the variables in ablock are ignored as they are wire, reg and integers
  for_each(db_var.begin_order(), db_var.end_order(), [](pair<const VIdentifier, shared_ptr<Variable> >& m) {
      m.second->db_register(1);
    });
  for_each(statements.begin(), statements.end(), [](shared_ptr<NetComp>& m) {m->db_register(1);});
  for_each(db_instance.begin(), db_instance.end(), [](pair<const IIdentifier, shared_ptr<Instance> >& m) {
	m.second->db_register(1);
      });
  for_each(db_func.begin(), db_func.end(), [](pair<const FIdentifier, shared_ptr<Function> >& m) {
	m.second->db_register(1);
      });
}

void netlist::Block::db_expunge() {
  for_each(db_var.begin_order(), db_var.end_order(), [](pair<const VIdentifier, shared_ptr<Variable> >& m) {
      m.second->db_expunge();
    });
  for_each(statements.begin(), statements.end(), [](shared_ptr<NetComp>& m) {m->db_expunge();});
  for_each(db_instance.begin(), db_instance.end(), [](pair<const IIdentifier, shared_ptr<Instance> >& m) {
	m.second->db_expunge();
      });
  for_each(db_func.begin(), db_func.end(), [](pair<const FIdentifier, shared_ptr<Function> >& m) {
	m.second->db_expunge();
      });
}

shared_ptr<Block> netlist::Block::unfold() {
  map<shared_ptr<NetComp>, shared_ptr<Block> > replace_map;
  typedef pair<const shared_ptr<NetComp>, shared_ptr<Block> > replace_map_type;
  
  // unfold content
  BOOST_FOREACH(shared_ptr<NetComp> item, statements) {
    shared_ptr<Block> rB = item->unfold();
    if(rB) {                  // reduced to a block
      replace_map[item] = rB;
    }
  }

  // replace the statements
  BOOST_FOREACH(replace_map_type rB, replace_map) {
    elab_replace_statement(rB.first, rB.second);
  }

  if(named && get_type() != tModule && get_type() != tSeqBlock) { // block is named
    // add the block name to all instance, variables and functions
    // instances
    std::set<IIdentifier> instances;
    for_each(db_instance.begin(), db_instance.end(),
             [&](pair<const IIdentifier, shared_ptr<Instance> >& inst) {
               instances.insert(inst.first);
                 });
    BOOST_FOREACH(IIdentifier inst, instances) {
      IIdentifier new_id = inst;
      new_id.add_prefix(name.get_name());
      shared_ptr<Instance> pinst = db_instance.find(inst);
      pinst->set_name(new_id);
      db_instance.insert(new_id, pinst);
      db_instance.erase(inst);
    }

    // variables
    std::set<VIdentifier> variables;
    for_each(db_var.begin_order(), db_var.end_order(),
             [&](pair<const VIdentifier, shared_ptr<Variable> >& bvar) {
               variables.insert(bvar.first);
                 });
    BOOST_FOREACH(VIdentifier bvar, variables) {
      VIdentifier new_id = bvar;
      new_id.add_prefix(name.get_name());
      shared_ptr<Variable> pvar = db_var.find(bvar);
      pvar->name = new_id;
      db_var.insert(new_id, pvar);
      db_var.erase(bvar);
      replace_variable(bvar, new_id);
    }

    /*
    // functions
    std::set<FIdentifier> functions;
    for_each(db_func.begin(), db_func.end(),
             [&](pair<const FIdentifier, shared_ptr<Function> >& func) {
               functions.insert(func.first);
                 });
    BOOST_FOREACH(FIdentifier func, functions) {
      FIdentifier new_id = func;
      new_id.add_prefix(name.get_name());
      shared_ptr<Function> pfunc = db_func.find(func);
      pfunc->set_name(new_id);
      db_func.insert(new_id, pfunc);
      db_func.erase(func);
    }
    */

    // change the block to be unamed
    if(named && get_type() != tModule && get_type() != tSeqBlock)
      named = false;
  }

  return static_pointer_cast<Block>(shared_from_this());
}

bool netlist::Block::elaborate(std::set<shared_ptr<NetComp> >&,
                               map<shared_ptr<NetComp>, list<shared_ptr<NetComp> > >&) {

  std::set<shared_ptr<NetComp> > to_del;
  map<shared_ptr<NetComp>, list<shared_ptr<NetComp> > > to_add;

  // variables
  list<pair<const VIdentifier, shared_ptr<Variable> > >::iterator vit, vend;
  for(vit = db_var.begin_order(), vend = db_var.end_order(); vit!=vend; ++vit)
    if(!vit->second->elaborate(to_del, to_add))
      return false;

  // elaborate the internals
  BOOST_FOREACH(shared_ptr<NetComp> m, statements) {
    if(!m->elaborate(to_del, to_add))
      return false;
  }

  typedef pair<const shared_ptr<NetComp>, list<shared_ptr<NetComp> > > to_add_type;
  BOOST_FOREACH(to_add_type m, to_add) {
    list<shared_ptr<NetComp> >::iterator it = std::find(statements.begin(), statements.end(), m.first);
    BOOST_FOREACH(shared_ptr<NetComp> st, m.second) {
      if(st->get_type() == tVariable) {
        SP_CAST(mvar, Variable, st);
        db_var.insert(mvar->name, mvar);
      } else{
        statements.insert(it, st);
      }
    }
  }

  BOOST_FOREACH(shared_ptr<NetComp> m, to_del) {
    statements.erase(std::find(statements.begin(), statements.end(), m));
  }
  
  // instance
  map<IIdentifier, shared_ptr<Instance> >::iterator iit, iend;
  for(iit = db_instance.begin(), iend = db_instance.end(); iit!=iend; ++iit)
    if(!iit->second->elaborate(to_del, to_add))
      return false;

  // re-classify staements
  elab_inparse();
  // block structure may be changed
  set_father();

  return true;
}

SDFG::RForest netlist::Block::get_rforest() const {
  SDFG::RForest rv;
  BOOST_FOREACH(const shared_ptr<NetComp>& m, statements) {
    rv.combine(m->get_rforest(), true);
  }
  return rv;
}

void netlist::Block::gen_sdfg(shared_ptr<SDFG::dfgGraph> G) {

  // put all modules into the graph
  for_each(db_instance.begin(), db_instance.end(),
           [&](const pair<const IIdentifier, shared_ptr<Instance> >& m) {
             if(m.second->type == Instance::modu_inst) {
               shared_ptr<SDFG::dfgNode> n = G->add_node(m.first.get_name(), SDFG::dfgNode::SDFG_MODULE);
               n->ptr.insert(m.second);
               shared_ptr<Module> subMod = G_ENV->find_module(m.second->mname);
               if(subMod) { // has sub-module
                 n->child_name = m.second->mname.get_name();
                 n->child = subMod->extract_sdfg(true);
                 n->child->father = n.get();
               }
             } else {           // gate
               shared_ptr<SDFG::dfgNode> n = G->add_node(m.first.get_name(), SDFG::dfgNode::SDFG_GATE);
               n->ptr.insert(m.second);
             }
           });

  BOOST_FOREACH(shared_ptr<NetComp>& m, statements) {
    m->gen_sdfg(G);
  }
  for_each(db_instance.begin(), db_instance.end(),
           [&](const pair<const IIdentifier, shared_ptr<Instance> >& m) {
             m.second->gen_sdfg(G);
           });
}

void netlist::Block::replace_variable(const VIdentifier& var, const Number& num) {
  if(!db_var.count(var)) {
    BOOST_FOREACH(shared_ptr<NetComp> stm, statements) {
      stm->replace_variable(var, num);
    }
    for_each(db_instance.begin(), db_instance.end(),
             [&](const pair<const IIdentifier, shared_ptr<Instance> >& m) {
               m.second->replace_variable(var, num);
             });
    for_each(db_var.begin_order(), db_var.end_order(),
             [&](const pair<const VIdentifier, shared_ptr<Variable> >& m) {
               m.second->replace_variable(var, num);
             });
  }
}

void netlist::Block::replace_variable(const VIdentifier& var, const VIdentifier& nvar) {
  if(!db_var.count(var)) {
    BOOST_FOREACH(shared_ptr<NetComp> stm, statements) {
      stm->replace_variable(var, nvar);
    }
    for_each(db_instance.begin(), db_instance.end(),
             [&](const pair<const IIdentifier, shared_ptr<Instance> >& m) {
               m.second->replace_variable(var, nvar);
             });
    for_each(db_var.begin_order(), db_var.end_order(),
             [&](const pair<const VIdentifier, shared_ptr<Variable> >& m) {
               m.second->replace_variable(var, nvar);
             });
  }
}

shared_ptr<Expression> netlist::Block::get_combined_expression(const VIdentifier& target, std::set<string> s_set) {
  shared_ptr<Expression> rv;
  BOOST_FOREACH(shared_ptr<NetComp> stm, statements) {
    shared_ptr<Expression> mexp = stm->get_combined_expression(target, s_set);
    if(mexp) {
      if(rv) {
        G_ENV->error("ANA-SSA-0", get_module()->DFG->get_node(SDFG::divide_signal_name(target.get_name()))->get_full_name(), toString(*rv), toString(*mexp));
      }
      //assert(!rv);
      rv = mexp;
    }
  }
  //std::cout << "Block: (target)" << target << " Exp: " <<*rv << std::endl;
  return rv;
}

