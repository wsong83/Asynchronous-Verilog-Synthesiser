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
        
    if(!pp->exp->is_singular()) return; 

    if(pp->exp->get_op().get_type() == Operation::oVar) {
      pp->type = T::type_t::CVAR;
      pp->var = pp->exp->get_op().get_var();
      pp->exp.reset();
      return;
    }

    if(pp->exp->get_op().get_type() == Operation::oNum) {
      pp->type = T::type_t::CNUM;
      pp->num = pp->exp->get_op().get_num();
      pp->exp.reset();
      return;
    }
    
    if(pp->exp->is_valuable()) {
      pp->type = T::type_t::CNUM;
      pp->num = pp->exp->get_value();
      pp->exp.reset();
      return;
    }

    return;
  }

  class PortConn : public NetComp {
  public:
    PortConn(const boost::shared_ptr<Expression>&, int dir_m = 0);    
    PortConn(const shell::location&, const boost::shared_ptr<Expression>&, int dir_m = 0);    
    PortConn(const boost::shared_ptr<LConcatenation>&, int dir_m = 0);    
    PortConn(const shell::location&, const boost::shared_ptr<LConcatenation>&, int dir_m = 0);    
    PortConn();
    PortConn(const VIdentifier&, const boost::shared_ptr<Expression>&, int dir_m = 0);
    PortConn(const shell::location&, const VIdentifier&, const boost::shared_ptr<Expression>&, int dir_m = 0);
    PortConn(const VIdentifier&);
    PortConn(const shell::location&, const VIdentifier&);

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
    NETLIST_SET_FATHER_DECL;
    NETLIST_REPLACE_VARIABLE;
    virtual PortConn* deep_copy(NetComp*) const;
    NETLIST_DB_DECL;
    NETLIST_ELABORATE_DECL;
    NETLIST_STREAMOUT_DECL;

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
    ParaConn();
    ParaConn(const boost::shared_ptr<Expression>&);
    ParaConn(const shell::location&, const boost::shared_ptr<Expression>&);
    ParaConn(const VIdentifier&, const boost::shared_ptr<Expression>&);
    ParaConn(const shell::location&, const VIdentifier&, const boost::shared_ptr<Expression>&);
    ParaConn(const VIdentifier&);
    ParaConn(const shell::location&, const VIdentifier&);

    // helpers
    void reduce() { preduce<ParaConn>(this); }
    bool is_named() const { return named;}

    NETLIST_SET_FATHER_DECL;
    NETLIST_REPLACE_VARIABLE;
    virtual ParaConn* deep_copy(NetComp*) const;
    NETLIST_DB_DECL;
    NETLIST_ELABORATE_DECL;
    NETLIST_STREAMOUT_DECL;
    
    // data
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
// Local Variables:
// mode: c++
// End:
