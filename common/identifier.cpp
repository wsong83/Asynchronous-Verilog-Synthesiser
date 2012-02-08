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

// for hash function
#include <boost/functional/hash.hpp>

// for convertion between string and int
#include <boost/lexical_cast.hpp>

#include <cstdlib>
#include "identifier.h"

using namespace netlist;

////////////////////////////// Base class /////////////////
netlist::Identifier::Identifier(const std::string& nm)
  : name(nm)
{
  // initialize the hasher
  boost::hash<std::string> s2i;
  
  // calculate the uid
  hashid = s2i(nm);
}

int netlist::Identifier::compare(const Identifier& rhs) const {
  if(hashid != rhs.hashid)
    return hashid > rhs.hashid ? 1 : -1;

  // it cannot be identified by hashids
  return name.compare(rhs.name);
}

std::string netlist::Identifier::to_string() const {
  return name;
}

bool netlist::operator< (const Identifier& lhs, const Identifier& rhs) {
  return lhs.compare(rhs) < 0;
}

bool netlist::operator> (const Identifier& lhs, const Identifier& rhs) {
  return lhs.compare(rhs) > 0;
}

bool netlist::operator== (const Identifier& lhs, const Identifier& rhs) {
  return lhs.compare(rhs) == 0;
}

std::ostream& netlist::operator<< (std::ostream& os, const Identifier& rhs) {
  os << rhs.to_string();
  return os;
}

//////////////////////////////// Block identifier /////////////////
netlist::BIdentifier::BIdentifier(const std::string& nm)
  : Identifier(nm), anonymous(false)  {  }

netlist::BIdentifier::BIdentifier()
  : Identifier("B0"), anonymous(true)  {  }

BIdentifier& netlist::BIdentifier::operator++ () {
  // initialize the hasher
  boost::hash<std::string> s2i;
  
  if(!anonymous) return *this;	// named block idenitifers cannot slef-increase

  // increase the block sequence by 1
  name = std::string("B") + boost::lexical_cast<std::string>(atoi(name.substr(1).c_str()) + 1);

  // recalculate the hashid
  hashid = s2i(name);
  
  return *this;
}

//////////////////////////////// Function identifier /////////////////
netlist::FIdentifier::FIdentifier(const std::string& nm)
  : Identifier(nm) {  }
