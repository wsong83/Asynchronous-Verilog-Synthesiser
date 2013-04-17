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
 * Ports
 * 16/02/2012   Wei Song
 *
 *
 */

#ifndef AV_H_PORT_
#define AV_H_PORT_

namespace netlist {

  class Port : public NetComp {
  public:
    // constructors
    Port(const VIdentifier&);
    Port(const shell::location&, const VIdentifier&);
    Port(const shell::location&);

    // inherit from NetComp
    NETLIST_STREAMOUT_DECL;
    NETLIST_SET_FATHER_DECL;
    virtual Port* deep_copy() const;
    NETLIST_DB_DECL;
    NETLIST_ELABORATE_DECL;
    virtual boost::shared_ptr<Expression> get_combined_expression(const VIdentifier&);
   
    // helpers
    void set_in() { dir = -1; }
    void set_out() { dir = 1; }
    void set_inout() { dir = 0; }
    void set_dir(int mdir) { dir = mdir; }
    void set_signed() { signed_flag = true; }
    bool is_signed() const { return signed_flag; }
    int get_dir() const { return dir;}
    bool is_in() const { return dir == -1; }
    bool is_out() const { return dir == 1; }
    bool is_inout() const { return dir == 0; }

    VIdentifier name;
    int ptype;                  /* 0, unknown/wire; 1 reg */
  private:
    int dir;                    /* -1 in, 0 inout, 1 out, -2 default */
    bool signed_flag;
  };
  NETLIST_STREAMOUT(Port);

}

#endif
