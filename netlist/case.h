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
 * Case and casex statements
 * 26/03/2012   Wei Song
 *
 *
 */


#ifndef AV_H_AV_CASE_
#define AV_H_AV_CASE_

namespace netlist{

  // one case in the whole case statement
  class CaseItem : public NetComp {
  public:
    NETLIST_DEFAULT_CON(CaseItem, tCaseItem);
    NETLIST_DEFAULT_CON_WL(CaseItem, tCaseItem);
    // constructor with only one expression, the normal case
    CaseItem(const boost::shared_ptr<Expression>& exp, const boost::shared_ptr<Block>& body)
      : NetComp(tCaseItem), body(body)
    {
      exps.push_back(exp);
      body->elab_inparse();
    }
    CaseItem(const shell::location& lloc, const boost::shared_ptr<Expression>& exp, const boost::shared_ptr<Block>& body)
      : NetComp(tCaseItem, lloc), body(body)
    {
      exps.push_back(exp);
      body->elab_inparse();
    }
    // normal default case
    CaseItem(const boost::shared_ptr<Block>& body)
      : NetComp(tCaseItem), body(body)
    {
      body->elab_inparse();
    }
    CaseItem(const shell::location& lloc, const boost::shared_ptr<Block>& body)
      : NetComp(tCaseItem, lloc), body(body)
    {
      body->elab_inparse();
    }
    // multiple expressions
    CaseItem(const std::list<boost::shared_ptr<Expression> >& expm, const boost::shared_ptr<Block>& body)
      : NetComp(tCaseItem), exps(expm), body(body)
    {
      body->elab_inparse();
    }
    CaseItem(const shell::location& lloc, const std::list<boost::shared_ptr<Expression> >& expm, const boost::shared_ptr<Block>& body)
      : NetComp(tCaseItem, lloc), exps(expm), body(body)
    {
      body->elab_inparse();
    }

    // helpers
    NETLIST_STREAMOUT_DECL;
    NETLIST_SET_FATHER_DECL;
    NETLIST_CHECK_INPARSE_DECL;
    virtual CaseItem* deep_copy() const;
    virtual void db_register(int iod = 1);
    virtual void db_expunge();
    NETLIST_ELABORATE_DECL;
    NETLIST_SET_ALWAYS_POINTER_DECL;
    bool is_default() const {return exps.size() == 0; }
    bool is_match(const Number&) const; /* whether this case match with the number (arguement) */
    NETLIST_GEN_SDFG_BLK;

    // data
    std::list<boost::shared_ptr<Expression> > exps;
    boost::shared_ptr<Block> body;

  };
  NETLIST_STREAMOUT(CaseItem);

  class CaseState : public NetComp {
  public:
    // constructors
    NETLIST_DEFAULT_CON(CaseState, tCase);
    NETLIST_DEFAULT_CON_WL(CaseState, tCase);
    CaseState(const boost::shared_ptr<Expression>& exp, const std::list<boost::shared_ptr<CaseItem> >& citems, 
              const boost::shared_ptr<CaseItem>& ditem, bool mcasex = false)
      : NetComp(tCase), exp(exp), cases(citems), named(false), casex(mcasex) 
    {
      cases.push_back(ditem);
    }
    CaseState(const shell::location& lloc, const boost::shared_ptr<Expression>& exp, 
              const std::list<boost::shared_ptr<CaseItem> >& citems, const boost::shared_ptr<CaseItem>& ditem, 
              bool mcasex = false)
      : NetComp(tCase, lloc), exp(exp), cases(citems), named(false), casex(mcasex) 
    {
      cases.push_back(ditem);
    }
    CaseState(const boost::shared_ptr<Expression>& exp, const std::list<boost::shared_ptr<CaseItem> >& citems, 
              bool mcasex = false)
      : NetComp(tCase), exp(exp), cases(citems), named(false), casex(mcasex) 
    { 
    }
    CaseState(const shell::location& lloc, const boost::shared_ptr<Expression>& exp, 
              const std::list<boost::shared_ptr<CaseItem> >& citems, bool mcasex = false)
      : NetComp(tCase, lloc), exp(exp), cases(citems), named(false), casex(mcasex) 
    { 
    }
    CaseState(const boost::shared_ptr<Expression>& exp, const boost::shared_ptr<CaseItem>& ditem, 
              bool mcasex = false)
      : NetComp(tCase), exp(exp), named(false), casex(mcasex) 
    {
      cases.push_back(ditem);
    }
    CaseState(const shell::location& lloc, const boost::shared_ptr<Expression>& exp, 
              const boost::shared_ptr<CaseItem>& ditem, bool mcasex = false)
      : NetComp(tCase, lloc), exp(exp), named(false), casex(mcasex) 
    {
      cases.push_back(ditem);
    }

    // helpers
    NETLIST_STREAMOUT_DECL;
    NETLIST_SET_FATHER_DECL;
    NETLIST_CHECK_INPARSE_DECL;
    virtual CaseState* deep_copy() const;
    virtual void db_register(int iod = 1);
    virtual void db_expunge();
    NETLIST_ELABORATE_DECL;
    NETLIST_SET_ALWAYS_POINTER_DECL;
    void set_name(const BIdentifier& nm) {name = nm; named=true;}
    bool is_named() const { return named; }
    bool is_casex() const { return casex; }
    NETLIST_GEN_SDFG_BLK;

    // data
    BIdentifier name;           /* dummy name for search index */
    boost::shared_ptr<Expression> exp;
    std::list<boost::shared_ptr<CaseItem> > cases;
    
  private:
    bool named;
    bool casex;

  };
  NETLIST_STREAMOUT(CaseState);

}

#endif
// Local Variables:
// mode: c++
// End:
