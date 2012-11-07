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

    Variable();
    Variable(const shell::location&);
    Variable(const VIdentifier&, vtype_t mtype = TVar);
    Variable(const Port&);
    Variable(const shell::location&, const VIdentifier&, vtype_t mtype = TVar);
    Variable(const VIdentifier&, const boost::shared_ptr<Expression>&, vtype_t mtype = TVar);
    Variable(const shell::location&, const VIdentifier&, const boost::shared_ptr<Expression>&, vtype_t mtype = TVar);

    // inherit from NetComp
    NETLIST_SET_FATHER_DECL;
    NETLIST_STREAMOUT_DECL;
    virtual Variable* deep_copy() const;
    NETLIST_DB_DECL;
    NETLIST_ELABORATE_DECL;
    NETLIST_REPLACE_VARIABLE;

    // helpers
    void set_value(const Number&); /* reset the value of this variable */
    void set_value(const VIdentifier&);
    void set_value(const boost::shared_ptr<Expression>&);
    void set_wire() { vtype = TWire; }
    void set_reg() { vtype = TReg; }
    void set_para() { vtype = TParam; }
    void set_genvar() { vtype = TGenvar; }
    void set_signed() { signed_flag = true; }
    vtype_t get_vtype() const { return vtype; }
    bool update();  /* recalculate the value and update all fanouts, true when it is reduced to a number */
    bool is_valuable() const {  return exp ? exp->is_valuable() : false; }
    Number get_value() const {  return exp ? exp->get_value() : 0; }
    std::string get_short_string() const;
    bool check_post_elaborate(); /* check fan-in/out */
    bool is_useless() const { return fan[0].size() == 0 && fan[1].size() == 1; }

    VIdentifier name;
    std::map<unsigned int, VIdentifier *> fan[2]; /* fan[0] for fanin, fan[1] for fanout */
    unsigned int get_id();
    boost::shared_ptr<Expression> exp;

  private:

    // helper
    bool driver_and_load_checker() const;

    unsigned int uid;
    
    bool signed_flag;
  };

  NETLIST_STREAMOUT(Variable);
}

#endif
// Local Variables:
// mode: c++
// End:
