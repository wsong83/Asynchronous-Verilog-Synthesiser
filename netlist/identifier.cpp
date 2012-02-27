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

// for hash function
#include <boost/functional/hash.hpp>

// for convertion between string and int
#include <boost/lexical_cast.hpp>

// for regular expression
#include <boost/regex.hpp>

#include "component.h"

using namespace netlist;

////////////////////////////// Base class /////////////////
netlist::Identifier::Identifier(NetComp::ctype_t ctype, const string& nm)
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

ostream& netlist::Identifier::streamout(ostream& os) const {
  os << name;
  return os;
}

void netlist::Identifier::hash_update() {
  boost::hash<string> s2i;
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
netlist::BIdentifier::BIdentifier(const string& nm)
  : Identifier(NetComp::tBlockName, nm), anonymous(false)  {  }

netlist::BIdentifier::BIdentifier()
  : Identifier(NetComp::tBlockName, "B0"), anonymous(true)  {  }

BIdentifier& netlist::BIdentifier::operator++ () {
  if(!anonymous) return *this;	// named block idenitifers cannot slef-increase

  // increase the block sequence by 1
  name = string("B") + boost::lexical_cast<string>(atoi(name.substr(1).c_str()) + 1);

  hash_update();
  
  return *this;
}

//////////////////////////////// Function identifier /////////////////
netlist::FIdentifier::FIdentifier(const string& nm)
  : Identifier(NetComp::tFuncName, nm) {  }


//////////////////////////////// Module identifier /////////////////
netlist::MIdentifier::MIdentifier(const string& nm)
  : Identifier(NetComp::tModuleName, nm), numbered(false) {  }

netlist::MIdentifier::MIdentifier(const averilog::avID &id)
  : Identifier(NetComp::tModuleName, id.name), numbered(false) {  }

MIdentifier& netlist::MIdentifier::operator++ () {
  const boost::regex numbered_name("_(\\d+)\\z");
  boost::smatch mr;

  if(numbered && boost::regex_search(name, mr, numbered_name)) { // numbered already
    //the new sufix
    string new_suffix = 
      string("_") + boost::lexical_cast<string>(atoi(mr.str().substr(1).c_str()) + 1);
    
    // replace the name
    name = boost::regex_replace(name, numbered_name, new_suffix);
  } else { 			// not numbered yet
    name = name + string("_0"); // directly add a suffix
  }

  hash_update();
  numbered = true;

  return *this;
}

ostream& netlist::MIdentifier::streamout(ostream& os) const{
  os << "module " << name;
  return os;
}
    
//////////////////////////////// Instance identifier /////////////////
netlist::IIdentifier::IIdentifier()
  : Identifier(NetComp::tInstName, "u_0"), numbered(true) {  }

netlist::IIdentifier::IIdentifier(const string& nm)
  : Identifier(NetComp::tInstName, nm), numbered(false) {  }

IIdentifier& netlist::IIdentifier::operator++ () {
  const boost::regex numbered_name("_(\\d+)\\z");
  boost::smatch mr;

  if(numbered && boost::regex_search(name, mr, numbered_name)) { // numbered already
    //the new sufix
    string new_suffix = 
      string("_") + boost::lexical_cast<string>(atoi(mr.str().substr(1).c_str()) + 1);
    
    // replace the name
    name = boost::regex_replace(name, numbered_name, new_suffix);
  } else { 			// not numbered yet
    name = name + string("_0"); // directly add a suffix
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
netlist::PaIdentifier::PaIdentifier(const string& nm)
  : Identifier(NetComp::tParaName, nm) {  }

//////////////////////////////// port identifier /////////////////
netlist::PoIdentifier::PoIdentifier(const string& nm)
  : Identifier(NetComp::tPortName, nm) {  }

netlist::PoIdentifier::PoIdentifier(const averilog::avID& id)
  : Identifier(NetComp::tVarName, id.name) { }

ostream& netlist::PoIdentifier::streamout(ostream& os) const {
  vector<Range>::const_iterator it, end;
  
  for(it=m_range.begin(), end=m_range.end(); it != end; it++) {
    os << "[" << *it << "]";
  }

  os << " " << name ;

  return os;
}

//////////////////////////////// variable identifier /////////////////
netlist::VIdentifier::VIdentifier()
  : Identifier(NetComp::tVarName, "n_0"), numbered(true) {  }

netlist::VIdentifier::VIdentifier(const string& nm)
  : Identifier(NetComp::tVarName, nm), numbered(false) {  }

netlist::VIdentifier::VIdentifier(const averilog::avID& id)
  : Identifier(NetComp::tVarName, id.name), numbered(false) { }

netlist::VIdentifier::VIdentifier(const string& nm, const vector<Range>& rg)
  : Identifier(NetComp::tVarName, nm), m_range(rg), numbered(false) {  }

netlist::VIdentifier::VIdentifier(const VIdentifier& rhs)
  : Identifier(NetComp::tVarName, rhs.name), 
    m_range(rhs.m_range), m_dimension(rhs.m_dimension),
    db_sig(rhs.db_sig), inout_t(rhs.inout_t), numbered(false) { }

VIdentifier& netlist::VIdentifier::operator++ () {
  const boost::regex numbered_name("_(\\d+)\\z");
  boost::smatch mr;

  if(numbered && boost::regex_search(name, mr, numbered_name)) { // numbered already
    //the new sufix
    string new_suffix = 
      string("_") + boost::lexical_cast<string>(atoi(mr.str().substr(1).c_str()) + 1);
    
    // replace the name
    name = boost::regex_replace(name, numbered_name, new_suffix);
  } else { 			// not numbered yet
    name = name + string("_0"); // directly add a suffix
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

ostream& netlist::VIdentifier::streamout(ostream& os) const {
  vector<Range>::const_iterator it, end;

  //os << "wire ";
  for(it=m_range.begin(), end=m_range.end(); it != end; it++) {
    os << "[" << *it << "]";
  }
  os << name;
  for(it=m_dimension.begin(), end=m_dimension.end(); it != end; it++) {
    os << "[" << *it << "]";
  }

  return os;
}

shared_ptr<VIdentifier> netlist::VIdentifier::deep_copy() const {
  shared_ptr<VIdentifier> rv(new VIdentifier(*this));

  for(int i=m_range.size()-1; i>=0; i++ )
    rv->m_range[i] = m_range[i].deep_copy();
  
  for(int i=m_dimension.size()-1; i>=0; i++ )
    rv->m_dimension[i] = m_dimension[i].deep_copy();
  
  switch(db_sig->get_type()) {
  case NetComp::tWire: {
    bool result = rv->db_register<Wire>(*(db_sig->dbp), inout_t);
    assert(result);
    break;
  }
  case NetComp::tRegister: {
    bool result = rv->db_register<Register>(*(db_sig->dbp), inout_t);
    assert(result);
    break;
  }
  default:
    // should not run to here
    assert(0 == "Variable type wrong");
  }

  return rv;

}  

