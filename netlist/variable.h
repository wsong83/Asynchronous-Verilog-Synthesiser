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
 * Variable
 * 27/02/2012   Wei Song
 *
 *
 */

#ifndef AV_H_VA_REGISTER_
#define AV_H_VA_REGISTER_

namespace netlist {
  
  class Variable : public NetComp {
  public:

    enum vtype_t {
      TVar, TWire, TReg, TParam, TGenvar
    } vtype;

    Variable() : NetComp(tVariable), uid(0) {}
    Variable(const shell::location& lloc) : NetComp(tVariable, lloc), uid(0) {}
    Variable(const VIdentifier& id, vtype_t mtype = TVar)
      : NetComp(tVariable), vtype(mtype), name(id), uid(0) {}
    Variable(const Port& p)
      : NetComp(tVariable, p.loc), vtype(TWire), uid(0) {
      VIdentifier *newName = p.name.deep_copy();
      name = *newName;
      delete newName;
    }
    Variable(const shell::location& lloc, const VIdentifier& id, vtype_t mtype = TVar)
      : NetComp(tVariable, lloc), vtype(mtype), name(id), uid(0) {}
    Variable(const VIdentifier& id, const boost::shared_ptr<Expression>& expp, vtype_t mtype = TVar)
      : NetComp(tVariable), vtype(mtype), name(id), uid(0), exp(expp) {}
    Variable(const shell::location& lloc, const VIdentifier& id, 
             const boost::shared_ptr<Expression>& expp, vtype_t mtype = TVar)
    : NetComp(tVariable, lloc), vtype(mtype), name(id), uid(0), exp(expp) {}

    // inherit from NetComp
    NETLIST_SET_FATHER_DECL;
    NETLIST_STREAMOUT_DECL;
    NETLIST_CHECK_INPARSE_DECL;
    virtual Variable* deep_copy() const;
    virtual void db_register(int iod = 1);
    virtual void db_expunge();
    NETLIST_ELABORATE_DECL;

    // helpers
    void set_value(const Number&); /* reset the value of this variable */
    void set_value(const VIdentifier&);
    void set_value(const boost::shared_ptr<Expression>&);
    void set_wire() { vtype = TWire; }
    void set_reg() { vtype = TReg; }
    void set_para() { vtype = TParam; }
    void set_genvar() { vtype = TGenvar; }
    vtype_t get_vtype() const { return vtype; }
    bool update();  /* recalculate the value and update all fanouts, true when it is reduced to a number */
    bool is_valuable() const {  return (exp.use_count() != 0) ? exp->is_valuable() : false; }
    Number get_value() const {  return (exp.use_count() != 0) ? exp->get_value() : 0; }
    std::string get_short_string() const;

    VIdentifier name;
    std::map<unsigned int, VIdentifier *> fan[2]; /* fan[0] for fanin, fan[1] for fanout */
    unsigned int get_id();

  private:

    // helper
    bool driver_and_load_checker();

    unsigned int uid;
    boost::shared_ptr<Expression> exp;

  };

  NETLIST_STREAMOUT(Variable);


}

#endif
