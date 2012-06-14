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

#ifndef AV_H_RANGE_
#define AV_H_RANGE_

namespace netlist {

  typedef std::pair<boost::shared_ptr<Expression>, boost::shared_ptr<Expression> > Range_Exp;
  typedef std::pair<Number, Number> Range_Const;

  class Range : public NetComp {
  public:
    // constructors
    Range() : NetComp(tRange), dim(false), rtype(TR_Err) { }
    Range(const mpz_class&);	/* select by a fix number */
    Range(const shell::location&, const mpz_class&);	/* select by a fix number */
    Range(const mpz_class&, const mpz_class&);	/* select by a fix number */
    Range(const shell::location&, const mpz_class&, const mpz_class&);	/* select by a fix number */
    Range(const boost::shared_ptr<Expression>&);	/* select by an expression  */
    Range(const shell::location&, const boost::shared_ptr<Expression>&);	/* select by an expression  */
    Range(const Range_Exp&, bool dim = false);    /* declare or select by a range expression */
    Range(const shell::location&, const Range_Exp&, bool dim = false);    /* declare or select by a range expression */
    Range(const Range_Exp&, int); /* select by a range expression using positive or negtive colon */
    Range(const shell::location&, const Range_Exp&, int); /* select by a range expression using positive or negtive colon */

    // helpers
    bool is_single() const {return rtype != TR_Range && rtype != TR_CRange; }
    bool is_empty() const { return rtype == TR_Empty; }
    void set_dim() { dim = true;}
    bool is_dim() const { return dim;}
    bool is_valuable() const { return (rtype == TR_Const || rtype == TR_CRange|| rtype == TR_Empty); }
    bool is_valid() const { return rtype != TR_Err; }
    Range& op_and(const Range&);           /* helper for operator & */
    Range& op_or(const Range&);            /* helper for operator | */
    bool op_equ(const Range&) const;       /* helper for operator == */
    bool op_belong_to(const Range&) const; /* helper for >= */
    bool op_adjacent_to(const Range&) const;

    // inherit from NetComp
    NETLIST_SET_FATHER_DECL;
    NETLIST_STREAMOUT_DECL;
    NETLIST_CHECK_INPARSE_DECL;
    virtual Range* deep_copy() const;
    virtual void db_register(int iod = 1);
    virtual void db_expunge();
    NETLIST_ELABORATE_DECL;

    std::list<boost::shared_ptr<Range> > child; /* a multi-tree structure to support range array*/

  private:
    Number c;                          /* constant */
    boost::shared_ptr<Expression> v;   /* variable */
    Range_Exp r;                       /* range expression */
    Range_Const cr;                    /* const range */
    bool dim;                          /* true when it is a dimension rather than a range */
    
    enum rtype_t {
      TR_Err,
      TR_Empty,
      TR_Const, 
      TR_Var,
      TR_Range,
      TR_CRange
    } rtype;

  };

  Range operator& ( const Range&, const Range&);
  Range operator| ( const Range&, const Range&);
  bool operator>= ( const Range& lhs, const Range& rhs); /* whether rhs belongs to lhs */
  bool operator== ( const Range& lhs, const Range& rhs);

  NETLIST_STREAMOUT(Range)

}

#endif
