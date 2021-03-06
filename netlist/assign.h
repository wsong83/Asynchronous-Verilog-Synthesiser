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
 * Block/non-block assignments
 * 23/02/2012   Wei Song
 *
 *
 */

#ifndef AV_H_ASSIGN_
#define AV_H_ASSIGN_

namespace netlist {

  class Assign : public NetComp {
  public:
    // constructors
    Assign(const boost::shared_ptr<LConcatenation>&, const boost::shared_ptr<Expression>&, bool);
    Assign(const shell::location& lloc, const boost::shared_ptr<LConcatenation>&, const boost::shared_ptr<Expression>&, bool);

    // inherit from NetComp
    NETLIST_STREAMOUT_DECL;
    NETLIST_SET_FATHER_DECL;
    virtual Assign* deep_copy(NetComp*) const;
    NETLIST_DB_DECL;
    NETLIST_ELABORATE_DECL;
    NETLIST_GEN_SDFG;
    NETLIST_REPLACE_VARIABLE;
    virtual SDFG::RForest get_rforest() const;
    virtual boost::shared_ptr<Block> unfold();

    // helpers
    void set_name(const BIdentifier& nm) {name = nm; named=true;}
    void set_continuous() { continuous = true; }
    bool is_named() const { return named; }
    bool is_continuous() const { return continuous; }

    // data
    BIdentifier name;                /* as a key in the database, it has no practical meaning */
    boost::shared_ptr<LConcatenation> lval; /* the left-value is a left-concatenation with one or multiple identifiers */
    boost::shared_ptr<Expression> rexp;     /* the right-side expression */
    virtual boost::shared_ptr<Expression> get_combined_expression(const VIdentifier&, std::set<std::string>);

  private:
    bool blocking;                   /* true when it is a blocking assignment */
    bool continuous;                 /* true when it is a continuous assignment */
    bool named;                      /* true when a unique name is set */
  };

  NETLIST_STREAMOUT(Assign);

}

#endif
// Local Variables:
// mode: c++
// End:
