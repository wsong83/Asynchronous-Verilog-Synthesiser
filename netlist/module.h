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
 * Definition of netlist components.
 * 14/02/2012   Wei Song
 *
 *
 */

#ifndef AV_H_MODULE_
#define AV_H_MODULE_

#include <deque>
#include <set>

namespace netlist {
  
  class Module : public Block {
  public:
    Module();
    Module(const MIdentifier&);
    Module(const shell::location&, const MIdentifier&);
    Module(const MIdentifier&, const boost::shared_ptr<Block>&);
    Module(const shell::location&, const MIdentifier&, const boost::shared_ptr<Block>&);
    Module(const MIdentifier&, const std::list<boost::shared_ptr<Port> >&, const boost::shared_ptr<Block>&);
    Module(const shell::location&, const MIdentifier&, const std::list<boost::shared_ptr<Port> >&, const boost::shared_ptr<Block>&);
    Module(const MIdentifier&, const std::list<boost::shared_ptr<Variable> >&, 
           const std::list<boost::shared_ptr<Port> >&, const boost::shared_ptr<Block>&);
    Module(const shell::location&, const MIdentifier&, 
           const std::list<boost::shared_ptr<Variable> >&, 
           const std::list<boost::shared_ptr<Port> >&, const boost::shared_ptr<Block>&);

    // inherit from NetComp
    NETLIST_STREAMOUT_DECL;
    using NetComp::set_father;
    virtual Module* deep_copy() const;
    NETLIST_DB_DECL;
    // build up the internal databases
    virtual void elab_inparse(); /* resolve the content in statements during parsing */
    /* elaborate the design */
    bool elaborate(std::deque<boost::shared_ptr<Module> >&,
                   std::map<MIdentifier, boost::shared_ptr<Module> > &); 

    // helpers
    virtual void set_name(const MIdentifier& nm) { name = nm; named=true;}
    virtual boost::shared_ptr<Variable>  find_var       (const VIdentifier&) const; /* find a variable */
    virtual boost::shared_ptr<Port>      find_port      (const VIdentifier&) const; /* find an item in db_other */
    /* find a variable in the global environment, up to the module level */
    virtual boost::shared_ptr<Variable>  gfind_var      (const VIdentifier&) const; 
    virtual boost::shared_ptr<NetComp>   search         (const std::string&) const; /* find any item using its name */
    virtual void set_father();                             /* set the father pointer to all sub-elements */
    /* return a pointer of the top-level module */
    virtual Block* get_module() { return this; }
    /* calculate the module name according to the parameters */
    bool calculate_name( std::string&, 
                         const std::list<boost::shared_ptr<ParaConn> >& 
                         mplist = std::list<boost::shared_ptr<ParaConn> >()) const;
    /* get a list of all the hierarchical modules using this one as the top */
    void get_hier(std::list<boost::shared_ptr<Module> >&, 
                  std::set<MIdentifier> &) const;

    // extract data flow graph
    boost::shared_ptr<SDFG::dfgGraph> extract_sdfg(bool);

    // data
    MIdentifier name;
    DataBase<VIdentifier, Port, true>      db_port;      /* input and output ports, ordered */
    DataBase<VIdentifier, Variable, true>  db_param;     /* parameters, ordered */

    // DFG graphs
    boost::shared_ptr<SDFG::dfgGraph> DFG; // the DFG graph of this module (sub-module is embedded, so it is a full graph)
    boost::shared_ptr<SDFG::dfgGraph> RRG; // the register relation graph of this module

  private:
    // only used in constructors
    void init_port_list(const std::list<boost::shared_ptr<Port> >&);
    void init_param_list(const std::list<boost::shared_ptr<Variable> >&);
  };

  NETLIST_STREAMOUT(Module);
}

#endif

// Local Variables:
// mode: c++
// End:
