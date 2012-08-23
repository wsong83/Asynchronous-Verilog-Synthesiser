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
cppPNML::details::ddNode::ddNode(pn_t t, const string& i, const string& n)
  : ddObj(t, i, n), fBGL(false), pos(0,0), shape(0,0) {}

bool cppPNML::details::ddNode::set_ref_node(const string& nrefId) {
  // check old
  if(pdoc_ != NULL && !ref.empty()) {
    assert(pdoc_->count_id(ref));
    assert(pdoc_->get<ddNode>(ref)->ref_set.count(id));
  }

  // check new
  if(pdoc_ != NULL && !nrefId.empty() && ref != nrefId) {
    if(!pdoc_->count_id(nrefId)) { // ref node not found
      cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
        ": fail to find the node id \"" + nrefId + "\" in current Petri-Net document.";
      return false;
    } else {
      shared_ptr<ddObj> orig = pdoc_->get<ddObj>(nrefId);
      if(orig->type != type) {
        cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
          ": current node and the ref node have different node type (transition/place).";
        return false;
      }
    }
  }

  // set new ref
  if(nrefId != ref) {
    if(pdoc_ != NULL) {
      if(!ref.empty())     pdoc_->get<ddNode>(ref)->ref_set.erase(id);
      if(!nrefId.empty())  pdoc_->get<ddNode>(nrefId)->ref_set.insert(id);
    }
    ref = nrefId;
  }
  
  return true;
}


string cppPNML::details::ddNode::get_display_name() const {
  string rv = name;
  if(!ref.empty()) {
    ddObj& orig = *(pdoc_->id_map.find(ref)->second);
    rv += "(r:" + orig.id;                        // append the orignal node id to ref nodes
    if(!orig.name.empty()) rv += "," + orig.name; // append name to it if any
    rv += ")";
  }
  return rv;
}

