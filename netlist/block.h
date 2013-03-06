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
    Block();
    Block(const shell::location&);
    Block(ctype_t, const BIdentifier&);
    Block(ctype_t, const shell::location&, const BIdentifier&);
    Block(const BIdentifier&);
    Block(const shell::location&, const BIdentifier&);
    Block(NetComp::ctype_t);
    Block(NetComp::ctype_t, const shell::location&);

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
                  CaseState::case_type_t m_casex = CaseState::CASE_DEFAULT); 
    
    /* add a general case statement */
    bool add_case(const shell::location& lloc, const boost::shared_ptr<Expression>&, 
                  const std::list<boost::shared_ptr<CaseItem> >&, const boost::shared_ptr<CaseItem>&,
                  CaseState::case_type_t m_casex = CaseState::CASE_DEFAULT);
 
    /* add a case statement without default */
    bool add_case(const boost::shared_ptr<Expression>&, 
                  const std::list<boost::shared_ptr<CaseItem> >&,
                  CaseState::case_type_t m_casex = CaseState::CASE_DEFAULT);

    /* add a case statement without default */
    bool add_case(const shell::location& lloc, const boost::shared_ptr<Expression>&, 
                  const std::list<boost::shared_ptr<CaseItem> >&,
                  CaseState::case_type_t m_casex = CaseState::CASE_DEFAULT); 

    /* add a case statement only with a default case, odd! */
    bool add_case(const boost::shared_ptr<Expression>&, 
                  const boost::shared_ptr<CaseItem>&,
                  CaseState::case_type_t m_casex = CaseState::CASE_DEFAULT);

    /* add a case statement only with a default case, odd! */
    bool add_case(const shell::location& lloc, const boost::shared_ptr<Expression>&, 
                  const boost::shared_ptr<CaseItem>&,
                  CaseState::case_type_t m_casex = CaseState::CASE_DEFAULT);

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

    bool add_statements(const boost::shared_ptr<Block>&);    /* add several statements */
    
    virtual void elab_inparse(); /* resolve the content in statements during parsing */

    // helpers
    virtual void set_name(const BIdentifier& nm) { name = nm; named=true;}
    void set_default_name(const BIdentifier& nm) { name = nm; }
    bool is_named() const { return named; }
    /* return a pointer of the top-level module */
    virtual Block* get_module() { 
      if(father != NULL) return father->get_module();
      else return NULL;
    }
    
    BIdentifier& new_BId();     /* generate an unused block id */
    IIdentifier& new_IId();     /* generate an unused instance id*/
    VIdentifier& new_VId();     /* generate an unused variable id */
    virtual boost::shared_ptr<Instance>  find_instance  (const IIdentifier&) const; /* find an instance */
    virtual boost::shared_ptr<Variable>  find_var       (const VIdentifier&) const; /* find a variable */
    /* find a variable in the global environment, up to the module level */
    virtual boost::shared_ptr<Variable>  gfind_var      (const VIdentifier&) const; 
    virtual std::ostream& streamout(std::ostream& os, unsigned int indent, bool fl_prefix, bool is_else = false) const;
    const boost::shared_ptr<NetComp>& front() const { return statements.front(); }
    boost::shared_ptr<NetComp>& front() { return statements.front(); }
    virtual void set_father();  /* set the father pointer to all sub-elements */
    
    // inherit from NetComp
    NETLIST_STREAMOUT_DECL;
    using NetComp::set_father;
    virtual Block* deep_copy() const;
    NETLIST_DB_DECL;
    NETLIST_ELABORATE_DECL;
    using NetComp::gen_sdfg;
    NETLIST_SCAN_VARS;
    NETLIST_REPLACE_VARIABLE;

    // data
    BIdentifier name;
    std::list<boost::shared_ptr<NetComp> > statements;   /* a general list to stor the statements */
    DataBase<VIdentifier, Variable, true>  db_var;       /* variables */
    DataBase<IIdentifier, Instance>        db_instance;  /* module instances */
    DataBase<FIdentifier, Function>        db_func;      /* functions */
    
  protected:

    //name for unnamed items
    BIdentifier unnamed_block;
    IIdentifier unnamed_instance;
    VIdentifier unnamed_var;

    bool named;                 /* true when named */
    
  };

  NETLIST_STREAMOUT(Block);
 
}

#endif
// Local Variables:
// mode: c++
// End:
