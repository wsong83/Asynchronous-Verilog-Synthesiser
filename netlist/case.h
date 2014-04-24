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
    CaseItem(const boost::shared_ptr<Expression>&, const boost::shared_ptr<Block>&);
    CaseItem(const shell::location&, const boost::shared_ptr<Expression>&, const boost::shared_ptr<Block>&);
    // normal default case
    CaseItem(const boost::shared_ptr<Block>&);
    CaseItem(const shell::location&, const boost::shared_ptr<Block>&);
    // multiple expressions
    CaseItem(const std::list<boost::shared_ptr<Expression> >&, const boost::shared_ptr<Block>&);
    CaseItem(const shell::location&, const std::list<boost::shared_ptr<Expression> >&, const boost::shared_ptr<Block>&);

    // helpers
    NETLIST_STREAMOUT_DECL;
    NETLIST_SET_FATHER_DECL;
    virtual CaseItem* deep_copy(NetComp*) const;
    NETLIST_DB_DECL;
    NETLIST_ELABORATE_DECL;
    bool is_default() const {return exps.size() == 0; }
    bool is_match(const Number&) const; /* whether this case match with the number (arguement) */
    virtual boost::shared_ptr<SDFG::RTree> get_rtree() const;
    NETLIST_REPLACE_VARIABLE;

    // data
    std::list<boost::shared_ptr<Expression> > exps;
    boost::shared_ptr<Block> body;

  };
  NETLIST_STREAMOUT(CaseItem);

  class CaseState : public NetComp {
  public:
    enum case_type_t {CASE_DEFAULT, CASE_X, CASE_Z};
    // constructors
    CaseState();
    CaseState(const shell::location&);
    CaseState(const boost::shared_ptr<Expression>&, const std::list<boost::shared_ptr<CaseItem> >&, 
              const boost::shared_ptr<CaseItem>&, case_type_t mcase = CASE_DEFAULT);
    CaseState(const shell::location&, const boost::shared_ptr<Expression>&, 
              const std::list<boost::shared_ptr<CaseItem> >&, const boost::shared_ptr<CaseItem>&, 
              case_type_t mcase = CASE_DEFAULT);
    CaseState(const boost::shared_ptr<Expression>&, const std::list<boost::shared_ptr<CaseItem> >&, 
              case_type_t mcase = CASE_DEFAULT);
    CaseState(const shell::location&, const boost::shared_ptr<Expression>&, 
              const std::list<boost::shared_ptr<CaseItem> >&, case_type_t mcase = CASE_DEFAULT);
    CaseState(const boost::shared_ptr<Expression>&, const boost::shared_ptr<CaseItem>&, 
              case_type_t mcase = CASE_DEFAULT);
    CaseState(const shell::location&, const boost::shared_ptr<Expression>&, 
              const boost::shared_ptr<CaseItem>&, case_type_t mcase = CASE_DEFAULT);

    // helpers
    NETLIST_STREAMOUT_DECL;
    NETLIST_SET_FATHER_DECL;
    virtual CaseState* deep_copy(NetComp*) const;
    NETLIST_DB_DECL;
    NETLIST_ELABORATE_DECL;
    void set_name(const BIdentifier& nm) {name = nm; named=true;}
    bool is_named() const { return named; }
    virtual boost::shared_ptr<SDFG::RTree> get_rtree() const;
    NETLIST_REPLACE_VARIABLE;
    
    // data
    BIdentifier name;           /* dummy name for search index */
    boost::shared_ptr<Expression> exp;
    std::list<boost::shared_ptr<CaseItem> > cases;
    virtual boost::shared_ptr<Expression> get_combined_expression(const VIdentifier&, std::set<std::string>);
    
  private:
    bool named;
    case_type_t case_type;

  };
  NETLIST_STREAMOUT(CaseState);

}

#endif
// Local Variables:
// mode: c++
// End:
