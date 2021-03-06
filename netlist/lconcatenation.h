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
 * left-side concatenation, only to be the target of an assignment
 * 23/02/2012   Wei Song
 *
 *
 */

#ifndef AV_H_LCONCATENATION_
#define AV_H_LCONCATENATION_

namespace netlist {

  class LConcatenation : public NetComp {
  public:
    // constructors
    NETLIST_DEFAULT_CON(LConcatenation, tLConcatenation);
    LConcatenation(boost::shared_ptr<Concatenation>&);
    LConcatenation(const shell::location&, boost::shared_ptr<Concatenation>&);
    LConcatenation(const VIdentifier&);
    LConcatenation(const shell::location&, const VIdentifier&);

    // inherit from NetComp
    NETLIST_SET_FATHER_DECL;
    NETLIST_STREAMOUT_DECL;
    virtual LConcatenation* deep_copy(NetComp*) const;
    NETLIST_DB_DECL;
    NETLIST_REPLACE_VARIABLE;
    SDFG::RForest get_rforest() const;

    // helpers
    bool is_valid() const { return valid; }
    unsigned int size() const { return data.size(); }
    VIdentifier& front() { return data.front(); }
    const VIdentifier& front() const { return data.front(); }
    void reduce();

    // data
    std::list<VIdentifier> data; /* store the list of variable identifiers, wires or registers */
    
  private:
    bool valid;

  };
  NETLIST_STREAMOUT(LConcatenation);

}

#endif
// Local Variables:
// mode: c++
// End:
