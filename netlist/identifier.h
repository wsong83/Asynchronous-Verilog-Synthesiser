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

#ifndef _H_IDENTIFIER_
#define _H_IDENTIFIER_

#include "averilog/av_comp.h"

namespace netlist {

  /////////////// base class
  class Identifier : public NetComp{
  public:
    // constructors
    Identifier() {}
    Identifier(NetComp::ctype_t ctype) : NetComp(ctype) {}
    Identifier(NetComp::ctype_t, const string&);

    // helpers
    int compare(const Identifier& rhs) const; /* compare two identifiers */
    virtual ostream& streamout(ostream&) const;
    void hash_update();			   /* update the nearly unique hash id */

    // data
    string name;		/* the name of the identifier */
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
    BIdentifier(const string&);
    BIdentifier();

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
    FIdentifier(const string&);
    
    // helpers

  };
  NETLIST_STREAMOUT(FIdentifier);

  ///////////// module identifier
  class MIdentifier : public Identifier {
  public:
    // constructors
    MIdentifier() : Identifier(NetComp::tModuleName) {}
    MIdentifier(const string&);
    MIdentifier(const averilog::avID& );
    
    // helpers
    MIdentifier& operator++ ();
    ostream& streamout(ostream& os) const;

  private:
    bool numbered;

  };
  NETLIST_STREAMOUT(MIdentifier);

  //////////// instance identifier
  class IIdentifier : public Identifier {
  public:
    // constructors
    IIdentifier();
    IIdentifier(const string&);

    // helpers
    IIdentifier& operator++ ();
    IIdentifier& add_prefix (const Identifier&);

  private:
    bool numbered;
  };
  NETLIST_STREAMOUT(IIdentifier);

  /////////// parameter identifier
  class PaIdentifier : public Identifier {
  public:
    // constructors
    PaIdentifier() : Identifier(NetComp::tParaName) {}
    PaIdentifier(const string&);

    // helpers
  };
  NETLIST_STREAMOUT(PaIdentifier);

  /////////// port identifier
  class PoIdentifier : public Identifier {
  public:
    // constructors
    PoIdentifier() : Identifier(NetComp::tPortName) {}
    PoIdentifier(const string&);
    PoIdentifier(const averilog::avID&);

    // helpers
    void set_range(const vector<Range>& nr) { m_range = nr; }
    const vector<Range>& get_range() const {return m_range;}
    ostream& streamout(ostream& os) const;

  private:
    vector<Range> m_range;

  };
  NETLIST_STREAMOUT(PoIdentifier);

  /////////// Variable identifier
  class VIdentifier : public Identifier {
  public:
    // constructors
    VIdentifier();
    VIdentifier(const string&);
    VIdentifier(const averilog::avID&);
    VIdentifier(const string&, const vector<Range>&);

    //helpers
    VIdentifier& operator++ ();
    VIdentifier& add_prefix (const Identifier&);
    ostream& streamout(ostream& os) const;
    void set_range(const vector<Range>& nr) { m_range = nr; }
    void set_dimension(const vector<Range>& nd) { m_dimension = nd; }
    const vector<Range>& get_range() const {return m_range;}
    const vector<Range>& get_dimension() const {return m_dimension;}
    
    // register the var name in the block database
    template<typename T> 
      bool db_register(DataBase<VIdentifier, T>& db, int inout_t) { /* inout_t: 0 in, 1 out, 2 inout */
      shared_ptr<T>item = db.find(*this);
      if(0 != item.use_count()) { /* found */
        switch(inout_t) {
        case 0: {               /* input */
          item->fin.push_back(this);
          this->db_sig = item;
          this->inout_t = inout_t;
          return true;
        }
        case 1: {
          item->fout.push_back(this);
          this->db_sig = item;
          this->inout_t = inout_t;
          return true;
        }
        default:
          // should not run to here
          assert(0 == "wrong inout type");
        }
        return false;
      } else {
        return false;
      }
    }

    // expunge the register in the block data base
    template<typename T>
      bool db_expunge() {
      if(db_sig.use_count() != 0) return false; /* not registered yet */
      
      shared_ptr<T> item = static_pointer_cast<T>(db_sig);
      
      switch(inout_t) {
      case 0: {               /* input */
        item->fin.remove(this);
        this->db_sig.reset();
        return true;
      }
      case 1: {
        item->fout.remove(this);
        this->db_sig.reset();
        return true;
      }
      default:
        // should not run to here
        assert(0 == "wrong inout type");
      }
      return false;
    }

    // update the registered info in the block data base
    template<typename T>
      bool db_update(DataBase<VIdentifier, T>& db, int inout_t) {
      db_expunge<T>();
      return db_register<T>(db, inout_t);
    }


  private:
    vector<Range> m_range;
    vector<Range> m_dimension;
    bool numbered;
    shared_ptr<NetComp> db_sig; /* the wire/reg/var in the database */
    int inout_t;
  };
  NETLIST_STREAMOUT(VIdentifier);


}


#endif
