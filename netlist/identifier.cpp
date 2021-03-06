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

// for hash function
#include <boost/functional/hash.hpp>

// for convertion between string and int
#include <boost/lexical_cast.hpp>

// for regular expression
#include <boost/regex.hpp>

#include "component.h"
#include "shell/env.h"
#include "sdfg/rtree.hpp"
#include "sdfg/sdfg.hpp"

using namespace netlist;
using std::ostream;
using std::string;
using std::vector;
using boost::shared_ptr;
using shell::location;
using std::pair;
using boost::lexical_cast;

////////////////////////////// Base class /////////////////
netlist::Identifier::Identifier() : numbered(false) {}

netlist::Identifier::Identifier(NetComp::ctype_t ctype) : NetComp(ctype),  numbered(false) {}

netlist::Identifier::Identifier(NetComp::ctype_t ctype, const shell::location& lloc) : NetComp(ctype, lloc), numbered(false) {}

netlist::Identifier::Identifier(ctype_t ctype, const string& nm)
  : NetComp(ctype), name(nm), numbered(false)
{
  hash_update();
}

netlist::Identifier::Identifier(ctype_t ctype, const location& lloc, const string& nm)
  : NetComp(ctype, lloc), name(nm), numbered(false) 
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

Identifier* netlist::Identifier::deep_copy(NetComp *bp) const {
  Identifier *rv;
  if(!bp) rv = new Identifier();
  else    rv = static_cast<Identifier *>(bp); // C++ does not support multiple dispatch
  NetComp::deep_copy(rv);
  rv->name = name;
  rv->hashid = hashid;
  rv->numbered = numbered;
  return rv;
}

void netlist::Identifier::hash_update() {
  boost::hash<string> s2i;
  hashid = s2i(name);
}  

string netlist::Identifier::get_suffix() const {
  const boost::regex numbered_name("_(\\d+)\\z");
  boost::smatch mr;

  if(numbered && boost::regex_search(name, mr, numbered_name))
    return mr.str().substr(1);
  else
    return string();
}

bool netlist::Identifier::replace_suffix(const string& newSuffix) {
  const boost::regex numbered_name("_(\\d+)\\z");
  boost::smatch mr;

  if(numbered && boost::regex_search(name, mr, numbered_name)) {
    set_name(boost::regex_replace(name, numbered_name, "_"+newSuffix));
    numbered = true;
    return true;
  } else {
    return false;
  }
}

void netlist::Identifier::add_suffix(const string& newSuffix) {
  set_name(name + "_" + newSuffix);
  numbered = true;
}

void netlist::Identifier::suffix_increase () {
  string oldSuffix = get_suffix();
  if(oldSuffix.size()) {	// already has a suffix
    string newSuffix = lexical_cast<string>(atoi(oldSuffix.c_str())+1);
    replace_suffix(newSuffix);
  } else {
    add_suffix("0");
  }
}

void netlist::Identifier::add_prefix(const string& newPrefix) {
  set_name(newPrefix + "." + name);
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

bool netlist::operator!= (const Identifier& lhs, const Identifier& rhs) {
  return lhs.compare(rhs) != 0;
}

//////////////////////////////// Block identifier /////////////////
netlist::BIdentifier::BIdentifier(const string& nm)
  : Identifier(tBlockName, nm), anonymous(false)  {  }

netlist::BIdentifier::BIdentifier(const location& lloc, const string& nm)
  : Identifier(tBlockName, lloc, nm), anonymous(false)  {  }

netlist::BIdentifier::BIdentifier()
  : Identifier(tBlockName, "B"), anonymous(true)  {  }

netlist::BIdentifier::BIdentifier(const location& lloc)
  : Identifier(tBlockName, lloc, "B"), anonymous(true)  {  }

netlist::BIdentifier::BIdentifier(const averilog::avID &id)
  : Identifier(tBlockName, id.name), anonymous(false) {  }

netlist::BIdentifier::BIdentifier(const location& lloc, const averilog::avID &id)
  : Identifier(tBlockName, lloc, id.name), anonymous(false) {  }

/*
BIdentifier& netlist::BIdentifier::operator++ () {
  if(!anonymous) return *this;	// named block idenitifers cannot slef-increase

  // increase the block sequence by 1
  string origName = get_name();
  unsigned int index = atoi(name.substr(1).c_str()) + 1;
  string newName = "B" + lexical_cast<string>(index);
  set_name(newName);  
  return *this;
}
*/

//////////////////////////////// Function identifier /////////////////
netlist::FIdentifier::FIdentifier() : Identifier(NetComp::tFuncName) { }

netlist::FIdentifier::FIdentifier(const string& nm)
  : Identifier(tFuncName, nm) {  }

netlist::FIdentifier::FIdentifier(const location& lloc, const string& nm)
  : Identifier(tFuncName, lloc, nm) {  }

netlist::FIdentifier::FIdentifier(const location& lloc, const averilog::avID &id)
  : Identifier(tFuncName, lloc, id.name) {  }

//////////////////////////////// Module identifier /////////////////
netlist::MIdentifier::MIdentifier() 
  : Identifier(tModuleName) {}

netlist::MIdentifier::MIdentifier(const shell::location& lloc) 
  : Identifier(tModuleName, lloc) {}

netlist::MIdentifier::MIdentifier(const string& nm)
  : Identifier(tModuleName, nm) {  }

netlist::MIdentifier::MIdentifier(const location& lloc, const string& nm)
  : Identifier(tModuleName, lloc, nm) {  }

netlist::MIdentifier::MIdentifier(const averilog::avID &id)
  : Identifier(tModuleName, id.name) {  }

netlist::MIdentifier::MIdentifier(const location& lloc, const averilog::avID &id)
  : Identifier(tModuleName, lloc, id.name) {  }

ostream& netlist::MIdentifier::streamout(ostream& os, unsigned int indent) const{
  os << string(indent, ' ') << "module " << name;
  return os;
}
    
//////////////////////////////// Instance identifier /////////////////
netlist::IIdentifier::IIdentifier()
  : Identifier(tInstName, "u") { add_suffix("0"); }

netlist::IIdentifier::IIdentifier(const location& lloc)
  : Identifier(tInstName, lloc, "u") { add_suffix("0"); }

netlist::IIdentifier::IIdentifier(const string& nm)
  : Identifier(tInstName, nm) {  }

netlist::IIdentifier::IIdentifier(const location& lloc, const string& nm)
  : Identifier(tInstName, lloc, nm) {  }

netlist::IIdentifier::IIdentifier(const averilog::avID& id)
  : Identifier(tInstName, id.name) { }

netlist::IIdentifier::IIdentifier(const location& lloc, const averilog::avID& id)
  : Identifier(tInstName, lloc, id.name) { }

//////////////////////////////// port identifier /////////////////
netlist::PoIdentifier::PoIdentifier() 
  : Identifier(NetComp::tPortName) {}

netlist::PoIdentifier::PoIdentifier(const shell::location& lloc) 
  : Identifier(NetComp::tPortName, lloc) {}

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
  : Identifier(tVarName, "n"), uid(0) { add_suffix("0"); }

netlist::VIdentifier::VIdentifier(const location& lloc)
  : Identifier(tVarName, lloc, "n"), uid(0) {  }

netlist::VIdentifier::VIdentifier(const string& nm)
  : Identifier(tVarName, nm), uid(0) {  }

netlist::VIdentifier::VIdentifier(const location& lloc, const string& nm)
  : Identifier(tVarName, lloc, nm), uid(0) {  }

netlist::VIdentifier::VIdentifier(const averilog::avID& id)
  : Identifier(tVarName, id.name), uid(0) { }

netlist::VIdentifier::VIdentifier(const location& lloc, const averilog::avID& id)
  : Identifier(tVarName, lloc, id.name), uid(0) { }

netlist::VIdentifier::VIdentifier(const string& nm, const RangeArray& rg)
  : Identifier(tVarName, nm), m_range(rg), uid(0) {  }

netlist::VIdentifier::VIdentifier(const location& lloc, const string& nm, const RangeArray& rg)
  : Identifier(tVarName, lloc, nm), m_range(rg), uid(0) {  }

netlist::VIdentifier::~VIdentifier() {
  db_expunge();
}

const RangeArray& netlist::VIdentifier::get_full_range() const {
  return pvar->name.get_range();
}

bool netlist::VIdentifier::is_valuable() const {
  return value.is_valid() && m_select.is_valuable();
}

Number netlist::VIdentifier::get_value() const {
  assert(value.is_valid());
  assert(m_select.RangeArrayCommon::is_empty() || // no selector
         (pvar && uid != 0 && m_select.is_valuable())); // otherwise the parent variable is set
  
  if(m_select.RangeArrayCommon::is_empty()) return value;

  // otherwise need calculate the specific range
  assert(m_select.is_selection()); // multi-range is not supported yet, and seems not to be
    

  string txt_value = value.get_txt_value();
  
  // get the range
  pair<long, long> str_range = 
    pvar->name.get_range().get_flat_range(m_select).get_plain_range();

  // get the range in string
  str_range.first = (long)(txt_value.size()) - str_range.first - 1;
  str_range.second = (long)(txt_value.size()) - str_range.second - 1;
  
  // remove range less than 0
  str_range.first = str_range.first < 0 ? 0 : str_range.first;
  str_range.second = str_range.second;
  unsigned long str_size = 
    str_range.second >= str_range.first ? 
    str_range.second - str_range.first + 1 : 0;
  
  // get the substr
  if(str_size == 0) return 0;
  else return Number(txt_value.substr(str_range.first, str_size));
}

string netlist::VIdentifier::get_selected_name() const {
  if(m_select.is_empty() || !m_select.is_valuable())
    return get_name() + toString(get_full_range());
  else {                        // need to output selector
    return get_name() + toString(m_select);
  }
}

void netlist::VIdentifier::reduce() {
  m_range.reduce();
  m_select.reduce();
}

std::set<shared_ptr<NetComp> >  netlist::VIdentifier::get_drivers() const {
  assert(pvar);
  assert(pvar->pDFGNode);
  assert(pvar->pDFGNode->ptr.size());
  return pvar->pDFGNode->ptr;
}


void netlist::VIdentifier::set_father(Block *pf) {
  if(father == pf) return;
  Identifier::set_father(pf);
  m_range.set_father(pf);
  m_select.set_father(pf);
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
    if(!mvar) { // this variable is not defined yet
      // not sure whether this is needed.
      // so assert it 
      //assert(0 == "really need to define a new variable! Analyse this case...");
      G_ENV->error(loc, "SYN-VAR-3", name);
      // define the variable in the lowest block as a wire
      mvar.reset(new Variable(*this, Variable::TWire));
      //mvar->name.db_expunge();  // why do I need to expunge it?
      bool rv = father->db_var.insert(mvar->name, mvar);
      assert(rv);
      mvar->set_father(father);
      mvar->db_register(1);
    }
  } else {
    mvar = pvar;
  }
  db_register(mvar, iod);
}

void netlist::VIdentifier::db_expunge() {
  if(uid != 0) {
    //std::cout << "expunge " << *this << std::endl;
    pvar->fan[0].erase(uid);
    pvar->fan[1].erase(uid);
  }
  uid = 0;
  pvar.reset();

  // expunge all range and selectors
  m_range.db_expunge();
  m_select.db_expunge();
}

VIdentifier* netlist::VIdentifier::deep_copy(NetComp* bp) const {
  VIdentifier *rv;
  if(!bp) rv = new VIdentifier();
  else    rv = static_cast<VIdentifier *>(bp); // C++ does not support multiple dispatch
  Identifier::deep_copy(rv);
  rv->value = this->value;
  rv->uid = 0;                  // unregistered
  rv->m_range = m_range.deep_object_copy();
  rv->m_select = m_select.deep_object_copy();
  return rv;
}

void netlist::VIdentifier::replace_variable(const VIdentifier& var, const Number& num) {
  m_range.replace_variable(var, num);
  m_select.replace_variable(var, num);
}

void netlist::VIdentifier::replace_variable(const VIdentifier& var, const VIdentifier& nvar) {
  m_range.replace_variable(var, nvar);
  m_select.replace_variable(var, nvar);
}

SDFG::RTree netlist::VIdentifier::get_rtree() const {
  SDFG::RTree rv;
  std::pair<string, SDFG::dfgRange> npair = 
    SDFG::divide_signal_name(SDFG::get_full_selected_name(get_selected_name(),
                                                          toString(get_full_range())));
  SDFG::RRelation var(npair.first, SDFG::dfgRangeMap(npair.second));
  rv.add(var);
  rv.combine(get_select().get_rtree().assign_type(SDFG::dfgEdge::SDFG_ADR));
  return rv;
}

unsigned int netlist::VIdentifier::get_width() const {
  assert(pvar);
  if(m_select.is_empty()) {
    return pvar->get_width();
  } else {
    return m_select.get_width(pvar->name.m_range);
  }
}
