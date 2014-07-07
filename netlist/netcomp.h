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
 * The root class for all netlist component.
 * 04/04/2012   Wei Song
 *
 *
 */

#ifndef AV_H_AV_NETCOMP_
#define AV_H_AV_NETCOMP_

#include <boost/enable_shared_from_this.hpp>
#include <boost/tuple/tuple.hpp>
#include "shell/location.h"
#include <set>

// forward declarations
namespace SDFG {
  class dfgNode;
  class dfgEdge;
  class dfgGraph;
  class RForest;
  class RTree;
}

// function macro for stream out operator <<
#ifndef NETLIST_STREAMOUT
#define NETLIST_STREAMOUT(COMP)                                          \
  inline std::ostream& operator<< ( std::ostream& os, const COMP& rhs) { \
    return rhs.streamout(os, (unsigned int)(0));                         \
  }
#endif

#ifndef NETLIST_DEFAULT_CON
#define NETLIST_DEFAULT_CON(COMP, CT) COMP() : NetComp(NetComp::CT) { }
#endif

#ifndef NETLIST_DEFAULT_CON_WL
#define NETLIST_DEFAULT_CON_WL(COMP, CT) COMP(const shell::location& lloc) : NetComp(NetComp::CT, lloc) { }
#endif

#ifndef NETLIST_STREAMOUT_DECL
#define NETLIST_STREAMOUT_DECL                    \
  virtual std::ostream& streamout (std::ostream&, unsigned int) const
#endif

#ifndef SP_CAST
#define SP_CAST(m, T, d)                      \
  boost::shared_ptr<T > m = boost::static_pointer_cast<T >(d)
#endif

#ifndef NETLIST_SET_FATHER_DECL
#define NETLIST_SET_FATHER_DECL           \
  virtual void set_father(Block* pf);    
#endif

#ifndef NETLIST_ELABORATE_DECL
#define NETLIST_ELABORATE_DECL                                \
  virtual bool elaborate(                                     \
    std::set<boost::shared_ptr<NetComp> >&,                  \
    std::map<boost::shared_ptr<NetComp>, std::list<boost::shared_ptr<NetComp> > >&); \
  using NetComp::elaborate;
#endif

#ifndef NETLIST_DB_DECL
#define NETLIST_DB_DECL                         \
  virtual void db_register(int);                \
  virtual void db_expunge();
#endif


#ifndef NETLIST_GEN_SDFG
#define NETLIST_GEN_SDFG                                   \
  virtual void gen_sdfg(boost::shared_ptr<SDFG::dfgGraph>); 
#endif

#ifndef NETLIST_REPLACE_VARIABLE
#define NETLIST_REPLACE_VARIABLE                   \
  virtual void replace_variable(const VIdentifier&, const Number&);  \
  virtual void replace_variable(const VIdentifier&, const VIdentifier&);
#endif

namespace netlist{
  
  // the base class of all netlist components
  class NetComp : public boost::enable_shared_from_this<NetComp> {
  public:
#include "comp_type.h"
    // no one should directly use this class
    NetComp();
    NetComp(ctype_t);
    NetComp(ctype_t, const shell::location&); 
    virtual ~NetComp() {}

    ctype_t get_type() const { return ctype; }
    ctype_t ctype;
    shell::location loc;
    
    virtual void reduce();	/* many netlist component need method to reduce itself */
    // the internal stream out method, to avoid friend declarations
    virtual std::ostream& streamout (std::ostream& os, unsigned int) const;    
    // deep copy when the content of a shared_ptr must be duplicated
    virtual NetComp* deep_copy( NetComp * rv) const;
    // set the father block pointer
    virtual void set_father(Block* pf);
    // get the hierarchy name
    virtual std::string get_hier_name();
    // replace a variable with a value in a block
    // use in unfolding for, while loops
    virtual void replace_variable(const VIdentifier&, const Number&);
    virtual void replace_variable(const VIdentifier&, const VIdentifier&);
    // register variable identifiers to the variable database
    virtual void db_register(int);
    // expunge the variable identifier in the variable database
    virtual void db_expunge();
    virtual bool elaborate(std::set<boost::shared_ptr<NetComp> >&,
                           std::map<boost::shared_ptr<NetComp>, std::list<boost::shared_ptr<NetComp> > >&);
    // unfold for before elaboration
    virtual boost::shared_ptr<Block> unfold();
    virtual unsigned int get_width() const;
    virtual void set_width(const unsigned int&);
    virtual void gen_sdfg(boost::shared_ptr<SDFG::dfgGraph>);
    virtual SDFG::RTree get_rtree() const;
    virtual boost::shared_ptr<NetComp> get_sp();
    virtual boost::shared_ptr<Expression> get_combined_expression(const VIdentifier&, std::set<std::string>);
    virtual Module* get_module(); // return the pointer of the father module

  protected:
    Block* father;              /* the pointer pointed to the father block */
      
  };
  NETLIST_STREAMOUT(NetComp);
  
}

#endif

// Local Variables:
// mode: c++
// End:
