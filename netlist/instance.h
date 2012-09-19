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

#ifndef AV_H_INSTANCE_
#define AV_H_INSTANCE_

namespace netlist {
  class Instance : public NetComp {
  public:
    enum type_t { unknown_inst, modu_inst, prim_in_inst, prim_out_inst, gate_inst};
 
    Instance()
      : NetComp(tInstance), type(unknown_inst), named(false) {}

    Instance(const IIdentifier& nm, const std::list<boost::shared_ptr<PortConn> >& polist)
      : NetComp(tInstance), name(nm), port_list(polist), type(unknown_inst), named(true) { }

    Instance(
            const shell::location& lloc, 
            const IIdentifier& nm, 
            const std::list<boost::shared_ptr<PortConn> >& polist
            )
      : NetComp(tInstance, lloc), name(nm), port_list(polist), 
      type(unknown_inst), named(true) { }

    // for primary gates in most cases
    Instance(const IIdentifier&, const std::list<boost::shared_ptr<PortConn> >&, type_t);
    Instance(const shell::location&, const IIdentifier&, const std::list<boost::shared_ptr<PortConn> >&, type_t);
    Instance(const std::list<boost::shared_ptr<PortConn> >&, type_t);
    Instance(const shell::location&, const std::list<boost::shared_ptr<PortConn> >&, type_t);

    // inherit from NetComp
    NETLIST_SET_FATHER_DECL;
    NETLIST_STREAMOUT_DECL;
    NETLIST_CHECK_INPARSE_DECL;
    virtual Instance* deep_copy() const;
    virtual void db_register(int iod = 1);
    virtual void db_expunge();
    NETLIST_ELABORATE_DECL;
    // used to add new module to the elaboration process
    bool elaborate(std::deque<boost::shared_ptr<Module> >&, 
                   std::map<MIdentifier, boost::shared_ptr<Module> > &);
    NETLIST_GEN_SDFG;

    // helpers
    void set_mname(const MIdentifier& mod_name) { mname = mod_name; }
    void set_name(const IIdentifier& nm) { name = nm; named=true; }
    void set_default_name(const IIdentifier& nm) {name = nm; }
    //void set_module_ptr(const boost::shared_ptr<Module>& mp) { module_ptr = mp;}
    void set_para(const std::list<boost::shared_ptr<ParaConn> >& para ) { para_list = para; }
    bool is_named() const { return named; }
    bool update_ports();   /* update port directions in elaboration */

    // data
    IIdentifier name;
    MIdentifier mname;
    //boost::shared_ptr<Module> module_ptr;
    std::list<boost::shared_ptr<PortConn> > port_list;
    std::list<boost::shared_ptr<ParaConn> > para_list;
    type_t type;

  private:
    bool named;
    
  };
  
  NETLIST_STREAMOUT(Instance);

}

#endif

// Local Variables:
// mode: c++
// End:
