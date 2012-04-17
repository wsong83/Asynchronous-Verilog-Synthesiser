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
 * Range declaration and definition
 * 08/02/2012   Wei Song
 *
 *
 */

#ifndef _H_RANGE_
#define _H_RANGE_

namespace netlist {

  typedef pair<shared_ptr<Expression>, shared_ptr<Expression> > Range_Exp;
  typedef pair<mpz_class, mpz_class> Range_Const;

  class Range : public NetComp {
  public:
    // constructors
    Range() : NetComp(tRange), dim(false), rtype(TR_Err) { }
    Range(const mpz_class&);	/* select by a fix number */
    Range(const location&, const mpz_class&);	/* select by a fix number */
    Range(const mpz_class&, const mpz_class&);	/* select by a fix number */
    Range(const location&, const mpz_class&, const mpz_class&);	/* select by a fix number */
    Range(const shared_ptr<Expression>&);	/* select by an expression  */
    Range(const location&, const shared_ptr<Expression>&);	/* select by an expression  */
    Range(const Range_Exp&, bool dim = false);    /* declare or select by a range expression */
    Range(const location&, const Range_Exp&, bool dim = false);    /* declare or select by a range expression */
    Range(const Range_Exp&, int); /* select by a range expression using positive or negtive colon */
    Range(const location&, const Range_Exp&, int); /* select by a range expression using positive or negtive colon */

    // helpers
    bool is_const() const { return rtype == TR_Const && rtype == TR_CRange; }
    bool is_single() const {return rtype != TR_Range && rtype != TR_CRange; }
    void db_register();
    void set_dim() { dim = true;}
    bool is_dim() const { return dim;}

    // inherit from NetComp
    NETLIST_STREAMOUT_DECL;
    virtual Range* deep_copy() const;
    
  private:
    mpz_class c;                /* constant */
    shared_ptr<Expression> v;   /* variable */
    Range_Exp r;                /* range expression */
    Range_Const cr;             /* const range */
    bool dim;                   /* true when it is a dimension rather than range */
    
    enum rtype_t {
      TR_Err,
      TR_Const, 
      TR_Var, 
      TR_Range,
      TR_CRange
    } rtype;
    

  };

  NETLIST_STREAMOUT(Range)

}

#endif
