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
    NETLIST_DEFAULT_CON(Instance, tInstance);
    Instance(const IIdentifier& nm, const list<pair<PoIdentifier, Expression> >& polist)
      : NetComp(tInstance), name(nm), port_list(polist), type(unknown) { }

    // helpers
    ostream& streamout(ostream& os) const;
    void set_mname(const MIdentifier& mod_name) { mname = mod_name; }
    void set_module_ptr(const shared_ptr<Module>& mp) { module_ptr = mp;}
    void set_para(const list<pair<VIdentifier, Expression> >& para ) { para_list = para; }

    // data
    IIdentifier name;
    MIdentifier mname;
    shared_ptr<Module> module_ptr;
    enum type_t { unknown, modu_inst, prim_inst, gate_inst} type;
    list<pair<PoIdentifier, Expression> > port_list;
    list<pair<VIdentifier, Expression> > para_list;
    
  };
  
  NETLIST_STREAMOUT(Instance);

}

#endif
