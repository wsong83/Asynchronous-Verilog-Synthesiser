/*
 * Copyright (c) 2012-2012 Wei Song <songw@cs.man.ac.uk> 
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
 * C++ graphic library for handling PNML petri-nets
 * 16/08/2012   Wei Song
 *
 *
 */

// not stand-alone code
// it is included into definitions.cpp
 
cppPNML::details::ddObj::ddObj(pn_t t, const string& i, const string& n)
  : type(t), pdoc_(NULL), pg_(NULL), id(i), name(n) {}

bool cppPNML::details::ddObj::set_name(const string& n) {
  // check the old name
  if(pg_ != NULL && !name.empty()) {
    assert(pg_->count_name(name));
  }

  // check the new name
  if(pg_ != NULL && !n.empty() && name != n) {
    if(pg_->count_name(n)) { // name alreasy existed in the name map
      cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
        ": name \"" + n + "\" is already existed in current graph.";
      return false;
    }
  }

  // renew name
  if(name != n) {
    if(pg_ != NULL) {
      if(!name.empty()) pg_->name_map.erase(name);
      if(!n.empty()) (pg_->name_map)[n] = id;
    }
    name = n;
  }

  return true;
}

void cppPNML::details::ddObj::write_dot(ostream&) const {
  // the write_dot of base class should not be called
  assert(0 == "Error: the write_dot() of the base class is called.");
}

void cppPNML::details::ddObj::write_pnml(pugi::xml_node&) const {
  // the write_pnml of base class should not be called
  assert(0 == "Error: the write_pnml() of the base class is called.");
}

bool cppPNML::details::ddObj::read_pnml(const pugi::xml_node&, ddObj *) {
  // the read_pnml of base class should not be called
  assert(0 == "Error: the read_pnml() of the base class is called.");
  return false;
}

void cppPNML::details::ddObj::write_ogdf(ogdf::Graph *, ogdf::GraphAttributes *,
                                         map<string, void *>&) const {
  // the write_gml of base class should not be called
  assert(0 == "Error: the write_OGDF() of the base class is called.");  
}

void cppPNML::details::ddObj::read_ogdf(void *, ogdf::GraphAttributes *) {
  // the write_gml of base class should not be called
  assert(0 == "Error: the read_OGDF() of the base class is called.");  
}
