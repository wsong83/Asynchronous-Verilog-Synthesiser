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
 * Port connection definition
 * 19/03/2012   Wei Song
 *
 *
 */

#ifndef AV_H_PORT_CONNECTION_
#define AV_H_PORT_CONNECTION_

namespace netlist {

  template<class T>             /* a function to reduce the connection */
    void preduce(T* pp) {
    if(pp->type != T::type_t::CEXP) return;

    pp->exp->reduce();
    
    /*
    if(pp->exp->size() > 1) return; 

    if(pp->exp->front()->get_type() == Operation::oVar) {
      pp->type = T::type_t::CVAR;
      pp->var = pp->exp->front()->get_var();
      pp->exp.reset();
      return;
    }

    if(pp->exp->front()->get_type() == Operation::oNum) {
      pp->type = T::type_t::CNUM;
      pp->num = pp->exp->front()->get_num();
      pp->exp.reset();
      return;
    }
    */

    if(pp->exp->is_valuable()) {
      pp->type = T::type_t::CNUM;
      pp->num = pp->exp->get_value();
      pp->exp.reset();
      return;
    }

    return;
  }

  class PortConn : public NetComp{
  public:
    PortConn(const boost::shared_ptr<Expression>& exp, int dir_m = 0) /* ordered connection */
      : NetComp(tPortConn), dir(dir_m), exp(exp), type(CEXP), named(false) { reduce(); }
    
    PortConn(const shell::location& lloc, 
             const boost::shared_ptr<Expression>& exp, 
             int dir_m = 0) /* ordered connection */
      : NetComp(tPortConn, lloc), dir(dir_m), exp(exp), type(CEXP), named(false) { reduce(); }
    
    PortConn(const boost::shared_ptr<LConcatenation>& lval, int dir_m = 0) /* ordered connection */
      : NetComp(tPortConn), dir(dir_m), exp(new Expression(lval)), type(CEXP), named(false) { reduce(); }
    
    PortConn(const shell::location& lloc, 
             const boost::shared_ptr<LConcatenation>& lval, 
             int dir_m = 0) /* ordered connection */
      : NetComp(tPortConn, lloc), dir(dir_m), exp(new Expression(lval)), type(CEXP), named(false) { reduce(); }
    
    PortConn()                  /* oredered open output connection */
      : NetComp(tPortConn), dir(1), type(COPEN) {}
    
    PortConn(const VIdentifier pn, const boost::shared_ptr<Expression>& exp, int dir_m = 0) /* named connection */
      : NetComp(tPortConn), pname(pn), dir(0), exp(exp), type(CEXP), named(true) { reduce(); }

    PortConn(const shell::location& lloc, 
             const VIdentifier pn, 
             const boost::shared_ptr<Expression>& exp, int dir_m = 0) /* named connection */
      : NetComp(tPortConn, lloc), pname(pn), dir(0), exp(exp), type(CEXP), named(true) { reduce(); }

    PortConn(const VIdentifier pn)
      : NetComp(tPortConn), pname(pn), dir(1), type(COPEN), named(true) {}

    PortConn(const shell::location& lloc, const VIdentifier pn)
      : NetComp(tPortConn, lloc), pname(pn), dir(1), type(COPEN), named(true) {}

    // helpers
    void reduce() { preduce<PortConn>(this); }
    bool is_named() const { return named;}
    void set_in() { dir = -1; }
    void set_out() { dir = 1; }
    void set_inout() { dir = 0; }
    void set_dir(int mdir) { dir = mdir; }
    int get_dir() const { return dir;}
    bool is_in() const { return dir == -1; }
    bool is_out() const { return dir == 1; }
    bool is_inout() const { return dir == 0; }
    void set_father(Block* pf) { 
      if(father == pf) return;
      father = pf;
      pname.set_father(pf);
      if(exp.use_count() != 0) exp->set_father(pf);
      var.set_father(pf);
    }
    bool check_inparse() {
      switch(type) {
      case CEXP: return exp->check_inparse();
      case CVAR: return var.check_inparse();
      default: return true;
      }
    }
    
    virtual PortConn* deep_copy() const {
      PortConn* rv = new PortConn();
      rv->loc = loc;
      rv->pname = pname;
      rv->named = named;
      rv->dir = dir;
      if(exp.use_count() != 0) rv->exp.reset(exp->deep_copy());
      VIdentifier *pm = var.deep_copy();
      rv->var = *pm;
      delete pm;
      rv->num = num;
      rv->type = type;
      return rv;
    } 

    virtual void db_register(int iod = 1) {
      switch(type) {
      case CEXP: 
        if(dir >= 0)  {
          assert(0 == "expression in a port connection when it is an output port, not support yet!");
          exp->db_register(0);
        }
        if(dir <= 0) exp->db_register(1);
        break;
      case CVAR: 
        if(dir >= 0) var.db_register(0);
        if(dir <= 0) var.db_register(1);
        break;
      default: ;
      }
    }
      
    virtual void db_expunge() {
      switch(type) {
      case CEXP: exp->db_expunge(); break;
      case CVAR: var.db_expunge(); break;
      default: ;
      }
    }

    std::ostream& streamout (std::ostream& os, unsigned int indent) const {
      if(named) os << "." << pname.name << "(";
      switch(type) {
      case CEXP: os << *exp; break;
      case CVAR: os << var; break;
      case CNUM: os << num; break;
      case COPEN: os << " "; break;
      default:
        assert(0 == "wrong connection type!");
      }
      if(named) os << ")";
      return os;
    }

    // date
    VIdentifier pname;                    /* the port name in the module definition, or parameter name */
    int dir;                              /* direction, -1 in, 0 inout, 1 out */
    boost::shared_ptr<Expression> exp;    /* used when the connection is in general expression */
    VIdentifier var;                      /* reduced to a single variable, one of the normal forms */
    Number num;                           /* reduced to a const number, one of the normal forms */
    enum type_t {CEXP, CVAR, CNUM, COPEN} type; /* connection type */

  private:
    bool named;

  };
  NETLIST_STREAMOUT(PortConn);

  class ParaConn : public NetComp {
  public:
    ParaConn()
      : NetComp(tParaConn), type(COPEN), named(false) { }

    ParaConn(const boost::shared_ptr<Expression>& exp) /* ordered connection */
      : NetComp(tParaConn), exp(exp), type(CEXP), named(false) { reduce(); }

    ParaConn(const shell::location& lloc, const boost::shared_ptr<Expression>& exp) /* ordered connection */
      : NetComp(tParaConn, lloc), exp(exp), type(CEXP), named(false) { reduce(); }

    ParaConn(const VIdentifier& pn, const boost::shared_ptr<Expression>& exp) /* named connection */
      : NetComp(tParaConn), pname(pn), exp(exp), type(CEXP), named(true) { reduce(); }

    ParaConn(const shell::location& lloc, const VIdentifier& pn, const boost::shared_ptr<Expression>& exp) /* named connection */
      : NetComp(tParaConn, lloc), pname(pn), exp(exp), type(CEXP), named(true) { reduce(); }

    ParaConn(const VIdentifier& pn) /* named connection */
      : NetComp(tParaConn), pname(pn), type(COPEN), named(true) { }

    ParaConn(const shell::location& lloc, const VIdentifier& pn) /* named connection */
      : NetComp(tParaConn, lloc), pname(pn), type(COPEN), named(true) { }

    // helpers
    void reduce() { preduce<ParaConn>(this); }
    bool is_named() const { return named;}

    void set_father(Block* pf) { 
      if(father == pf) return;
      father = pf;
      pname.set_father(pf);
      if(exp.use_count() != 0) exp->set_father(pf);
      var.set_father(pf);
    }

    bool check_inparse() {
      switch(type) {
      case CEXP: return exp->check_inparse();
      case CVAR: return var.check_inparse();
      default: return true;
      }
    }

    virtual ParaConn* deep_copy() const {
      ParaConn* rv = new ParaConn();
      rv->loc = loc;
      rv->pname = pname;
      rv->named = named;
      if(exp.use_count() != 0) rv->exp.reset(exp->deep_copy());
      VIdentifier *pm = var.deep_copy();
      rv->var = *pm;
      delete pm;
      rv->num = num;
      rv->type = type;
      return rv;
    } 

    virtual void db_register(int iod = 1) {
      switch(type) {
      case CEXP: exp->db_register(1); break;
      case CVAR: var.db_register(1); break;
      default: ;
      }
    }
      
    virtual void db_expunge() {
      switch(type) {
      case CEXP: exp->db_expunge(); break;
      case CVAR: var.db_expunge(); break;
      default: ;
      }
    }

    std::ostream& streamout (std::ostream& os, unsigned int indent) const {
      if(named) os << "." << pname.name << "(";
      switch(type) {
      case CEXP: os << *exp; break;
      case CVAR: os << var; break;
      case CNUM: os << num; break;
      case COPEN: os << " "; break;
      default:
        assert(0 == "wrong connection type!");
      }
      if(named) os << ")";
      return os;
    }

    // date
    VIdentifier pname;                 /* the port name in the module definition, or parameter name */
    boost::shared_ptr<Expression> exp; /* used when the connection is in general expression */
    VIdentifier var;                   /* reduced to a single variable, one of the normal forms */
    Number num;                        /* reduced to a const number, one of the normal forms */
    enum type_t {CEXP, CVAR, CNUM, COPEN} type; /* connection type */

  private:
    bool named;                 /* whether this port connection is already named */
  };
  NETLIST_STREAMOUT(ParaConn);

}

#endif
