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
 * A module instance
 * 19/03/2012   Wei Song
 *
 *
 */

#ifndef _H_INSTANCE_
#define _H_INSTANCE_

namespace netlist {
  class Instance : public NetComp {
  public:
    enum type_t { unknown, modu_inst, prim_in_inst, prim_out_inst, gate_inst};
 
    Instance()
      : NetComp(tInstance), type(unknown) {}

    Instance(const IIdentifier& nm, const list<shared_ptr<PortConn> >& polist)
      : NetComp(tInstance), name(nm), port_list(polist), type(unknown), named(true) { }

    // for primary gates in most cases
    Instance(const IIdentifier&, const list<shared_ptr<PortConn> >&, type_t);

    // helpers
    NETLIST_STREAMOUT_FUN_DECL;
    void set_mname(const MIdentifier& mod_name) { mname = mod_name; }
    void set_name(const IIdentifier& nm) { name = nm; named=true; }
    void set_module_ptr(const shared_ptr<Module>& mp) { module_ptr = mp;}
    void set_para(const list<shared_ptr<ParaConn> >& para ) { para_list = para; }
    bool is_named() const { return named; }

    // data
    IIdentifier name;
    MIdentifier mname;
    shared_ptr<Module> module_ptr;
    list<shared_ptr<PortConn> > port_list;
    list<shared_ptr<ParaConn> > para_list;
    type_t type;

  private:
    bool named;
    
  };
  
  NETLIST_STREAMOUT(Instance);

}

#endif
