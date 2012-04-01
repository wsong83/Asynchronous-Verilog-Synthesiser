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
 * Case statements
 * 26/03/2012   Wei Song
 *
 *
 */


#ifndef _H_AV_CASE_
#define _H_AV_CASE_

namespace netlist{

  // one case in the whole case statement
  class CaseItem : public NetComp {
  public:
    NETLIST_DEFAULT_CON(CaseItem, tCaseItem);
    // constructor with only one expression, the normal case
    CaseItem(const shared_ptr<Expression>& exp, const shared_ptr<SeqBlock>& body) {
      exps.push_back(exp);
      add_statements(body);
    }
    // normal default case
    CaseItem(const shared_ptr<SeqBlock>& body) {
      add_statements(body);
    }
    // multiple expressions
    CaseItem(list<shared_ptr<Expression> >& expm, const shared_ptr<SeqBlock>& body) {
      exps.splice(exps.end(), expm);
      add_statements(body);
    }

    // helpers
    NETLIST_STREAMOUT_FUN_DECL;
    bool is_default() const {return exps.size() == 0; }
    void add_statements (const shared_ptr<SeqBlock>&); /* add sattements to this case item */

    // data
    list<shared_ptr<Expression> > exps;
    list<shared_ptr<NetComp> > statements;

  };
  NETLIST_STREAMOUT(CaseItem);

  class CaseState : public NetComp {
  public:
    // constructors
    NETLIST_DEFAULT_CON(CaseState, tCase);
    CaseState(const shared_ptr<Expression>& exp, list<shared_ptr<CaseItem> >& citems, const shared_ptr<CaseItem>& ditem)
      : NetComp(tCase), exp(exp), cases(citems) {
      cases.push_back(ditem);
    }
    CaseState(const shared_ptr<Expression>& exp, list<shared_ptr<CaseItem> >& citems)
      : NetComp(tCase), exp(exp), cases(citems) { }
    CaseState(const shared_ptr<Expression>& exp, const shared_ptr<CaseItem>& ditem)
      : NetComp(tCase), exp(exp) {
      cases.push_back(ditem);
    }

    // helpers
    NETLIST_STREAMOUT_FUN_DECL;

    // data
    shared_ptr<Expression> exp;
    list<shared_ptr<CaseItem> > cases;

  };
  NETLIST_STREAMOUT(CaseState);

}

#endif
