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
 * Identifiers
 * 07/02/2012   Wei Song
 *
 *
 */

#ifndef AV_H_IDENTIFIER_
#define AV_H_IDENTIFIER_

#include "averilog/av_comp.h"

namespace netlist {

  /////////////// base class
  class Identifier : public NetComp{
  public:
    // constructors
    Identifier();
    Identifier(NetComp::ctype_t);
    Identifier(NetComp::ctype_t, const shell::location&);
    Identifier(NetComp::ctype_t, const std::string&);
    Identifier(NetComp::ctype_t, const shell::location&, const std::string&);

    // helpers
    int compare(const Identifier& rhs) const; /* compare two identifiers */
    NETLIST_STREAMOUT_DECL;
    NETLIST_SET_FATHER_DECL;
    void hash_update();			   /* update the nearly unique hash id */
    virtual std::string get_name() const { return name; }
    virtual void set_name(const std::string& newName) { name = newName; hash_update(); }
    virtual std::string get_suffix() const;
    virtual bool replace_suffix(const std::string& newSuffix);
    virtual void add_suffix(const std::string& newSuffix);
    virtual void suffix_increase();
    virtual void add_prefix(const std::string& prefix);

  protected:
    // data
    std::string name;		/* the name of the identifier */

  private:
    unsigned int hashid;	/* the nearly unique heash id */
    bool numbered;              // whether it is numbered (suffix)

  };
  
  bool operator< (const Identifier&, const Identifier&);
  bool operator> (const Identifier&, const Identifier&);
  bool operator== (const Identifier&, const Identifier&);
  bool operator!= (const Identifier&, const Identifier&);
  NETLIST_STREAMOUT(Identifier);
  
  ////////////// block identifier
  class BIdentifier : public Identifier{
  public:
    // constructors
    BIdentifier(const std::string&);
    BIdentifier(const shell::location&, const std::string&);
    BIdentifier();
    BIdentifier(const shell::location&);
    BIdentifier(const averilog::avID& );
    BIdentifier(const shell::location&, const averilog::avID& );

  private:
    bool anonymous;
  };
  NETLIST_STREAMOUT(BIdentifier);

  
  ///////////// function identifier
  class FIdentifier : public Identifier {
  public:
    // constructors
    FIdentifier();
    FIdentifier(const std::string&);
    FIdentifier(const shell::location&, const std::string&);
    FIdentifier(const shell::location&, const averilog::avID&);
    
  };
  NETLIST_STREAMOUT(FIdentifier);

  ///////////// module identifier
  class MIdentifier : public Identifier {
  public:
    // constructors
    MIdentifier();
    MIdentifier(const shell::location&);
    MIdentifier(const std::string&);
    MIdentifier(const shell::location&, const std::string&);
    MIdentifier(const averilog::avID& );
    MIdentifier(const shell::location&, const averilog::avID& );
    
    // helpers
    NETLIST_STREAMOUT_DECL;

  };
  NETLIST_STREAMOUT(MIdentifier);

  //////////// instance identifier
  class IIdentifier : public Identifier {
  public:
    // constructors
    IIdentifier();
    IIdentifier(const shell::location&);
    IIdentifier(const std::string&);
    IIdentifier(const shell::location&, const std::string&);
    IIdentifier(const averilog::avID&);
    IIdentifier(const shell::location&, const averilog::avID&);

  };
  NETLIST_STREAMOUT(IIdentifier);

  /////////// port identifier
  class PoIdentifier : public Identifier {
  public:
    // constructors
    PoIdentifier();
    PoIdentifier(const shell::location&);
    PoIdentifier(const std::string&);
    PoIdentifier(const shell::location&, const std::string&);
    PoIdentifier(const averilog::avID&);
    PoIdentifier(const shell::location&, const averilog::avID&);

    // helpers
    void set_range(const RangeArray& nr) { m_range = nr; }
    const RangeArray get_range() const {return m_range;}
    RangeArray& get_range() {return m_range;}
    NETLIST_SET_FATHER_DECL;
    NETLIST_STREAMOUT_DECL;

  private:
    RangeArray m_range;

  };
  NETLIST_STREAMOUT(PoIdentifier);

  /////////// Variable identifier
  class VIdentifier : public Identifier {
  public:
    // constructors
    VIdentifier();
    VIdentifier(const shell::location&);
    VIdentifier(const std::string&);
    VIdentifier(const shell::location&, const std::string&);
    VIdentifier(const averilog::avID&);
    VIdentifier(const shell::location&, const averilog::avID&);
    VIdentifier(const std::string&, const RangeArray&);
    VIdentifier(const shell::location&, const std::string&, const RangeArray&);
    virtual ~VIdentifier();

    //helpers
    const RangeArray& get_range() const {return m_range;}
    const RangeArray& get_select() const {return m_select;}
    RangeArray& get_range() {return m_range;}
    RangeArray& get_select() {return m_select;}
    bool is_valuable() const;
    Number get_value() const;
    void set_value(const Number& p) { value = p; }
    //const std::string& get_txt_value() const { return value.get_txt_value(); }
    void db_register(const boost::shared_ptr<Variable>&, int);
    bool db_registered() const { return uid != 0; }
    void reset_uid(unsigned int id) { uid = id; } /* only used by Variable::get_id() */
    void reduce();
    std::set<boost::shared_ptr<NetComp> > get_drivers() const; // get the driving sequential block or continuous assignment of this variable

    // inherit from NetComp
    NETLIST_SET_FATHER_DECL;
    NETLIST_STREAMOUT_DECL;
    virtual VIdentifier* deep_copy() const;
    NETLIST_DB_DECL;
    NETLIST_REPLACE_VARIABLE;
    virtual boost::shared_ptr<SDFG::RTree> get_rtree() const;
    virtual unsigned int get_width() const;

  private:
    Number value;
    RangeArray m_range;
    RangeArray m_select;
    boost::shared_ptr<Variable> pvar;     /* the wire/reg/var in the database */
    unsigned int uid;              /* used as the key to search this variable as fanin or fanout */
  };
  NETLIST_STREAMOUT(VIdentifier);


}


#endif

// Local Variables:
// mode: c++
// End:
