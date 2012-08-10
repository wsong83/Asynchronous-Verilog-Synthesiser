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
 * 24/07/2012   Wei Song
 *
 *
 */

#include "cpp_pnml.hpp"
#include "details/definitions.hpp"

#include <iostream>
#include <fstream>
#include <boost/format.hpp>

using boost::shared_ptr;
using boost::static_pointer_cast;
using std::string;
using std::ostream;
using std::istream;
using std::ofstream;
using namespace cppPNML;
using namespace cppPNML::details;

namespace cppPNML {
  extern string cppPNML_errMsg;        // simple error report technique
}

///////////////////////////////////////////////////////////////////////////
// wrapper classes

// a macro to reduce the work of checking pointers
#ifndef WRAPPER_POINTER_CHECK
#define WRAPPER_POINTER_CHECK(obj_name, rvalue)      \
  if(p_ == NULL) {                                   \
    cppPNML_errMsg = string(__PRETTY_FUNCTION__) +   \
      ": " + (obj_name) +                            \
      "is not initialized yet.";                     \
    return (rvalue);                                 \
  }
#endif

namespace cppPNML {
  namespace {
    static unsigned int genIDBaseID = 0;
    string genID() {          // generate a machine ID for anonymous arc or even node
      return boost::str(boost::format("@%1%") % genIDBaseID++);
    }
  }
}

/*-------- pnObj -----------*/
cppPNML::pnObj::pnObj()
  : p_(NULL) {}

void cppPNML::pnObj::init(ddObj * obj) {
  p_ = obj;
}

bool cppPNML::pnObj::setName(const string& n) {
  assert(p_ != NULL);
  return p_->set_name(n);
}

string cppPNML::pnObj::getName() const {
  assert(p_ != NULL);
  return p_->get_name();
}

/*-------- pnNode -----------*/
cppPNML::pnNode::pnNode() 
  : p_(NULL) {}

void cppPNML::pnNode::init(ddNode * n) {
  p_ = n;
  pnObj::init(n);               // initialise the base class
}

bool cppPNML::pnNode::setRefNode(const string& ref_id) {
  WRAPPER_POINTER_CHECK("node", false);
  return p_->set_ref_node(ref_id);
}

string cppPNML::pnNode::getRefNode() const {
  WRAPPER_POINTER_CHECK("node", "Error!");
  return p_->get_ref_node();
}

/*-------- pnPlace -----------*/
cppPNML::pnPlace::pnPlace() {}

cppPNML::pnPlace::pnPlace(const string& id, const string& name_or_ref_id, bool is_ref) {
  if(is_ref) initRef (id, name_or_ref_id);
  else       init    (id, name_or_ref_id);
}

cppPNML::pnPlace::pnPlace(shared_ptr<ddPlace> p)
  : p_(p) 
{
  pnNode::init(p_.get());            // initialise base classes
}  

void cppPNML::pnPlace::init(const string& id, const string& name) {
  if(id.empty())
    p_.reset(new ddPlace(genID(), name));
  else
    p_.reset(new ddPlace(id, name)); // build a new place
  pnNode::init(p_.get());            // initialise base classes
}

void cppPNML::pnPlace::initRef(const string& id, const string& ref_id) {
  if(id.empty())
    p_.reset(new ddPlace(genID(), string()));
  else
    p_.reset(new ddPlace(id, string())); // build a new place
  pnNode::init(p_.get());                // initialise base classes
  p_->set_ref_node(ref_id);              // set the ref_id will set the name to the original place if any
}

bool cppPNML::pnPlace::setInitMarking(unsigned int nt, bool force) {
  WRAPPER_POINTER_CHECK("place", false);
  return p_->set_initial_mark(nt, force);
}

/*-------- pnTran -----------*/
cppPNML::pnTran::pnTran() {}

cppPNML::pnTran::pnTran(const string& id, const string& name_or_ref_id, bool is_ref) {
  if(is_ref) initRef (id, name_or_ref_id);
  else       init    (id, name_or_ref_id);
}

cppPNML::pnTran::pnTran(shared_ptr<ddTransition> p)
  : p_(p) 
{
  pnNode::init(p_.get());            // initialise base classes
}  

void cppPNML::pnTran::init(const string& id, const string& name) {
  if(id.empty())
    p_.reset(new ddTransition(genID(), name));
  else
    p_.reset(new ddTransition(id, name));   // build a new place
  pnNode::init(p_.get());            // initialise base classes
}

void cppPNML::pnTran::initRef(const string& id, const string& ref_id) {
  if(id.empty())
    p_.reset(new ddTransition(genID(), string()));
  else
    p_.reset(new ddTransition(id, string())); // build a new place
  pnNode::init(p_.get());                     // initialise base classes
  p_->set_ref_node(ref_id);                   // set the ref_id will set the name to the original transition if any
}


/*-------- pnArc -----------*/
cppPNML::pnArc::pnArc() {}

cppPNML::pnArc::pnArc(const string& id, const string& source,
                      const string& target, const string& name) {
  init(id, source, target, name);
}

cppPNML::pnArc::pnArc(shared_ptr<ddArc> p)
  : p_(p) 
{
  pnObj::init(p_.get());            // initialise base classes
}  

void cppPNML::pnArc::init(const string& id, const string& source,
                          const string& target, const string& name) {
  if(id.empty())
    p_.reset(new ddArc(genID(), name, source, target));
  else
    p_.reset(new ddArc(id, name, source, target));
  pnObj::init(p_.get());
}

/*-------- pnGraph -----------*/
cppPNML::pnGraph::pnGraph() {}

cppPNML::pnGraph::pnGraph(const string& id, const string& name) {
  init(id, name);
}

cppPNML::pnGraph::pnGraph(shared_ptr<ddGraph> p)
  : p_(p) 
{
  pnObj::init(p_.get());            // initialise base classes
}  

void cppPNML::pnGraph::init(const string& id, const string& name) {
  p_.reset(new ddGraph(id, name));
  pnObj::init(p_.get());
}

template<>
bool cppPNML::pnGraph::add<pnPlace> (const pnPlace& p) {
  WRAPPER_POINTER_CHECK("graph", false);
  return p_->add(p.get_());
}

template<>
bool cppPNML::pnGraph::add<pnTran> (const pnTran& t) {
  WRAPPER_POINTER_CHECK("graph", false);
  return p_->add(t.get_());
}

template<>
bool cppPNML::pnGraph::add<pnArc> (const pnArc& a) {
  WRAPPER_POINTER_CHECK("graph", false);
  return p_->add(a.get_());
}

template<>
bool cppPNML::pnGraph::add<pnGraph> (const pnGraph& g) {
  WRAPPER_POINTER_CHECK("graph", false);
  return p_->add(g.get_());
}

template<> 
pnPlace cppPNML::pnGraph::get<pnPlace>(const string& id){
  WRAPPER_POINTER_CHECK("graph", pnPlace());
  return pnPlace(p_->get<ddPlace>(id));
}

template<> 
pnTran cppPNML::pnGraph::get<pnTran>(const string& id) {
  WRAPPER_POINTER_CHECK("graph", pnTran());
  return pnTran(p_->get<ddTransition>(id));
}

template<> 
pnArc cppPNML::pnGraph::get<pnArc>(const string& id) {
  WRAPPER_POINTER_CHECK("graph", pnArc());
  return pnArc(p_->get<ddArc>(id));
}

template<> 
pnGraph cppPNML::pnGraph::get<pnGraph>(const string& id) {
  WRAPPER_POINTER_CHECK("graph", pnGraph());
  return pnGraph(p_->get<ddGraph>(id));
}

template<> 
const pnPlace cppPNML::pnGraph::get<pnPlace>(const string& id) const {
  WRAPPER_POINTER_CHECK("graph", pnPlace());
  return pnPlace(p_->get<ddPlace>(id));
}

template<> 
const pnTran cppPNML::pnGraph::get<pnTran>(const string& id) const {
  WRAPPER_POINTER_CHECK("graph", pnTran());
  return pnTran(p_->get<ddTransition>(id));
}

template<> 
const pnArc cppPNML::pnGraph::get<pnArc>(const string& id) const{
  WRAPPER_POINTER_CHECK("graph", pnArc());
  return pnArc(p_->get<ddArc>(id));
}

template<> 
const pnGraph cppPNML::pnGraph::get<pnGraph>(const string& id) const {
  WRAPPER_POINTER_CHECK("graph", pnGraph());
  return pnGraph(p_->get<ddGraph>(id));
}

bool cppPNML::pnGraph::isOneSafe() const {
  WRAPPER_POINTER_CHECK("graph", false);
  return p_->fOneSafe;
}

bool cppPNML::pnGraph::setOneSafe(bool b) {
  WRAPPER_POINTER_CHECK("graph", false);
  return p_->set_one_safe(b);
}

bool cppPNML::pnGraph::layout() {
  WRAPPER_POINTER_CHECK("graph", false);
  return p_->layout();
}  


/*-------- pnPetriNet -----------*/
cppPNML::pnPetriNet::pnPetriNet() {}

cppPNML::pnPetriNet::pnPetriNet(const string& id, const string& name) {
  init(id, name);
}

cppPNML::pnPetriNet::pnPetriNet(shared_ptr<ddPetriNet> p)
  : p_(p) 
{
  pnObj::init(p_.get());            // initialise base classes
}  

void cppPNML::pnPetriNet::init(const string& id, const string& name) {
  p_.reset(new ddPetriNet(id, name));
  pnObj::init(p_.get());
}

bool cppPNML::pnPetriNet::add(const pnGraph& g) {
  WRAPPER_POINTER_CHECK("petri-net", false);
  return p_->add(g.get_());
}

bool cppPNML::pnPetriNet::setPNMLType(const string& t) {
  WRAPPER_POINTER_CHECK("petri-net", false);
  if(t == "HLPNG")
    p_->pnml_type = ddPetriNet::PNML_HLPNG;
  else if(t == "SN")
    p_->pnml_type = ddPetriNet::PNML_SN;
  else if(t == "PT")
    p_->pnml_type = ddPetriNet::PNML_PT;
  else
    p_->pnml_type = ddPetriNet::PNML_Other; // error actually

  return true;                  // right now there is no check about compatiblity
}

string cppPNML::pnPetriNet::getPNMLType() const {
  WRAPPER_POINTER_CHECK("petri-net", "Error!");
  string rv;
  switch(p_->pnml_type) {
  case ddPetriNet::PNML_HLPNG:  rv = "HLPNG"; break;
  case ddPetriNet::PNML_SN:     rv = "SN";    break;
  case ddPetriNet::PNML_PT:     rv = "PT";    break;
  default:                      rv = "Other";
  }
  return rv;
}

/*-------- pnPetrNetDoc -----------*/
cppPNML::pnPetriNetDoc::pnPetriNetDoc()
  : p_(new ddPetriNetDoc()) {}

cppPNML::pnPetriNetDoc::pnPetriNetDoc(shared_ptr<ddPetriNetDoc> p)
  : p_(p) {}

bool cppPNML::pnPetriNetDoc::add(const pnPetriNet& g) {
  WRAPPER_POINTER_CHECK("petri-net document", false);
  return p_->add_petriNet(g.get_());
}


///////////////////////////////////////////////////////////////////////////
// global functions
bool cppPNML::writeDot(ostream& os, const pnGraph& g) {
  g.get_()->write_dot(os);
  return true;
}

bool cppPNML::writeDot(const string& fname, const pnGraph& g) {
  std::ofstream fhandler(fname.c_str());
  g.get_()->write_dot(fhandler);
  fhandler.close();
  return true;
}

bool cppPNML::writeGML(ostream& os, const pnGraph& g) {
  g.get_()->write_gml(os);
  return true;
}

bool cppPNML::writeGML(const string& fname, const pnGraph& g) {
  std::ofstream fhandler(fname.c_str());
  g.get_()->write_gml(fhandler);
  fhandler.close();
  return true;
}

bool cppPNML::writePNML(ostream& os, const pnPetriNetDoc& pn) {
  pn.get_()->write_pnml(os);
  return true;
}

bool cppPNML::writePNML(const string& fname, const pnPetriNetDoc& pn) {
  std::ofstream fhandler(fname.c_str());
  pn.get_()->write_pnml(fhandler);
  fhandler.close();
  return true;
}

#undef WRAPPER_POINTER_CHECK
