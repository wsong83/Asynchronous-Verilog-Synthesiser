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
 * General block definition
 * A block may be embedded in another one.
 * 22/03/2012   Wei Song
 *
 *
 */

#ifndef AV_H_AV_BLOCK_
#define AV_H_AV_BLOCK_

namespace netlist {

  class Block : public NetComp {
  public:
    // constructors
    Block() : NetComp(tBlock), named(false), blocked(false) {}
    Block(const shell::location& lloc) : NetComp(tBlock, lloc), named(false), blocked(false) {}
    Block(ctype_t t, const BIdentifier& nm) 
      : NetComp(t), name(nm), named(true), blocked(true) {}
    Block(ctype_t t, const shell::location& lloc, const BIdentifier& nm) 
      : NetComp(t, lloc), name(nm), named(true), blocked(true) {}
    Block(const BIdentifier& nm) 
      : NetComp(tBlock), name(nm), named(true), blocked(true) {}
    Block(const shell::location& lloc, const BIdentifier& nm) 
      : NetComp(tBlock, lloc), name(nm), named(true), blocked(true) {}
    Block(NetComp::ctype_t t) 
      : NetComp(t), named(false), blocked(false) {}
    Block(NetComp::ctype_t t, const shell::location& lloc) 
      : NetComp(t, lloc), named(false), blocked(false) {}

    /* add a general statement to this block */
    virtual bool add(const boost::shared_ptr<NetComp>&); 

    /* a template funtion to add all sorts of netlist components */
    template<typename T>
      bool add_list(const std::list<boost::shared_ptr<T> >& ll) {
      typename std::list<boost::shared_ptr<T> >::const_iterator it, end;
      for(it=ll.begin(), end=ll.end(); it!=end; it++)
        add(*it);
      return true;
    }

    /* add a general case statement */
    bool add_case(const boost::shared_ptr<Expression>&, 
                  const std::list<boost::shared_ptr<CaseItem> >&, const boost::shared_ptr<CaseItem>&,
                  bool m_casex = false); 
    
    /* add a general case statement */
    bool add_case(const shell::location& lloc, const boost::shared_ptr<Expression>&, 
                  const std::list<boost::shared_ptr<CaseItem> >&, const boost::shared_ptr<CaseItem>&,
                  bool m_casex = false);
 
    /* add a case statement without default */
    bool add_case(const boost::shared_ptr<Expression>&, 
                  const std::list<boost::shared_ptr<CaseItem> >&,
                  bool m_casex = false);

    /* add a case statement without default */
    bool add_case(const shell::location& lloc, const boost::shared_ptr<Expression>&, 
                  const std::list<boost::shared_ptr<CaseItem> >&,
                  bool m_casex = false); 

    /* add a case statement only with a default case, odd! */
    bool add_case(const boost::shared_ptr<Expression>&, 
                  const boost::shared_ptr<CaseItem>&,
                  bool m_casex = false);

    /* add a case statement only with a default case, odd! */
    bool add_case(const shell::location& lloc, const boost::shared_ptr<Expression>&, 
                  const boost::shared_ptr<CaseItem>&,
                  bool m_casex = false);

    /* add an if statement with else case */
    bool add_if(const boost::shared_ptr<Expression>&, 
                const boost::shared_ptr<Block>&, const boost::shared_ptr<Block>&); 
    
    /* add an if statement with else case */
    bool add_if(const shell::location& lloc, const boost::shared_ptr<Expression>&, 
                const boost::shared_ptr<Block>&, const boost::shared_ptr<Block>&);

    /* add an if statement without else case */
    bool add_if(const boost::shared_ptr<Expression>&, const boost::shared_ptr<Block>&);

    /* add an if statement without else case */
    bool add_if(const shell::location& lloc, const boost::shared_ptr<Expression>&, const boost::shared_ptr<Block>&);

    /* add a while statement */
    bool add_while(const boost::shared_ptr<Expression>&, const boost::shared_ptr<Block>&);

    /* add a while statement */
    bool add_while(const shell::location& lloc, const boost::shared_ptr<Expression>&, const boost::shared_ptr<Block>&);

    /* add a for statement */
    bool add_for(const boost::shared_ptr<Assign>&, 
                 const boost::shared_ptr<Expression>&, const boost::shared_ptr<Assign>&, const boost::shared_ptr<Block>&); 

    /* add a for statement */
    bool add_for(const shell::location& lloc, const boost::shared_ptr<Assign>&, 
                 const boost::shared_ptr<Expression>&, const boost::shared_ptr<Assign>&, const boost::shared_ptr<Block>&);

    bool add_statements(const boost::shared_ptr<Block>&); /* add several statements */
    virtual void elab_inparse(); /* resolve the content in statements during parsing */

    // helpers
    virtual void set_name(const BIdentifier& nm) { name = nm; named=true; blocked = true; }
    void set_default_name(const BIdentifier& nm) { name = nm; }
    virtual void set_blocked() { blocked = true; }
    bool is_named() const { return named; }
    bool is_blocked() const { return blocked; }
    /* return a pointer of the top-level module */
    virtual Block* get_module() { 
      if(father != NULL) return father->get_module();
      else return NULL;
    }
    
    BIdentifier& new_BId();     /* generate an unused block id */
    IIdentifier& new_IId();     /* generate an unused instance id*/
    VIdentifier& new_VId();     /* generate an unused variable id */
    virtual boost::shared_ptr<NetComp>   find_item      (const BIdentifier&) const; /* find an item in db_other */
    virtual boost::shared_ptr<Instance>  find_instance  (const IIdentifier&) const; /* find an instance */
    virtual boost::shared_ptr<Variable>  find_var       (const VIdentifier&) const; /* find a variable */
    /* find a variable in the global environment, up to the module level */
    virtual boost::shared_ptr<Variable>  gfind_var      (const VIdentifier&) const; 
    virtual std::ostream& streamout(std::ostream& os, unsigned int indent, bool fl_prefix) const;
    const boost::shared_ptr<NetComp>& front() const { return statements.front(); }
    boost::shared_ptr<NetComp>& front() { return statements.front(); }
    virtual void set_father();  /* set the father pointer to all sub-elements */
    
    // inherit from NetComp
    NETLIST_STREAMOUT_DECL;
    NETLIST_CHECK_INPARSE_DECL;
    using NetComp::set_father;
    virtual Block* deep_copy() const;
    virtual void db_register(int iod = 1);
    virtual void db_expunge();
    NETLIST_ELABORATE_DECL;
    NETLIST_SET_ALWAYS_POINTER_DECL;
    using NetComp::gen_sdfg;
    NETLIST_SCAN_VARS;
    NETLIST_GEN_SDFG_BLK;

    // data
    BIdentifier name;
    std::list<boost::shared_ptr<NetComp> > statements;   /* a general list to stor the statements */
    DataBase<VIdentifier, Variable, true>  db_var;       /* variables */
    DataBase<IIdentifier, Instance>        db_instance;  /* module instances */
    DataBase<BIdentifier, NetComp>         db_other;     /* non-block statements, including assignements, if, etc. */
    
  protected:

    //name for unnamed items
    BIdentifier unnamed_block;
    IIdentifier unnamed_instance;
    VIdentifier unnamed_var;

    bool named;                 /* true when named */
    bool blocked;               /* user used begin and end for this block */
    
    // helper in elab_inparse
    bool elab_inparse_item (const boost::shared_ptr<NetComp>&, std::list<boost::shared_ptr<NetComp> >::iterator&);

  };

  NETLIST_STREAMOUT(Block);
 
}

#endif
// Local Variables:
// mode: c++
// End:
