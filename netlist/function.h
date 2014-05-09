/*
 * Copyright (c) 2013-2014 Wei Song <songw@cs.man.ac.uk> 
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
 * Combinational function
 * 22/02/2013   Wei Song
 *
 *
 */

#ifndef AV_H_AV_FUNCTION_
#define AV_H_AV_FUNCTION_

namespace netlist{

  class Function : public Block {
  public:
    Function();
    Function(const shell::location& lloc);
    Function(const shell::location& lloc, const FIdentifier& nm);

    // data
    FIdentifier fname;
    DataBase<VIdentifier, Port, true>      db_port;

    // inherited from NetComp
    NETLIST_STREAMOUT_DECL;
    virtual std::ostream& streamout(std::ostream&, unsigned int, bool) const;
    virtual void elab_inparse(); /* resolve the content in statements during parsing */
    NETLIST_SET_FATHER_DECL;
    virtual void set_father();   /* set the father pointer to all sub-elements */
    virtual Function* deep_copy(NetComp*) const;
    NETLIST_DB_DECL;
    NETLIST_ELABORATE_DECL;

    // helpers
    void set_return(boost::shared_ptr<Expression>, boost::shared_ptr<Expression>); // return a value with a range
    void set_automatic() {automatic = true; }   // automatic function
    void set_inputs(const std::list<boost::shared_ptr<netlist::Port> >&); // set the input ports
    void add_input(boost::shared_ptr<Port>&); // add a single input port

  protected:
    bool automatic;             // default 0, (automatic function?)
    RangeArray rtype;           // type of the return value


  };

  NETLIST_STREAMOUT(Function);

}

#endif

// Local Variables:
// mode: c++
// End:
