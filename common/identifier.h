/*
 * Copyright (c) 2011 Wei Song <songw@cs.man.ac.uk> 
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
 * 07/02/2011   Wei Song
 *
 *
 */

#ifndef _H_IDENTIFIER_
#define _H_IDENTIFIER_

#include <vector>
#include "component.h"

namespace netlist {

  /////////////// base class
  class Identifier : public NetComp {
  public:
    // constructors
    Identifier(const std::string&); 

    // helpers
    int compare(const Identifier& rhs) const; /* compare two identifiers */
    virtual std::ostream& streamout(std::ostream&) const;
    void hash_update();			   /* update the nearly unique hash id */

    // data
    std::string name;		/* the name of the identifier */
    unsigned int hashid;	/* the nearly unique heash id */

  };
  
  bool operator< (const Identifier& lhs, const Identifier& rhs);
  bool operator> (const Identifier& lhs, const Identifier& rhs);
  bool operator== (const Identifier& lhs, const Identifier& rhs);
  NETLIST_STREAMOUT(Identifier)
  
  ////////////// block identifier
  class BIdentifier : public Identifier {
  public:
    // constructors
    BIdentifier(const std::string&);
    BIdentifier();

    // helpers
    BIdentifier& operator++ ();

  private:
    bool anonymous;
  };

  
  ///////////// function identifier
  class FIdentifier : public Identifier {
  public:
    // constructors
    FIdentifier(const std::string&);
    
    // helpers

  };

  ///////////// module identifier
  class MIdentifier : public Identifier {
  public:
    // constructors
    MIdentifier(const std::string&);
    
    // helpers
    MIdentifier& operator++ ();

  private:
    bool numbered;

  };

  //////////// instance identifier
  class IIdentifier : public Identifier {
  public:
    // constructors
    IIdentifier(const std::string&);

    // helpers
    IIdentifier& operator++ ();
    IIdentifier& add_prefix (const Identifier&);

  private:
    bool numbered;
  };

  /////////// parameter identifier
  class PIdentifier : public Identifier {
  public:
    // constructors
    PIdentifier(const std::string&);

    // helpers
  };

  /////////// Variable identifier
  class VIdentifier : public Identifier {
  public:
    // constructors
    VIdentifier();
    VIdentifier(const std::string&);
    VIdentifier(const std::string&, const std::vector<Range>&);

    //helpers
    VIdentifier& operator++ ();
    VIdentifier& add_prefix (const Identifier&);
    
  private:
    std::vector<Range> m_range;
    bool numbered;
  };


}


#endif
