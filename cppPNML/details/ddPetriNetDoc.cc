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
cppPNML::details::ddPetriNetDoc::ddPetriNetDoc() 
  : ddObj(PN_PetriNetDoc, "", "") {}

bool cppPNML::details::ddPetriNetDoc::add_petriNet(shared_ptr<ddPetriNet> pn) {
  assert(pn.use_count() != 0);
  if(id_map.count(pn->id)) {   // error, id existed
    cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
      ": The id \"" + pn->id + "\" of Petri-Net \"" + 
      pn->name + "\" is already existed in current Petri-Net document.";
    return false;
  }
  if(!pn->name.empty() && name_map.count(pn->name)) { // error name existed
    cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
      ": Petri-Net \"" + pn->name + 
      "\" is already existed in current Petri-Net document.";
    return false;
  }
  
  // put it into the document
  if(!pn->name.empty()) name_map[pn->name] = pn->id;
  pn_set.insert(pn->id);
  id_map[pn->id] = pn;
  pn->pdoc_ = this;
  return true;
}

bool cppPNML::details::ddPetriNetDoc::add_obj(shared_ptr<ddObj> obj) {
  assert(obj.use_count() != 0);
  if(id_map.count(obj->id)) { // error, id existed
    cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
      ": The id \"" + obj->id + "\" of Petri-Net object \"" + obj->name + 
      "\" is already existed in current Petri-Net document.";
    return false;
  }

  id_map[obj->id] = obj;
  obj->pdoc_ = this;
  return true;
}

shared_ptr<ddPetriNet> cppPNML::details::ddPetriNetDoc::operator() (const string& n) {
  if(name_map.count(n) && count_id(name_map[n]))
    return static_pointer_cast<ddPetriNet>(id_map.find(name_map.find(n)->second)->second);
  else
    return shared_ptr<ddPetriNet>();
}

shared_ptr<const ddPetriNet> cppPNML::details::ddPetriNetDoc::operator() (const string& n) const {
  if(name_map.count(n) && count_id(name_map.find(n)->second))
    return static_pointer_cast<ddPetriNet>(id_map.find(name_map.find(n)->second)->second);
  else
    return shared_ptr<const ddPetriNet>();
}

unsigned int cppPNML::details::ddPetriNetDoc::count_id(const string& i) const {
  return id_map.count(i);
}

unsigned int cppPNML::details::ddPetriNetDoc::count_name(const string& n) const {
  return name_map.count(n);
}

string cppPNML::details::ddPetriNetDoc::get_name(const string& i) const {
  if(id_map.count(i))
    return id_map.find(i)->second->name;
  else 
    return "";
}

string cppPNML::details::ddPetriNetDoc::get_id(const string& n) const {
  if(name_map.count(n))
    return name_map.find(n)->second;
  else
    return "";
}

void cppPNML::details::ddPetriNetDoc::write_pnml(ostream& os) const {
  pugi::xml_document pnml_doc;       // using the pugixml library
  write_pnml(pnml_doc);
  pnml_doc.save(os);
}

void cppPNML::details::ddPetriNetDoc::write_pnml(pugi::xml_document& xml_doc) const {
  // declaration
  pugi::xml_node node_xml = xml_doc.append_child(pugi::node_declaration);
  node_xml.append_attribute("version") = "1.0";
  node_xml.append_attribute("encoding") = "UTF-8";

  // PNML standard
  pugi::xml_node node_pnml = xml_doc.append_child("pnml");
  node_pnml.append_attribute("xmlns") = "http://www.pnml.org/version-2009/grammar/pnml";

  BOOST_FOREACH(const string& i, pn_set) {
    // write out all child petri-net
    // the petri-net should alway be available as it is check when added
    assert(count_id(i)); 
    pugi::xml_node xnode = node_pnml.append_child("net");
    id_map.find(i)->second->write_pnml(xnode);
  }
  
}

bool cppPNML::details::ddPetriNetDoc::read_pnml(istream& istr) {
  pugi::xml_document pnml_doc;       // using the pugixml library
  pugi::xml_parse_result rv = pnml_doc.load(istr, 
                                            pugi::parse_default     |
                                            pugi::parse_declaration |   // declaration
                                            pugi::parse_comments        // commetns
                                            );
  if(!rv) {
    cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
      ": pugixml parse error, " + rv.description();
    return false;
  }

  return read_pnml(pnml_doc);
}

bool cppPNML::details::ddPetriNetDoc::read_pnml(const pugi::xml_document& xml_doc){
  pugi::xml_node node_pnml = xml_doc.child("pnml");
  if(node_pnml.empty()) {
    cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
      ": no <pnml> node. Not a PNML file? ";
    return false;
  }

  // read in all petri nets
  for(pugi::xml_node xnode = node_pnml.first_child(); xnode; xnode = xnode.next_sibling()) {
    
    // check name
    if("net" != string(xnode.name())) {
      cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
      ": invalid child node type \"" + xnode.name() + "\" for <xml>.";
      return false;
    }

    // check id
    string xnode_id = xnode.attribute("id").as_string();
    
    if(xnode_id.empty()) {
      cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
      ": the <pnml> node does not have an identifier.";
      return false;
    }
    
    if(count_id(xnode_id)) {
      cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
      ": the <pnml> \"" + xnode_id + "\" is already existed in the document.";
      return false;
    }
    
    // make a new one and read in data
    shared_ptr<ddPetriNet> new_pn(new ddPetriNet(xnode_id, ""));
    if(!new_pn->read_pnml(xnode, this)) 
      return false;
  }

  return true;
}

template<>
shared_ptr<const ddObj> cppPNML::details::ddPetriNetDoc::get<ddObj> (const string& id) const{
  if(id_map.count(id)) 
    return id_map.find(id)->second;
  else
    return shared_ptr<ddObj>();
}

template<>
shared_ptr<ddObj> cppPNML::details::ddPetriNetDoc::get<ddObj> (const string& id){
  if(id_map.count(id)) 
    return id_map.find(id)->second;
  else
    return shared_ptr<ddObj>();
}

template<>
shared_ptr<const ddNode> cppPNML::details::ddPetriNetDoc::get<ddNode> (const string& id) const{
  shared_ptr<const ddObj> obj = get<ddObj>(id);
  if(obj && (obj->type == PN_Place || obj->type == PN_Transition)) 
    return static_pointer_cast<const ddNode>(obj);
  else
    return shared_ptr<const ddNode>();
}

template<>
shared_ptr<const ddPlace> cppPNML::details::ddPetriNetDoc::get<ddPlace> (const string& id) const{
  shared_ptr<const ddObj> obj = get<ddObj>(id);
  if(obj && obj->type == PN_Place) 
    return static_pointer_cast<const ddPlace>(obj);
  else
    return shared_ptr<const ddPlace>();
}

template<>
shared_ptr<const ddTransition> cppPNML::details::ddPetriNetDoc::get<ddTransition> (const string& id) const{
  shared_ptr<const ddObj> obj = get<ddObj>(id);
  if(obj && obj->type == PN_Transition) 
    return static_pointer_cast<const ddTransition>(obj);
  else 
    return shared_ptr<const ddTransition>();
}

template<>
shared_ptr<const ddArc> cppPNML::details::ddPetriNetDoc::get<ddArc> (const string& id) const{
  shared_ptr<const ddObj> obj = get<ddObj>(id);
  if(!obj && obj->type == PN_Arc) 
    return static_pointer_cast<const ddArc>(obj);
  else
    return shared_ptr<const ddArc>();
}

template<>
shared_ptr<const ddGraph> cppPNML::details::ddPetriNetDoc::get<ddGraph> (const string& id) const{
  shared_ptr<const ddObj> obj = get<ddObj>(id);
  if(obj && obj->type == PN_Graph) 
    return static_pointer_cast<const ddGraph>(obj);
  else 
    return shared_ptr<const ddGraph>();
}

template<>
shared_ptr<const ddPetriNet> cppPNML::details::ddPetriNetDoc::get<ddPetriNet> (const string& id) const{
  shared_ptr<const ddObj> obj = get<ddObj>(id);
  if(obj && obj->type == PN_PetriNet) 
    return static_pointer_cast<const ddPetriNet>(obj);
  else 
    return shared_ptr<const ddPetriNet>();
}

template<>
shared_ptr<ddNode> cppPNML::details::ddPetriNetDoc::get<ddNode> (const string& id){
  shared_ptr<ddObj> obj = get<ddObj>(id);
  if(obj && (obj->type == PN_Place || obj->type == PN_Transition)) 
    return static_pointer_cast<ddNode>(obj);
  else
    return shared_ptr<ddNode>();
}

template<>
shared_ptr<ddPlace> cppPNML::details::ddPetriNetDoc::get<ddPlace> (const string& id){
  shared_ptr<ddObj> obj = get<ddObj>(id);
  if(obj && obj->type == PN_Place) 
    return static_pointer_cast<ddPlace>(obj);
  else
    return shared_ptr<ddPlace>();
}

template<>
shared_ptr<ddTransition> cppPNML::details::ddPetriNetDoc::get<ddTransition> (const string& id){
  shared_ptr<ddObj> obj = get<ddObj>(id);
  if(obj && obj->type == PN_Transition) 
    return static_pointer_cast<ddTransition>(obj);
  else 
    return shared_ptr<ddTransition>();
}

template<>
shared_ptr<ddArc> cppPNML::details::ddPetriNetDoc::get<ddArc> (const string& id) {
  shared_ptr<ddObj> obj = get<ddObj>(id);
  if(obj && obj->type == PN_Arc) 
    return static_pointer_cast<ddArc>(obj);
  else
    return shared_ptr<ddArc>();
}

template<>
shared_ptr<ddGraph> cppPNML::details::ddPetriNetDoc::get<ddGraph> (const string& id){
  shared_ptr<ddObj> obj = get<ddObj>(id);
  if(obj && obj->type == PN_Graph) 
    return static_pointer_cast<ddGraph>(obj);
  else 
    return shared_ptr<ddGraph>();
}

template<>
shared_ptr<ddPetriNet> cppPNML::details::ddPetriNetDoc::get<ddPetriNet> (const string& id) {
  shared_ptr<ddObj> obj = get<ddObj>(id);
  if(obj && obj->type == PN_PetriNet) 
    return static_pointer_cast<ddPetriNet>(obj);
  else 
    return shared_ptr<ddPetriNet>();
}

