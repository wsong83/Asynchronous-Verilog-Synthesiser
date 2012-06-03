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
    Identifier() {}
    Identifier(NetComp::ctype_t ctype) : NetComp(ctype) {}
    Identifier(NetComp::ctype_t ctype, const shell::location& lloc) : NetComp(ctype, lloc) {}
    Identifier(NetComp::ctype_t, const std::string&);
    Identifier(NetComp::ctype_t, const shell::location& lloc, const std::string&);

    // helpers
    int compare(const Identifier& rhs) const; /* compare two identifiers */
    NETLIST_STREAMOUT_DECL;
    NETLIST_SET_FATHER_DECL;
    void hash_update();			   /* update the nearly unique hash id */

    // data
    std::string name;		/* the name of the identifier */
    unsigned int hashid;	/* the nearly unique heash id */

  };
  
  bool operator< (const Identifier& lhs, const Identifier& rhs);
  bool operator> (const Identifier& lhs, const Identifier& rhs);
  bool operator== (const Identifier& lhs, const Identifier& rhs);
  NETLIST_STREAMOUT(Identifier);
  
  ////////////// block identifier
  class BIdentifier : public Identifier{
  public:
    // constructors
    BIdentifier(const std::string&);
    BIdentifier(const shell::location& lloc, const std::string&);
    BIdentifier();
    BIdentifier(const shell::location& lloc);
    BIdentifier(const averilog::avID& );
    BIdentifier(const shell::location& lloc, const averilog::avID& );

    // helpers
    BIdentifier& operator++ ();

  private:
    bool anonymous;
  };
  NETLIST_STREAMOUT(BIdentifier);

  
  ///////////// function identifier
  class FIdentifier : public Identifier {
  public:
    // constructors
    FIdentifier() : Identifier(NetComp::tFuncName) { }
    FIdentifier(const std::string&);
    FIdentifier(const shell::location& lloc, const std::string&);
    
    // helpers

  };
  NETLIST_STREAMOUT(FIdentifier);

  ///////////// module identifier
  class MIdentifier : public Identifier {
  public:
    // constructors
    MIdentifier() : Identifier(tModuleName) {}
    MIdentifier(const shell::location& lloc) : Identifier(tModuleName, lloc) {}
    MIdentifier(const std::string&);
    MIdentifier(const shell::location& lloc, const std::string&);
    MIdentifier(const averilog::avID& );
    MIdentifier(const shell::location& lloc, const averilog::avID& );
    
    // helpers
    MIdentifier& operator++ ();
    NETLIST_STREAMOUT_DECL;

  private:
    bool numbered;

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

    // helpers
    IIdentifier& operator++ ();
    IIdentifier& add_prefix (const Identifier&);

  private:
    bool numbered;
  };
  NETLIST_STREAMOUT(IIdentifier);

  /////////// port identifier
  class PoIdentifier : public Identifier {
  public:
    // constructors
    PoIdentifier() : Identifier(NetComp::tPortName) {}
    PoIdentifier(const shell::location& lloc) : Identifier(NetComp::tPortName, lloc) {}
    PoIdentifier(const std::string&);
    PoIdentifier(const shell::location&, const std::string&);
    PoIdentifier(const averilog::avID&);
    PoIdentifier(const shell::location&, const averilog::avID&);

    // helpers
    void set_range(const std::vector<boost::shared_ptr<Range> >& nr) { m_range = nr; }
    const std::vector<boost::shared_ptr<Range> >& get_range() const {return m_range;}
    std::vector<boost::shared_ptr<Range> >& get_range() {return m_range;}
    NETLIST_SET_FATHER_DECL;
    NETLIST_STREAMOUT_DECL;

  private:
    std::vector<boost::shared_ptr<Range> > m_range;

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
    VIdentifier(const std::string&, const std::vector<boost::shared_ptr<Range> >&);
    VIdentifier(const shell::location&, const std::string&, const std::vector<boost::shared_ptr<Range> >&);

    //helpers
    VIdentifier& operator++ ();
    VIdentifier& add_prefix (const Identifier&);
    const std::vector<boost::shared_ptr<Range> >& get_range() const {return m_range;}
    const std::vector<boost::shared_ptr<Range> >& get_select() const {return m_select;}
    std::vector<boost::shared_ptr<Range> >& get_range() {return m_range;}
    std::vector<boost::shared_ptr<Range> >& get_select() {return m_select;}
    bool is_valuable() const { return value.is_valuable(); }
    mpz_class get_value() const { return value.get_value(); }
    void set_value(const Number& p) { value = p; }
    const std::string& get_txt_value() const { return value.get_txt_value(); }
    void db_register(const boost::shared_ptr<Variable>&, int);
    bool db_registered() const { return uid != 0; }
    void reset_uid(unsigned int id) { uid = id; } /* only used by Variable::get_id() */
    void set_pcomp(NetComp * p) { pcomp = p;}     /* set the linked component */

    // inherit from NetComp
    NETLIST_SET_FATHER_DECL;
    NETLIST_STREAMOUT_DECL;
    NETLIST_CHECK_INPARSE_DECL;
    virtual VIdentifier* deep_copy() const;
    virtual void db_register(int iod = 1);
    virtual void db_expunge();
    virtual bool elaborate();

  private:
    Number value;
    std::vector<boost::shared_ptr<Range> > m_range;
    std::vector<boost::shared_ptr<Range> > m_select;
    bool numbered;                 /* true when it is numbered unnamed variable */
    boost::shared_ptr<Variable> pvar;     /* the wire/reg/var in the database */
    NetComp*  pcomp;               /* the netlist component using this variable */
    unsigned int uid;              /* used as the key to search this variable as fanin or fanout */
  };
  NETLIST_STREAMOUT(VIdentifier);


}


#endif
