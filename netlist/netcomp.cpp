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
 * The root class for all netlist component.
 * 30/10/2012   Wei Song
 *
 *
 */

#include "component.h"

using namespace netlist;
using std::ostream;
using std::endl;
using std::list;
using std::map;
using std::string;
using boost::shared_ptr;

netlist::NetComp::NetComp() 
  : ctype(tUnknown), father(NULL) {}

netlist::NetComp::NetComp(ctype_t tt) 
  : ctype(tt), father(NULL) {}

netlist::NetComp::NetComp(ctype_t tt, const shell::location& lloc) 
  : ctype(tt), loc(lloc), father(NULL) {}

void netlist::NetComp::reduce() {
  std::cerr << "ERROR!!, the reduce() of NetComp is used!!!" << std::endl;
  assert(0 == "the reduce() of NetComp is used");
}

ostream& netlist::NetComp::streamout (ostream& os, unsigned int) const {
  os << "ERROR!!, the streamout() of NetComp is used!!!" << endl;
  assert(0 == "the streamout() of NetComp is used");
  return os;
}
    
NetComp* netlist::NetComp::deep_copy( NetComp * rv) const { /* deep copy a netlist component */
  if(src == NULL) {
    std::cerr << "ERROR!!, the deep_copy() of NetComp is used!!! The component type is \"" << get_type_name() << "\"." << endl;
    assert(0 == "the deep_copy() of NetComp is used");
    return NULL;
  } else {
    rv->ctype = ctype;
    rv->loc = loc;
    return rv;
  }
}

void netlist::NetComp::set_father(Block* pf) {
  if((ctype == tNumber) || (ctype == tBlock) || (ctype == tGenBlock)) {
    // here a naked pointer is used because tranfer this to shared_ptr is too complicated to be employed
    father = pf;
  } else {
    std::cerr << "ERROR!!, the set_father() of NetComp is used!!! The component type is \"" << get_type_name() << "\"." << endl;
    assert(0 == "the set_father() of NetComp is used");
    // here a naked pointer is used because tranfer this to shared_ptr is too complicated to be employed
    father = pf;
  }
}

std::string netlist::NetComp::get_hier_name() {
  std::cerr << "ERROR!!, get_hier_name() of NetComp is used!!! The component type is \"" << get_type_name() << "\"." << endl;
  assert(0 == "the get_hier_name() of NetComp is used");
  return "";
}

void netlist::NetComp::replace_variable(const VIdentifier&, const Number&) {
  std::cerr << "ERROR!!, the replace_variable() of NetComp is used!!! The component type is \"" << get_type_name() << "\"." << endl;
  assert(0 == "the replace_variable() of NetComp is used");
}

void netlist::NetComp::replace_variable(const VIdentifier&, const VIdentifier&) {
  std::cerr << "ERROR!!, the replace_variable() of NetComp is used!!! The component type is \"" << get_type_name() << "\"." << endl;
  assert(0 == "the replace_variable() of NetComp is used");  
}

void netlist::NetComp::db_register(int) {
  if(ctype == tNumber) return;
  std::cerr << "ERROR!!, the db_register() of NetComp is used!!! The component type is \"" << get_type_name() << "\"." << endl;
  assert(0 == "the db_register() of NetComp is used");
}

// expunge the variable identifier in the variable database
void netlist::NetComp::db_expunge() {
  if(ctype == tNumber) return;
  std::cerr << "ERROR!!, the db_expunge() of NetComp is used!!! The component type is \"" << get_type_name() << "\"." << endl;
  assert(0 == "the db_expunge() of NetComp is used");
}

bool netlist::NetComp::elaborate(std::set<shared_ptr<NetComp> >&,
                       map<shared_ptr<NetComp>, list<shared_ptr<NetComp> > >&) {
  std::cerr << "ERROR!!, the elaborate() of NetComp is used!!! The component type is \"" << get_type_name() << "\"." << endl;
  assert(0 == "elaborate() of NetComp is used");
  return false;
}

void netlist::NetComp::unfold() {
  std::cerr << "ERROR!! the unfold() of NetComp is used!!! The component type is \"" << get_type_name() << "\"." << endl;
  assert(0 == "unfold() of NetComp is used");
}

unsigned int netlist::NetComp::get_width() const {
  std::cerr << "ERROR!!, the get_width() of NetComp is used!!! The component type is \"" << get_type_name() << "\"." << endl;
  assert(0 == "get_width() of NetComp is used");
  return 0;
}

void netlist::NetComp::set_width(const unsigned int&) {
  std::cerr << "ERROR!!, the set_width() of NetComp is used!!! The component type is \"" << get_type_name() << "\"." << endl;
  assert(0 == "set_width() of NetComp is used");
}

void netlist::NetComp::gen_sdfg(shared_ptr<SDFG::dfgGraph>) {
  std::cerr << "ERROR!!, the gen_sdfg() of NetComp is used!!! The component type is \"" << get_type_name() << "\"." << endl;
  assert(0 == "gen_sdfg() of NetComp is used");
} 

shared_ptr<SDFG::RTree> netlist::NetComp::get_rtree() const {
  std::cerr << "ERROR!!, the get_rtree() of NetComp is used!!! The component type is \"" << get_type_name() << "\"." << endl;
  assert(0 == "get_rtree() of NetComp is used");
  return shared_ptr<SDFG::RTree>();
}

shared_ptr<NetComp> netlist::NetComp::get_sp(){
  return shared_from_this();
}

shared_ptr<Expression> netlist::NetComp::get_combined_expression(const VIdentifier&, std::set<string>) {
  std::cerr << "ERROR!!, the get_combined_expression() of NetComp is used!!! The component type is \"" << get_type_name() << "\"." << endl;
  assert(0 == "get_combined_expression() of NetComp is used");
  return shared_ptr<Expression>();
}

Module* netlist::NetComp::get_module(){
  if(father != NULL) return father->get_module();
  else               return NULL;
}
