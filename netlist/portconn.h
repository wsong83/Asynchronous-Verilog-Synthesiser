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

#ifndef _H_PORT_CONNECTION_
#define _H_PORT_CONNECTION_

namespace netlist {

  template<class T>             /* a function to reduce the connection */
    void preduce(T* pp) {
    if(pp->type != T::type_t::CEXP) return;

    pp->exp->reduce();
    
    if(pp->exp->size() > 1) return; /* unable to reduce it to another type */

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
  }

  class PortConn {
  public:
    PortConn(const shared_ptr<Expression>& exp, int dir_m = 0) /* ordered connection */
      : dir(dir_m), exp(exp), type(CEXP), named(false) { reduce(); }
    
    PortConn(const shared_ptr<LConcatenation>& lval, int dir_m = 0) /* ordered connection */
      : dir(dir_m), exp(new Expression(lval)), type(CEXP), named(false) { reduce(); }
    
    PortConn()                  /* oredered open output connection */
      : dir(1), type(COPEN) {}
    
    PortConn(const PoIdentifier pn, const shared_ptr<Expression>& exp, int dir_m = 0) /* named connection */
      : pname(pn), dir(0), exp(exp), type(CEXP), named(true) { reduce(); }

    PortConn(const PoIdentifier pn)
      : pname(pn), dir(1), type(COPEN), named(true) {}

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

    ostream& streamout (ostream& os, unsigned int indent) const {
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
    PoIdentifier pname;         /* the port name in the module definition, or parameter name */
    int dir;                    /* direction, -1 in, 0 inout, 1 out */
    shared_ptr<Expression> exp; /* used when the connection is in general expression */
    VIdentifier var;            /* reduced to a single variable, one of the normal forms */
    Number num;                 /* reduced to a const number, one of the normal forms */
    enum type_t {CEXP, CVAR, CNUM, COPEN} type; /* connection type */

  private:
    bool named;

  };
  NETLIST_STREAMOUT(PortConn);

  class ParaConn {
  public:
    ParaConn()
      : type(COPEN), named(false) { }

    ParaConn(const shared_ptr<Expression>& exp) /* ordered connection */
      : exp(exp), type(CEXP), named(false) { reduce(); }

    ParaConn(const VIdentifier& pn, const shared_ptr<Expression>& exp) /* named connection */
      : pname(pn), exp(exp), type(CEXP), named(true) { reduce(); }

    ParaConn(const VIdentifier& pn) /* named connection */
      : pname(pn), type(COPEN), named(true) { }

    // helpers
    void reduce() { preduce<ParaConn>(this); }
    bool is_named() const { return named;}
    ostream& streamout (ostream& os, unsigned int indent) const {
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
    VIdentifier pname;          /* the port name in the module definition, or parameter name */
    shared_ptr<Expression> exp; /* used when the connection is in general expression */
    VIdentifier var;            /* reduced to a single variable, one of the normal forms */
    Number num;                 /* reduced to a const number, one of the normal forms */
    enum type_t {CEXP, CVAR, CNUM, COPEN} type; /* connection type */

  private:
    bool named;                 /* whether this port connection is already named */
  };
  NETLIST_STREAMOUT(ParaConn);

}

#endif
