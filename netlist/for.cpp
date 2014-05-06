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
 * for statements
 * 03/04/2012   Wei Song
 *
 *
 */

#include "component.h"
#include "shell/env.h"
#include <algorithm>
#include <map>
#include <boost/foreach.hpp>

using namespace netlist;
using std::ostream;
using std::string;
using boost::shared_ptr;
using shell::location;
using std::vector;
using std::list;
using std::map;
using std::pair;

netlist::ForState::ForState(
                            const shared_ptr<Assign>& init,
                            const shared_ptr<Expression>& cond, 
                            const shared_ptr<Assign>& incr, 
                            const shared_ptr<Block>& body
                            )
  : NetComp(NetComp::tFor), init(init), cond(cond), incr(incr), body(body), named(false)
{
  body->elab_inparse();
}

netlist::ForState::ForState(
                            const location& lloc,
                            const shared_ptr<Assign>& init, 
                            const shared_ptr<Expression>& cond, 
                            const shared_ptr<Assign>& incr, 
                            const shared_ptr<Block>& body
                            )
  : NetComp(NetComp::tFor, lloc), init(init), cond(cond), incr(incr), body(body), named(false)
{
  body->elab_inparse();
}

void netlist::ForState::set_father(Block *pf) {
  if(father == pf) return;
  father = pf;
  name.set_father(pf);
  init->set_father(pf);
  cond->set_father(pf);
  incr->set_father(pf);
  body->set_father(pf);
}

ostream& netlist::ForState::streamout(ostream& os, unsigned int indent) const {
  assert(init.use_count() != 0);

  os << string(indent, ' ') << "for (" << *init << "; " << *cond << "; " << *incr << ") ";
  body->streamout(os, indent, true);

  return os;
}

ForState* netlist::ForState::deep_copy(NetComp* bp) const {
  ForState *rv;
  if(!bp) rv = new ForState(loc);
  else    rv = static_cast<ForState *>(bp); // C++ does not support multiple dispatch
  NetComp::deep_copy(rv);

  rv->name = name;
  rv->named = named;
  
  if(init) rv->init.reset(init->deep_copy(NULL));
  if(cond) rv->cond.reset(cond->deep_copy(NULL));
  if(incr) rv->incr.reset(incr->deep_copy(NULL));
  if(body) rv->body.reset(body->deep_copy(NULL));

  return rv;
}

void netlist::ForState::db_register(int) {
  if(init) init->db_register(1);
  if(cond) cond->db_register(1);
  if(incr) incr->db_register(1);
  if(body) body->db_register(1);
}

void netlist::ForState::db_expunge() {
  if(init) init->db_expunge();
  if(cond) cond->db_expunge();
  if(incr) incr->db_expunge();
  if(body) body->db_expunge();
}

bool netlist::ForState::elaborate(std::set<shared_ptr<NetComp> >& to_del,
                                  map<shared_ptr<NetComp>, list<shared_ptr<NetComp> > >& to_add) {
  // set up the initial assignment
  if(!init) {
    G_ENV->error(loc, "ELAB-FOR-0");
    return false;
  } else if(!init->rexp->is_valuable()) {
    G_ENV->error(init->loc, "ELAB-FOR-1", toString(*(init->rexp)));
    return false;
  } else if(init->lval->size() != 1) {
    G_ENV->error(init->loc, "ELAB-FOR-2", toString(*(init->lval)));
    return false;
  }

  VIdentifier& var = init->lval->front();
  Number num = init->rexp->get_value();

  // unfold the for statement
  shared_ptr<Expression> m_cond(cond->deep_copy(NULL));
  m_cond->replace_variable(var, num);
  m_cond->reduce();
  if(!m_cond->is_valuable()) {
    G_ENV->error(cond->loc, "ELAB-FOR-3", toString(*cond));
    return false;
  }

  while(m_cond->get_value().is_true()) {
    shared_ptr<Block> m_blk(body->deep_copy(NULL));
    m_blk->replace_variable(var, num);
    to_add[get_sp()].push_back(m_blk);
    m_blk->set_father(father);
    m_blk->db_register(1);
    m_blk->elaborate(to_del, to_add);

    // increment
    if(!incr || incr->lval->size() != 1 || incr->lval->front() != var) {
      G_ENV->error(cond->loc, "ELAB-FOR-4", toString(*incr));
      return false;
    }
    shared_ptr<Assign> m_incr(incr->deep_copy(NULL));
    m_incr->rexp->replace_variable(var, num);
    m_incr->rexp->reduce();
    if(!m_incr->rexp->is_valuable()) {
      G_ENV->error(cond->loc, "ELAB-FOR-4", toString(*incr));
      return false;
    }
    
    // update num
    num = m_incr->rexp->get_value();
    m_cond.reset(cond->deep_copy(NULL));
    m_cond->replace_variable(var, num);
    m_cond->reduce();
  }

  // delete this
  to_del.insert(get_sp());

  return true;
}

shared_ptr<Block> netlist::ForState::unfold() {
  // set up the initial assignment
  if(!init) {
    G_ENV->error(loc, "ELAB-FOR-0");
    return shared_ptr<Block>();
  } else if(!init->rexp->is_valuable()) {
    G_ENV->error(init->loc, "ELAB-FOR-1", toString(*(init->rexp)));
    return shared_ptr<Block>();
  } else if(init->lval->size() != 1) {
    G_ENV->error(init->loc, "ELAB-FOR-2", toString(*(init->lval)));
    return shared_ptr<Block>();
  }

  VIdentifier& var = init->lval->front();
  Number num = init->rexp->get_value();
  
  // unfold the for statement
  shared_ptr<Expression> m_cond(cond->deep_copy(NULL));
  m_cond->replace_variable(var, num);
  m_cond->reduce();
  if(!m_cond->is_valuable()) {
    G_ENV->error(cond->loc, "ELAB-FOR-3", toString(*cond));
    return shared_ptr<Block>();
  }

  // the new body
  shared_ptr<Block> newBody(new Block(body->loc));
  bool body_named = body->is_named();
  
  while(m_cond->get_value().is_true()) {
    // replace the index variable in the body
    shared_ptr<Block> m_blk(body->deep_copy(NULL));
    m_blk->replace_variable(var, num);

    // set up the new names for instances
    if(body_named) {
      // prepare the prefix for this iteration
      string locPrefix = body->name.get_name() + num.get_value().get_str(10) + ".";

      // define functions in a named for loop is not supported
      if(m_blk->db_func.size())
        G_ENV->error(m_blk->loc, "ELAB-FOR-5");

      if(m_blk->db_instance.size()) { // rename module instances
        // set to store the old instances
        std::set<IIdentifier> instances;
        // rename the instances
        for_each(m_blk->db_instance.begin(), m_blk->db_instance.end(),
                 [&](pair<const IIdentifier, shared_ptr<Instance> >& inst) {
                   instances.insert(inst.first);
                     });
        BOOST_FOREACH(IIdentifier inst, instances) {
          IIdentifier new_id = inst;
          new_id.add_prefix(locPrefix);
          shared_ptr<Instance> pinst = m_blk->db_instance.find(inst);
          pinst->set_name(new_id);
          m_blk->db_instance.insert(new_id, pinst);
          m_blk->db_instance.erase(inst);
        }
      }

      if(m_blk->db_var.size()) {  // rename variables
        // set to store the old variables
        std::set<VIdentifier> variables;
        // rename the variables
        for_each(m_blk->db_var.begin_order(), m_blk->db_var.end_order(),
                 [&](pair<const VIdentifier, shared_ptr<Variable> >& bvar) {
                   variables.insert(bvar.first);
                     });
        BOOST_FOREACH(VIdentifier bvar, variables) {
          VIdentifier new_id = bvar;
          new_id.add_prefix(locPrefix);
          shared_ptr<Variable> pvar = m_blk->db_var.find(bvar);
          pvar->name = new_id;
          m_blk->db_var.insert(new_id, pvar);
          m_blk->db_var.erase(bvar);
          m_blk->replace_variable(bvar, new_id);
        }
      }
    }

    // copy the statements in the body to the new body
    newBody->elab_add_block(m_blk);

    // increment
    if(!incr || incr->lval->size() != 1 || incr->lval->front() != var) {
      G_ENV->error(cond->loc, "ELAB-FOR-4", toString(*incr));
      return shared_ptr<Block>();
    }
    shared_ptr<Assign> m_incr(incr->deep_copy(NULL));
    m_incr->rexp->replace_variable(var, num);
    m_incr->rexp->reduce();
    if(!m_incr->rexp->is_valuable()) {
      G_ENV->error(cond->loc, "ELAB-FOR-4", toString(*incr));
      return shared_ptr<Block>();
    }
    
    // update num
    num = m_incr->rexp->get_value();
    m_cond.reset(cond->deep_copy(NULL));
    m_cond->replace_variable(var, num);
    m_cond->reduce();
  }

  return newBody;

}

void netlist::ForState::replace_variable(const VIdentifier& var, const Number& num) {
  if(init) init->replace_variable(var, num);
  if(cond) cond->replace_variable(var, num);
  if(incr) incr->replace_variable(var, num);
  body->replace_variable(var, num);
}

void netlist::ForState::replace_variable(const VIdentifier& var, const VIdentifier& nvar) {
  if(init) init->replace_variable(var, nvar);
  if(cond) cond->replace_variable(var, nvar);
  if(incr) incr->replace_variable(var, nvar);
  body->replace_variable(var, nvar);
}
