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
#include "shell/env.h"

using namespace netlist;
using std::ostream;
using std::string;
using boost::shared_ptr;
using shell::location;
using std::pair;

////////////////////////////// Base class /////////////////
netlist::Identifier::Identifier(ctype_t ctype, const string& nm)
  : NetComp(ctype), name(nm)
{
  hash_update();
}

netlist::Identifier::Identifier(ctype_t ctype, const location& lloc, const string& nm)
  : NetComp(ctype, lloc), name(nm)
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

void netlist::Identifier::set_father(Block *pf) {
  father = pf;
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

netlist::BIdentifier::BIdentifier(const location& lloc, const string& nm)
  : Identifier(tBlockName, lloc, nm), anonymous(false)  {  }

netlist::BIdentifier::BIdentifier()
  : Identifier(tBlockName, "B0"), anonymous(true)  {  }

netlist::BIdentifier::BIdentifier(const location& lloc)
  : Identifier(tBlockName, lloc, "B0"), anonymous(true)  {  }

netlist::BIdentifier::BIdentifier(const averilog::avID &id)
  : Identifier(tBlockName, id.name), anonymous(false) {  }

netlist::BIdentifier::BIdentifier(const location& lloc, const averilog::avID &id)
  : Identifier(tBlockName, lloc, id.name), anonymous(false) {  }

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

netlist::FIdentifier::FIdentifier(const location& lloc, const string& nm)
  : Identifier(tFuncName, lloc, nm) {  }


//////////////////////////////// Module identifier /////////////////
netlist::MIdentifier::MIdentifier(const string& nm)
  : Identifier(tModuleName, nm), numbered(false) {  }

netlist::MIdentifier::MIdentifier(const location& lloc, const string& nm)
  : Identifier(tModuleName, lloc, nm), numbered(false) {  }

netlist::MIdentifier::MIdentifier(const averilog::avID &id)
  : Identifier(tModuleName, id.name), numbered(false) {  }

netlist::MIdentifier::MIdentifier(const location& lloc, const averilog::avID &id)
  : Identifier(tModuleName, lloc, id.name), numbered(false) {  }

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

netlist::IIdentifier::IIdentifier(const location& lloc)
  : Identifier(tInstName, lloc, "u_0"), numbered(true) {  }

netlist::IIdentifier::IIdentifier(const string& nm)
  : Identifier(tInstName, nm), numbered(false) {  }

netlist::IIdentifier::IIdentifier(const location& lloc, const string& nm)
  : Identifier(tInstName, lloc, nm), numbered(false) {  }

netlist::IIdentifier::IIdentifier(const averilog::avID& id)
  : Identifier(tInstName, id.name), numbered(false) { }

netlist::IIdentifier::IIdentifier(const location& lloc, const averilog::avID& id)
  : Identifier(tInstName, lloc, id.name), numbered(false) { }

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

netlist::PoIdentifier::PoIdentifier(const location& lloc, const string& nm)
  : Identifier(tPortName, lloc, nm) {  }

netlist::PoIdentifier::PoIdentifier(const averilog::avID& id)
  : Identifier(tVarName, id.name) { }

netlist::PoIdentifier::PoIdentifier(const location& lloc, const averilog::avID& id)
  : Identifier(tVarName, lloc, id.name) { }

void netlist::PoIdentifier::set_father(Block *pf) {
  if(father == pf) return;
  Identifier::set_father(pf);
  m_range.set_father(pf);
}

ostream& netlist::PoIdentifier::streamout(ostream& os, unsigned int indent) const {
  os << string(indent, ' ');
  os << m_range << " " << name ;
  return os;
}

//////////////////////////////// variable identifier /////////////////
netlist::VIdentifier::VIdentifier()
  : Identifier(tVarName, "n_0"), numbered(true), pcomp(NULL), uid(0), alwaysp(NULL) {  }

netlist::VIdentifier::VIdentifier(const location& lloc)
  : Identifier(tVarName, lloc, "n_0"), numbered(true), pcomp(NULL), uid(0), alwaysp(NULL) {  }

netlist::VIdentifier::VIdentifier(const string& nm)
  : Identifier(tVarName, nm), numbered(false), pcomp(NULL), uid(0), alwaysp(NULL) {  }

netlist::VIdentifier::VIdentifier(const location& lloc, const string& nm)
  : Identifier(tVarName, lloc, nm), numbered(false), pcomp(NULL), uid(0), alwaysp(NULL) {  }

netlist::VIdentifier::VIdentifier(const averilog::avID& id)
  : Identifier(tVarName, id.name), numbered(false), pcomp(NULL), uid(0), alwaysp(NULL) { }

netlist::VIdentifier::VIdentifier(const location& lloc, const averilog::avID& id)
  : Identifier(tVarName, lloc, id.name), numbered(false), pcomp(NULL), uid(0), alwaysp(NULL) { }

netlist::VIdentifier::VIdentifier(const string& nm, const RangeArray& rg)
  : Identifier(tVarName, nm), m_range(rg), numbered(false), pcomp(NULL), uid(0), alwaysp(NULL) {  }

netlist::VIdentifier::VIdentifier(const location& lloc, const string& nm, const RangeArray& rg)
  : Identifier(tVarName, lloc, nm), m_range(rg), numbered(false), pcomp(NULL), uid(0), alwaysp(NULL) {  }

netlist::VIdentifier::~VIdentifier() {
  db_expunge();
}

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

void netlist::VIdentifier::set_father(Block *pf) {
  if(father == pf) return;
  Identifier::set_father(pf);
  m_range.set_father(pf);
  m_select.set_father(pf);
}

void netlist::VIdentifier::set_always_pointer(SeqBlock* p) {
  alwaysp = p;
}

bool netlist::VIdentifier::check_inparse() {
  bool rv = true;

  // check whether the variable is declared before use
  if((father->gfind_var(*this)).use_count() == 0) {
    G_ENV->error(loc, "SYN-VAR-3", name);
    father->get_module()->db_var.insert(*this, shared_ptr<Variable>(new Variable(loc, *this, Variable::TWire)));
    rv = false;
  }
  
  rv &= m_range.check_inparse();
  rv &= m_select.check_inparse();

  return rv;
}

ostream& netlist::VIdentifier::streamout(ostream& os, unsigned int indent) const {
  os << string(indent, ' ');
  m_select.RangeArrayCommon::streamout(os, 0, name);
  return os;
}

void netlist::VIdentifier::db_register(const shared_ptr<Variable>& f, int iod) {
  pvar = f;

  if(uid == 0) {                // generate a new uid
    uid = pvar->get_id();
  }

  // store my pointer to the fan database in the variable object
  pvar->fan[iod][uid] = this;

  // update the value
  if(pvar->is_valuable()) value = pvar->get_value();

  // register all range and selectors
  m_range.db_register(1);
  m_select.db_register(1);
  
}

void netlist::VIdentifier::db_register(int iod) {
  shared_ptr<Variable> mvar;
  if(uid == 0) { // the root Variable unkown yet, need to find it out
    mvar = father->gfind_var(*this);
    if(mvar.use_count() == 0) { // this variable is not defined yet
      // not sure whether this is needed.
      // so assert it 
      assert(0 == "really need to define a new variable! Analyse this case...");
      // define the variable in the lowest block as a wire
      mvar.reset(new Variable(*this, Variable::TWire));
      //mvar->name.db_expunge();  // why do I need to expunge it?
      father->db_var.insert(mvar->name, mvar);
    }
  } else {
    mvar = pvar;
  }
  db_register(mvar, iod);
}

void netlist::VIdentifier::db_expunge() {
  if(uid != 0) {
    pvar->fan[0].erase(uid);
    pvar->fan[1].erase(uid);
  }
  uid = 0;
  pvar.reset();

  // expunge all range and selectors
  m_range.db_expunge();
  m_select.db_expunge();
}

VIdentifier* netlist::VIdentifier::deep_copy() const {
  VIdentifier* rv = new VIdentifier(this->loc, this->name);
  rv->value = this->value;
  rv->numbered = this->numbered;
  rv->uid = 0;                  // unregistered
  rv->m_range = m_range.deep_object_copy();
  rv->m_select = m_select.deep_object_copy();
  return rv;
}
  
bool netlist::VIdentifier::elaborate(elab_result_t &result, const ctype_t mctype, const vector<NetComp *>& fp) {
  bool rv = true;
  result = ELAB_Normal;
  
  // check the basic link info.
  assert(uid != 0);
  assert(pvar.use_count() != 0); // variable registered
    
  // depending on the type of linked component, checking range and selector
  switch(mctype) {
  case tUnknown :
    assert(0 == "the linked component should not be an unkown component!");
    rv = false;
    break;
  case tExp: {
    // for an expression, no range is used
    assert(m_range.size() == 0);
    rv &= m_select.elaborate(result, mctype, fp);
    // rv &= m_select.is_valuable();
    // it might have variable range in always blocks and it is existed in OR1200, so let it pass
    //if(!rv) { G_ENV->error(loc, "ELAB-RANGE-0", name); }
    break;
  }
  case tPort: 
  case tVariable: {
    // for a port, range should be resolved numbers
    assert(m_select.size() == 0);
    rv &= m_range.elaborate(result);
    rv &= m_range.is_valuable();
    if(!rv) { G_ENV->error(loc, "ELAB-RANGE-0", name); }
    break;
  }
  case tLConcatenation: {
    // for a left-side concatenation, select should be resolved numbers
    assert(m_range.size() == 0);
    rv &= m_select.elaborate(result);
    // rv &= m_select.is_valuable();
    // it might have variable range in always blocks and it is existed in OR1200, so let it pass
    //if(!rv) { G_ENV->error(loc, "ELAB-RANGE-0", name); }
    break;
  }    
  default:
    assert(0 == "this type of component has not been processed here!");
    rv=false;
  }

  return rv;
}
