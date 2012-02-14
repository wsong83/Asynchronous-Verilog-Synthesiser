/*
 * Copyright (c) 2012 Wei Song <songw@cs.man.ac.uk> 
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

// for hash function
#include <boost/functional/hash.hpp>

// for convertion between string and int
#include <boost/lexical_cast.hpp>

// for regular expression
#include <boost/regex.hpp>

#include <cstdlib>
#include "component.h"

using namespace netlist;

////////////////////////////// Base class /////////////////
netlist::Identifier::Identifier(NetComp::ctype_t ctype, const std::string& nm)
  : NetComp(ctype), name(nm)
{
  hash_update();
}

int netlist::Identifier::compare(const Identifier& rhs) const {
  if(hashid != rhs.hashid)
    return hashid > rhs.hashid ? 1 : -1;

  // it cannot be identified by hashids
  return name.compare(rhs.name);
}

std::ostream& netlist::Identifier::streamout(std::ostream& os) const {
  os << name;
  return os;
}

void netlist::Identifier::hash_update() {
  boost::hash<std::string> s2i;
  hashid = s2i(name);
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

//////////////////////////////// Block identifier /////////////////
netlist::BIdentifier::BIdentifier(const std::string& nm)
  : Identifier(NetComp::tBlockName, nm), anonymous(false)  {  }

netlist::BIdentifier::BIdentifier()
  : Identifier(NetComp::tBlockName, "B0"), anonymous(true)  {  }

BIdentifier& netlist::BIdentifier::operator++ () {
  if(!anonymous) return *this;	// named block idenitifers cannot slef-increase

  // increase the block sequence by 1
  name = std::string("B") + boost::lexical_cast<std::string>(atoi(name.substr(1).c_str()) + 1);

  hash_update();
  
  return *this;
}

//////////////////////////////// Function identifier /////////////////
netlist::FIdentifier::FIdentifier(const std::string& nm)
  : Identifier(NetComp::tFuncName, nm) {  }


//////////////////////////////// Module identifier /////////////////
netlist::MIdentifier::MIdentifier(const std::string& nm)
  : Identifier(NetComp::tModuleName, nm), numbered(false) {  }

netlist::MIdentifier::MIdentifier(const averilog::avID &id)
  : Identifier(NetComp::tModuleName, id.name), numbered(false) {  }

MIdentifier& netlist::MIdentifier::operator++ () {
  const boost::regex numbered_name("_(\\d+)\\z");
  boost::smatch mr;

  if(numbered && boost::regex_search(name, mr, numbered_name)) { // numbered already
    //the new sufix
    std::string new_suffix = 
      std::string("_") + boost::lexical_cast<std::string>(atoi(mr.str().substr(1).c_str()) + 1);
    
    // replace the name
    name = boost::regex_replace(name, numbered_name, new_suffix);
  } else { 			// not numbered yet
    name = name + std::string("_0"); // directly add a suffix
  }

  hash_update();
  numbered = true;

  return *this;
}
    
//////////////////////////////// Instance identifier /////////////////
netlist::IIdentifier::IIdentifier()
  : Identifier(NetComp::tInstName, "u_0"), numbered(true) {  }

netlist::IIdentifier::IIdentifier(const std::string& nm)
  : Identifier(NetComp::tInstName, nm), numbered(false) {  }

IIdentifier& netlist::IIdentifier::operator++ () {
  const boost::regex numbered_name("_(\\d+)\\z");
  boost::smatch mr;

  if(numbered && boost::regex_search(name, mr, numbered_name)) { // numbered already
    //the new sufix
    std::string new_suffix = 
      std::string("_") + boost::lexical_cast<std::string>(atoi(mr.str().substr(1).c_str()) + 1);
    
    // replace the name
    name = boost::regex_replace(name, numbered_name, new_suffix);
  } else { 			// not numbered yet
    name = name + std::string("_0"); // directly add a suffix
  }

  hash_update();
  numbered = true;

  return *this;
}

IIdentifier& netlist::IIdentifier::add_prefix(const Identifier& prefix) {
  name = prefix.name + "_" + name;
  hash_update();
  return *this;
}

//////////////////////////////// parameter identifier /////////////////
netlist::PaIdentifier::PaIdentifier(const std::string& nm)
  : Identifier(NetComp::tParaName, nm) {  }

//////////////////////////////// port identifier /////////////////
netlist::PoIdentifier::PoIdentifier(const std::string& nm)
  : Identifier(NetComp::tPortName, nm) {  }

//////////////////////////////// variable identifier /////////////////
netlist::VIdentifier::VIdentifier()
  : Identifier(NetComp::tVarName, "n_0"), numbered(true) {  }

netlist::VIdentifier::VIdentifier(const std::string& nm)
  : Identifier(NetComp::tVarName, nm), numbered(false) {  }

netlist::VIdentifier::VIdentifier(const averilog::avID& id)
  : Identifier(NetComp::tVarName, id.name), numbered(false) { }

netlist::VIdentifier::VIdentifier(const std::string& nm, const std::vector<Range>& rg)
  : Identifier(NetComp::tVarName, nm), m_range(rg), numbered(false) {  }

VIdentifier& netlist::VIdentifier::operator++ () {
  const boost::regex numbered_name("_(\\d+)\\z");
  boost::smatch mr;

  if(numbered && boost::regex_search(name, mr, numbered_name)) { // numbered already
    //the new sufix
    std::string new_suffix = 
      std::string("_") + boost::lexical_cast<std::string>(atoi(mr.str().substr(1).c_str()) + 1);
    
    // replace the name
    name = boost::regex_replace(name, numbered_name, new_suffix);
  } else { 			// not numbered yet
    name = name + std::string("_0"); // directly add a suffix
  }

  hash_update();
  numbered = true;

  return *this;
}

VIdentifier& netlist::VIdentifier::add_prefix(const Identifier& prefix) {
  name = prefix.name + "_" + name;
  hash_update();
  return *this;
}

