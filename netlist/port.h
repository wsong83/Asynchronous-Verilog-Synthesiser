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
 * Ports
 * 16/02/2012   Wei Song
 *
 *
 */

#ifndef _H_PORT_
#define _H_PORT_

namespace netlist {

  class Port : public NetComp {
  public:
    // constructors
    Port(const PoIdentifier&);
    Port(const location&, const PoIdentifier&);

    // helpers
    void set_in() { dir = -1; }
    void set_out() { dir = 1; }
    void set_inout() { dir = 0; }
    void set_dir(int mdir) { dir = mdir; }
    int get_dir() const { return dir;}
    bool is_in() const { return dir == -1; }
    bool is_out() const { return dir == 1; }
    bool is_inout() const { return dir == 0; }
    NETLIST_STREAMOUT_FUN_DECL;
   
    PoIdentifier name;

  private:
    int dir;                    /* -1 in, 0 inout, 1 out */
  };
  NETLIST_STREAMOUT(Port);

}

#endif
