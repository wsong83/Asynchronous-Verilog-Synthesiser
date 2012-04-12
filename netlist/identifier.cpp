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
netlist::Identifier::Identifier(ctype_t ctype, const string& nm)
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

ostream& netlist::Identifier::streamout(ostream& os, unsigned int indent) const {
  os << string(indent, ' ') << name;
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
  : Identifier(tBlockName, nm), anonymous(false)  {  }

netlist::BIdentifier::BIdentifier()
  : Identifier(tBlockName, "B0"), anonymous(true)  {  }

netlist::BIdentifier::BIdentifier(const averilog::avID &id)
  : Identifier(tBlockName, id.name), anonymous(false) {  }

BIdentifier& netlist::BIdentifier::operator++ () {
  if(!anonymous) return *this;	// named block idenitifers cannot slef-increase

  // increase the block sequence by 1
  name = string("B") + boost::lexical_cast<string>(atoi(name.substr(1).c_str()) + 1);

  hash_update();
  
  return *this;
}

//////////////////////////////// Function identifier /////////////////
netlist::FIdentifier::FIdentifier(const string& nm)
  : Identifier(tFuncName, nm) {  }


//////////////////////////////// Module identifier /////////////////
netlist::MIdentifier::MIdentifier(const string& nm)
  : Identifier(tModuleName, nm), numbered(false) {  }

netlist::MIdentifier::MIdentifier(const averilog::avID &id)
  : Identifier(tModuleName, id.name), numbered(false) {  }

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

ostream& netlist::MIdentifier::streamout(ostream& os, unsigned int indent) const{
  os << string(indent, ' ') << "module " << name;
  return os;
}
    
//////////////////////////////// Instance identifier /////////////////
netlist::IIdentifier::IIdentifier()
  : Identifier(tInstName, "u_0"), numbered(true) {  }

netlist::IIdentifier::IIdentifier(const string& nm)
  : Identifier(tInstName, nm), numbered(false) {  }

netlist::IIdentifier::IIdentifier(const averilog::avID& id)
  : Identifier(tInstName, id.name), numbered(false) { }

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

//////////////////////////////// port identifier /////////////////
netlist::PoIdentifier::PoIdentifier(const string& nm)
  : Identifier(tPortName, nm) {  }

netlist::PoIdentifier::PoIdentifier(const averilog::avID& id)
  : Identifier(tVarName, id.name) { }

ostream& netlist::PoIdentifier::streamout(ostream& os, unsigned int indent) const {
  os << string(indent, ' ');
  
  vector<shared_ptr<Range> >::const_iterator it, end;
  
  for(it=m_range.begin(), end=m_range.end(); it != end; it++) {
    os << "[" << *(*it) << "]";
  }

  os << " " << name ;

  return os;
}

//////////////////////////////// variable identifier /////////////////
netlist::VIdentifier::VIdentifier()
  : Identifier(tVarName, "n_0"), numbered(true), inout_t(0), uid(0) {  }

netlist::VIdentifier::VIdentifier(const string& nm)
  : Identifier(tVarName, nm), numbered(false), inout_t(0), uid(0) {  }

netlist::VIdentifier::VIdentifier(const averilog::avID& id)
  : Identifier(tVarName, id.name), numbered(false), inout_t(0), uid(0) { }

netlist::VIdentifier::VIdentifier(const string& nm, const vector<shared_ptr<Range> >& rg)
  : Identifier(tVarName, nm), m_range(rg), numbered(false), inout_t(0), uid(0) {  }

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

ostream& netlist::VIdentifier::streamout(ostream& os, unsigned int indent) const {
  vector<shared_ptr<Range> >::const_iterator it, end;

  os << string(indent, ' ') << name;
  for(it=m_select.begin(), end=m_select.end(); it != end; it++) {
    os << "[" << *(*it) << "]";
  }

  return os;
}

void netlist::VIdentifier::db_register(shared_ptr<Variable>& f, int iod) {
  assert(uid == 0);
  assert(iod >= 0 && iod <= 1);

  // store the father and the inout direction
  father = f;
  inout_t = iod;

  // generate a unique id
  uid = f->get_id(iod);
  
  // insert the fanin/out
  pair<map<unsigned int, VIdentifier*>::iterator, bool> rv;
  rv = f->fan[iod].insert(pair<unsigned int, VIdentifier *>(uid, this));
  assert(rv.second);

  // TODO:
  // currently variables in range are not registered or considered, supposing no variable in ranges
  // this must be fixed when parameters are support
  // register in range may also be supported when memory structure is supported for auot synthesis
}

void netlist::VIdentifier::db_register(int iod) {
  assert(father.use_count());
  assert(uid == 0);
  db_register(father, iod);
}

void netlist::VIdentifier::db_register() {
  assert(father.use_count());
  assert(uid == 0);
  db_register(father, inout_t);
}

void netlist::VIdentifier::db_expunge() {
  assert(uid != 0);
  assert(father.use_count());
  
  int rv = father->fan[inout_t].erase(uid);
  assert(rv == 1);
}

VIdentifier* netlist::VIdentifier::deep_copy() const {
  VIdentifier* rv = new VIdentifier(this->name);
  rv->value = this->value;
  rv->numbered = this->numbered;
  rv->father = this->father;
  rv->inout_t = this->inout_t;
  rv->uid = 0;                  // unregistered
  vector<shared_ptr<Range> >::const_iterator it, end;
  for(it=this->m_range.begin(), end=this->m_range.end(); it!=end; it++)
    rv->m_range.push_back(shared_ptr<Range>((*it)->deep_copy()));
  for(it=this->m_select.begin(), end=this->m_select.end(); it!=end; it++)
    rv->m_select.push_back(shared_ptr<Range>((*it)->deep_copy()));
  return rv;
}
  


