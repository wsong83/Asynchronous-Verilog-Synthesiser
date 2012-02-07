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

#include <string>
#include <ostream>

namespace netlist {

  class Identifier {
  public:
    // constructors
    Identifier(const std::string&); 

    // helpers
    int compare(const Identifier& rhs) const; /* compare two identifiers */
    virtual std::string to_string() const = 0; /* print the indentifier, but different for different sub-classes */

    // data
    std::string name;		/* the name of the identifier */
    unsigned int hashid;	/* the nearly unique heash id */

  };
  
  bool operator< (const Identifier& lhs, const Identifier& rhs);
  bool operator> (const Identifier& lhs, const Identifier& rhs);
  bool operator== (const Identifier& lhs, const Identifier& rhs);

  
  // block identifier
  class BIdentifier : public Identifier {
  public:
    // constructors
    BIdentifier(const std::string&);
    BIdentifier();

    // helpers
    virtual std::string to_string() const;
    BIdentifier& operator++ ();
  };

  std::ostream& operator<< (std::ostream&, const BIdentifier&);







}


#endif
