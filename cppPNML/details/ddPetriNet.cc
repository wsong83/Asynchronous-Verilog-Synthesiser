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
cppPNML::details::ddPetriNet::ddPetriNet(const string& i, const string& n, pnml_t t)
  : ddObj(PN_PetriNet, i, n),
    pnml_type(t) {}

unsigned int cppPNML::details::ddPetriNet::count_name(const string& n) const {
  return name_map.count(n);
}

shared_ptr<ddGraph> cppPNML::details::ddPetriNet::operator() (const string& n) {
  if(name_map.count(n) && pdoc_ != NULL && pdoc_->count_id(name_map[n]))
    return static_pointer_cast<ddGraph>(pdoc_->id_map.find(name_map.find(n)->second)->second);
  else
    return shared_ptr<ddGraph>();
}

shared_ptr<const ddGraph> cppPNML::details::ddPetriNet::operator() (const string& n) const {
  if(name_map.count(n) && pdoc_ != NULL && pdoc_->count_id(name_map.find(n)->second))
    return static_pointer_cast<ddGraph>(pdoc_->id_map.find(name_map.find(n)->second)->second);
  else
    return shared_ptr<const ddGraph>();
}

bool cppPNML::details::ddPetriNet::add(shared_ptr<ddGraph> g) {
  if(g.use_count() == 0) {    // object empty
    cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
      ": object is empty.";
    return false;
  }
  if(pdoc_ == NULL) {           // error, doc not assigned
    cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
      ": link the Petri-Net to a Petri-Net document before adding an object.";
    return false;
  }
  if(pdoc_->count_id(g->id)) {   // error, id existed
    cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
      ": The id \"" + g->id + "\" is already existed in current Petri-Net document.";
    return false;
  } 
  if(!g->name.empty() && name_map.count(g->name)) { // error name existed
    cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
      ": node \"" + g->name + "\" is already existed in current Petri-Net.";
    return false;
  }
  switch(g->type) {
  case PN_Graph:
    break;
  default:
    cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
      ": illegal object type";
    return false;
  }
  
  // put it into the document
  if(!g->name.empty()) name_map[g->name] = g->id;
  page_set.insert(g->id);
  g->ppn_ = this;
  pdoc_->add_obj(g);
  return true;
}

string cppPNML::details::ddPetriNet::get_id(const string& n) const {
  if(name_map.count(n))
    return name_map.find(n)->second;
  else
    return "";
}

void cppPNML::details::ddPetriNet::write_pnml(pugi::xml_node& xnode) const {

  xnode.append_attribute("id") = id.c_str();

  string type_str;
  switch(pnml_type) {
  case PNML_HLPNG: type_str = "highlevelnet"; break;
  case PNML_SN:    type_str = "symmetricnet"; break;
  case PNML_PT:    type_str = "ptnet";        break;
  default:         type_str = "highlevelnet"; // I believe high-level Petri-Net is the most compatible net format
  }
  xnode.append_attribute("type")
    = ("http://www.pnml.org/version-2009/grammar/" + type_str).c_str();

  // net name
  if(!name.empty()) {
    xnode.append_child("name").append_child("text").text() = name.c_str();
  }

  BOOST_FOREACH(const string& i, page_set) {
    // write out all child pages
    // the page should alway be available as it is check when added
    assert(pdoc_ != NULL);
    assert(pdoc_->count_id(i)); 
    pugi::xml_node xpage = xnode.append_child("page");
    pdoc_->id_map.find(i)->second->write_pnml(xpage);
  }
}

bool cppPNML::details::ddPetriNet::read_pnml(const pugi::xml_node& xpn, ddObj* pf) {
  // find out the hash id
  if(0 == 1) {
    show_hash("highlevelnet");   // 0x4ddb7275
    show_hash("symmetricnet");   // 0xac1bf274
    show_hash("ptnet");          // 0xe9bb2f4
    return true;
  }

  // get the PNML net type
  boost::smatch mresult;
  boost::regex regex_exp("/([^/]+)$");
  string pn_type = xpn.attribute("type").as_string();
  if(boost::regex_search(pn_type, mresult, regex_exp)) {
    switch(shash(boost::algorithm::to_lower_copy(string(mresult[1])))) {
    case 0x4ddb7275: pnml_type = PNML_HLPNG; break;
    case 0xac1bf274: pnml_type = PNML_SN;    break;
    case 0xe9bb2f4:  pnml_type = PNML_PT;    break;
    default:
      cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
        ": <net> invalid net type \"" + pn_type +"\".";
      return false;      
    }
  } else {
    cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
      ": <net> invalid net type \"" + pn_type +"\".";
    return false;
  }

  // get the name if any
  pugi::xml_node xname = xpn.child("name");
  if(xname)
    set_name(xname.child("text").text().as_string());

  // add the Petri Net to the document
  if(!static_cast<ddPetriNetDoc *>(pf)
     ->add_petriNet(
                    static_pointer_cast<ddPetriNet>
                    (shared_from_this())
                    )
     ) return false;
  
  // process all childs
  for(pugi::xml_node xpage = xpn.child("page"); xpage; xpage = xpage.next_sibling("page")) {
    string xid = xpage.attribute("id").as_string();
    
    if(xid.empty()) {
      cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
        ": the <page> node does not have an identifier.";
      return false;
    }
    
    if(pdoc_->count_id(xid)) {
      cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
        ": the <page> \"" + xid + "\" is already existed in the document.";
      return false;
    }
    
    // make a new one and read in data
    shared_ptr<ddGraph> nGraph(new ddGraph(xid, ""));
    if(!nGraph->read_pnml(xpage, this)) 
      return false;
  }
  
  return true;
}

