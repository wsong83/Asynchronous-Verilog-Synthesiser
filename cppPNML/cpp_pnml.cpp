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
using std::set;
using std::map;
using std::pair;
using std::list;
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
  return p_->name;
}

string cppPNML::pnObj::getID() const {
  assert(p_ != NULL);
  return p_->id;
}

bool cppPNML::pnObj::check_bgl() const {
  if(!p_) return false;
  if(!p_->pdoc_) return false;
  return true;
}

/*-------- pnNode -----------*/
cppPNML::pnNode::pnNode() 
  : p_(NULL) {}

cppPNML::pnNode::pnNode(shared_ptr<details::ddNode> p)
  : p_(p.get()) {
  // this is quite dangerous, pnNode should be used immediately
  // after being constructed in this way
  pnObj::init(p.get());
}

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

pair<double, double> cppPNML::pnNode::getPosition() const {
  if(!p_) return pair<double, double>(0,0);
  else return p_->pos;
}

pair<double, double> cppPNML::pnNode::getBBox() const {
  if(!p_) return pair<double, double>(0,0);
  else return p_->shape;
} 

pnNode& cppPNML::pnNode::setPosition(const pair<double, double>& pos) {
  if(p_) p_->pos = pos;
  return *this;
}

pnNode& cppPNML::pnNode::setBBox(const pair<double, double>& bbox) {
  if(p_) p_->shape = bbox;
  return *this;
}

bool cppPNML::pnNode::isPlace() const {
  return p_ && p_->type == ddObj::PN_Place;
}

bool cppPNML::pnNode::isTransition() const {
  return p_ && p_->type == ddObj::PN_Transition;
}

pnNode cppPNML::pnNode::next() {
  if(!check_bgl()) return pnNode();
  map<string, details::vertex_descriptor>::const_iterator it;
  it = p_->pg_->node_map.find(p_->id);
  if(++it != p_->pg_->node_map.end())
    return pnNode(p_->pdoc_->get<ddNode>(it->first));
  else
    return pnNode();
}

const pnNode cppPNML::pnNode::next() const{
  if(!check_bgl()) return pnNode();
  map<string, details::vertex_descriptor>::const_iterator it;
  it = p_->pg_->node_map.find(p_->id);
  if(++it != p_->pg_->node_map.end())
    return pnNode(p_->pdoc_->get<ddNode>(it->first));
  else
    return pnNode();
}

pnNode cppPNML::pnNode::pre() {
  if(!check_bgl()) return pnNode();
  map<string, details::vertex_descriptor>::const_iterator it;
  it = p_->pg_->node_map.find(p_->id);
  if(it != p_->pg_->node_map.begin())
    return pnNode(p_->pdoc_->get<ddNode>((--it)->first));
  else
    return pnNode();
}

const pnNode cppPNML::pnNode::pre() const{
  if(!check_bgl()) return pnNode();
  map<string, details::vertex_descriptor>::const_iterator it;
  it = p_->pg_->node_map.find(p_->id);
  if(it != p_->pg_->node_map.begin())
    return pnNode(p_->pdoc_->get<ddNode>((--it)->first));
  else
    return pnNode();
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

cppPNML::pnPlace::pnPlace(const pnNode& node) { // generate a Place node from a node if it is a place
  if(!node.check_bgl()) return;
  if(!node.isPlace()) return;
  p_ = node.get_()->pdoc_->get<ddPlace>(node.get_()->id);
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

bool cppPNML::pnPlace::setInitMarking(unsigned int nt/*, bool force*/) {
  WRAPPER_POINTER_CHECK("place", false);
  return p_->set_initial_mark(nt/*, force*/);
}

unsigned int cppPNML::pnPlace::getToken() const {
  if(!p_) return 0;
  else return p_->nToken;
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

cppPNML::pnTran::pnTran(const pnNode& node) {  // generate a pnTran from a pnNode if it is a transition
  if(!node.check_bgl()) return;
  if(!node.isTransition()) return;
  p_ = node.get_()->pdoc_->get<ddTransition>(node.get_()->id);
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
                      const string& target, const string& name,
                      pnArcT atype) {
  init(id, source, target, name, atype);
}

cppPNML::pnArc::pnArc(shared_ptr<ddArc> p)
  : p_(p) 
{
  pnObj::init(p_.get());            // initialise base classes
} 

void cppPNML::pnArc::init(const string& id, const string& source,
                          const string& target, const string& name,
                          pnArc::pnArcT atype) {
  if(id.empty())
    p_.reset(new ddArc(genID(), name, source, target, atype));
  else
    p_.reset(new ddArc(id, name, source, target, atype));
  pnObj::init(p_.get());
}

pnArc::pnArcT cppPNML::pnArc::getArcType() const {
  if(!p_) return Normal;
  else    return p_->arc_type;
}

void cppPNML::pnArc::setArcType(pnArc::pnArcT atype) {
  if(p_) p_->set_arc_type(atype);
}

list<pair<double, double> > cppPNML::pnArc::getBends() const {
  if(!p_) return list<pair<double, double> >();
  else return p_->curve;
}

pnNode cppPNML::pnArc::getSource() const {
  if(!check_bgl()) return pnNode();
  shared_ptr<ddNode> node = p_->pdoc_->get<ddNode>(p_->source);
  return pnNode(node);
}

pnNode cppPNML::pnArc::getTarget() const {
  if(!check_bgl()) return pnNode();
  shared_ptr<ddNode> node = p_->pdoc_->get<ddNode>(p_->target);
  return pnNode(node);
} 

pnArc cppPNML::pnArc::next() {
  if(!check_bgl()) return pnArc();
  map<string, details::edge_descriptor>::const_iterator it;
  it = p_->pg_->arc_map.find(p_->id);
  if(++it != p_->pg_->arc_map.end())
    return pnArc(p_->pdoc_->get<ddArc>(it->first));
  else
    return pnArc();
}

const pnArc cppPNML::pnArc::next() const {
  if(!check_bgl()) return pnArc();
  map<string, details::edge_descriptor>::const_iterator it;
  it = p_->pg_->arc_map.find(p_->id);
  if(++it != p_->pg_->arc_map.end())
    return pnArc(p_->pdoc_->get<ddArc>(it->first));
  else
    return pnArc();
}

pnArc cppPNML::pnArc::pre() {
  if(!check_bgl()) return pnArc();
  map<string, details::edge_descriptor>::const_iterator it;
  it = p_->pg_->arc_map.find(p_->id);
  if(it != p_->pg_->arc_map.begin())
    return pnArc(p_->pdoc_->get<ddArc>((--it)->first));
  else
    return pnArc();
}

const pnArc cppPNML::pnArc::pre() const {
  if(!check_bgl()) return pnArc();
  map<string, details::edge_descriptor>::const_iterator it;
  it = p_->pg_->arc_map.find(p_->id);
  if(it != p_->pg_->arc_map.begin())
    return pnArc(p_->pdoc_->get<ddArc>((--it)->first));
  else
    return pnArc();
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

template<> 
pnNode cppPNML::pnGraph::front<pnNode>() {
  if(!check_bgl()) return pnNode();
  map<string, details::vertex_descriptor>::const_iterator it;
  it = p_->node_map.begin();
  if(it != p_->node_map.end())
    return pnNode(p_->get<ddNode>(it->first));
  else
    return pnNode();
}

template<> 
const pnNode cppPNML::pnGraph::front<pnNode>() const{
  if(!check_bgl()) return pnNode();
  map<string, details::vertex_descriptor>::const_iterator it;
  it = p_->node_map.begin();
  if(it != p_->node_map.end())
    return pnNode(p_->get<ddNode>(it->first));
  else
    return pnNode();
}

template<> 
pnArc cppPNML::pnGraph::front<pnArc>() {
  if(!check_bgl()) return pnArc();
  map<string, details::edge_descriptor>::const_iterator it;
  it = p_->arc_map.begin();
  if(it != p_->arc_map.end())
    return pnArc(p_->get<ddArc>(it->first));
  else
    return pnArc();
}

template<> 
const pnArc cppPNML::pnGraph::front<pnArc>() const {
  if(!check_bgl()) return pnArc();
  map<string, details::edge_descriptor>::const_iterator it;
  it = p_->arc_map.begin();
  if(it != p_->arc_map.end())
    return pnArc(p_->get<ddArc>(it->first));
  else
    return pnArc();
}

template<> 
pnGraph cppPNML::pnGraph::front<pnGraph>() {
  if(!check_bgl()) return pnGraph();
  set<string>::const_iterator it = p_->page_set.begin();
  if(it != p_->page_set.end())
    return pnGraph(p_->get<ddGraph>(*it));
  else
    return pnGraph();
}

template<> 
const pnGraph cppPNML::pnGraph::front<pnGraph>() const {
  if(!check_bgl()) return pnGraph();
  set<string>::const_iterator it = p_->page_set.begin();
  if(it != p_->page_set.end())
    return pnGraph(p_->get<ddGraph>(*it));
  else
    return pnGraph();
}

template<> 
pnNode cppPNML::pnGraph::back<pnNode>() {
  if(!check_bgl()) return pnNode();
  map<string, details::vertex_descriptor>::const_reverse_iterator it;
  it = p_->node_map.rbegin();
  if(it != p_->node_map.rend())
    return pnNode(p_->get<ddNode>(it->first));
  else
    return pnNode();
}

template<> 
const pnNode cppPNML::pnGraph::back<pnNode>() const{
  if(!check_bgl()) return pnNode();
  map<string, details::vertex_descriptor>::const_reverse_iterator it;
  it = p_->node_map.rbegin();
  if(it != p_->node_map.rend())
    return pnNode(p_->get<ddNode>(it->first));
  else
    return pnNode();
}

template<> 
pnArc cppPNML::pnGraph::back<pnArc>() {
  if(!check_bgl()) return pnArc();
  map<string, details::edge_descriptor>::const_reverse_iterator it;
  it = p_->arc_map.rbegin();
  if(it != p_->arc_map.rend())
    return pnArc(p_->get<ddArc>(it->first));
  else
    return pnArc();
}

template<> 
const pnArc cppPNML::pnGraph::back<pnArc>() const {
  if(!check_bgl()) return pnArc();
  map<string, details::edge_descriptor>::const_reverse_iterator it;
  it = p_->arc_map.rbegin();
  if(it != p_->arc_map.rend())
    return pnArc(p_->get<ddArc>(it->first));
  else
    return pnArc();
}

template<> 
pnGraph cppPNML::pnGraph::back<pnGraph>() {
  if(!check_bgl()) return pnGraph();
  set<string>::const_reverse_iterator it = p_->page_set.rbegin();
  if(it != p_->page_set.rend())
    return pnGraph(p_->get<ddGraph>(*it));
  else
    return pnGraph();
}

template<> 
const pnGraph cppPNML::pnGraph::back<pnGraph>() const {
  if(!check_bgl()) return pnGraph();
  set<string>::const_reverse_iterator it = p_->page_set.rbegin();
  if(it != p_->page_set.rend())
    return pnGraph(p_->get<ddGraph>(*it));
  else
    return pnGraph();
}

template<> 
unsigned int cppPNML::pnGraph::size<pnNode>  (const pnNode&) const {
  if(!check_bgl()) return 0;
  return p_->node_map.size();
}

template<> 
unsigned int cppPNML::pnGraph::size<pnArc>   (const pnArc&) const {
  if(!check_bgl()) return 0;
  return p_->arc_map.size();
}

template<> 
unsigned int cppPNML::pnGraph::size<pnGraph> (const pnGraph&) const {
   if(!check_bgl()) return 0;
   return p_->page_set.size();
}

pnGraph cppPNML::pnGraph::next() {
  if(!check_bgl()) return pnGraph();
  set<string>::const_iterator it;
  if(p_->ppn_) {
    it = p_->ppn_->page_set.find(p_->id);
    if(++it != p_->ppn_->page_set.end())
      return pnGraph(p_->get<ddGraph>(*it));
    else 
      return pnGraph();
  } else {
    it = p_->ppg_->page_set.find(p_->id);
    if(++it != p_->ppg_->page_set.end())
      return pnGraph(p_->get<ddGraph>(*it));
    else 
      return pnGraph();
  }
}

const pnGraph cppPNML::pnGraph::next() const {
  if(!check_bgl()) return pnGraph();
  set<string>::const_iterator it;
  if(p_->ppn_) {
    it = p_->ppn_->page_set.find(p_->id);
    if(++it != p_->ppn_->page_set.end())
      return pnGraph(p_->get<ddGraph>(*it));
    else 
      return pnGraph();
  } else {
    it = p_->ppg_->page_set.find(p_->id);
    if(++it != p_->ppg_->page_set.end())
      return pnGraph(p_->get<ddGraph>(*it));
    else 
      return pnGraph();
  }
}  

pnGraph cppPNML::pnGraph::pre() {
  if(!check_bgl()) return pnGraph();
  set<string>::const_iterator it;
  if(p_->ppn_) {
    it = p_->ppn_->page_set.find(p_->id);
    if(it != p_->ppn_->page_set.begin())
      return pnGraph(p_->get<ddGraph>(*(--it)));
    else 
      return pnGraph();
  } else {
    it = p_->ppg_->page_set.find(p_->id);
    if(it != p_->ppg_->page_set.begin())
      return pnGraph(p_->get<ddGraph>(*(--it)));
    else 
      return pnGraph();
  }
}

const pnGraph cppPNML::pnGraph::pre() const{
  if(!check_bgl()) return pnGraph();
  set<string>::const_iterator it;
  if(p_->ppn_) {
    it = p_->ppn_->page_set.find(p_->id);
    if(it != p_->ppn_->page_set.begin())
      return pnGraph(p_->get<ddGraph>(*(--it)));
    else 
      return pnGraph();
  } else {
    it = p_->ppg_->page_set.find(p_->id);
    if(it != p_->ppg_->page_set.begin())
      return pnGraph(p_->get<ddGraph>(*(--it)));
    else 
      return pnGraph();
  }
}

/*
bool cppPNML::pnGraph::isOneSafe() const {
  WRAPPER_POINTER_CHECK("graph", false);
  return p_->fOneSafe;
}

bool cppPNML::pnGraph::setOneSafe(bool b) {
  WRAPPER_POINTER_CHECK("graph", false);
  return p_->set_one_safe(b);
}
*/

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

pnGraph cppPNML::pnPetriNet::front() {
  if(!check_page_set()) return pnGraph();
  set<string>::const_iterator it = p_->page_set.begin();
  if(it != p_->page_set.end())
    return pnGraph(p_->pdoc_->get<ddGraph>(*it));
  else
    return pnGraph();
}

const pnGraph cppPNML::pnPetriNet::front() const{
  if(!check_page_set()) return pnGraph();
  set<string>::const_iterator it = p_->page_set.begin();
  if(it != p_->page_set.end())
    return pnGraph(p_->pdoc_->get<ddGraph>(*it));
  else
    return pnGraph();
}

pnGraph cppPNML::pnPetriNet::back() {
  if(!check_page_set()) return pnGraph();
  set<string>::const_reverse_iterator it = p_->page_set.rbegin();
  if(it != p_->page_set.rend())
    return pnGraph(p_->pdoc_->get<ddGraph>(*it));
  else
    return pnGraph();
}

const pnGraph cppPNML::pnPetriNet::back() const{
  if(!check_page_set()) return pnGraph();
  set<string>::const_reverse_iterator it = p_->page_set.rbegin();
  if(it != p_->page_set.rend())
    return pnGraph(p_->pdoc_->get<ddGraph>(*it));
  else
    return pnGraph();
}

unsigned int cppPNML::pnPetriNet::size() const {
  if(!check_page_set()) return 0;
  return p_->page_set.size();
}

pnPetriNet cppPNML::pnPetriNet::next() {
  if(!p_) return pnPetriNet();
  if(!p_->pdoc_) return pnPetriNet();
  set<string>::const_iterator it = p_->pdoc_->pn_set.find(p_->id);
  if(++it != p_->pdoc_->pn_set.end())
    return pnPetriNet(p_->pdoc_->get<ddPetriNet>(*it));
  else
    return pnPetriNet();
}

const pnPetriNet cppPNML::pnPetriNet::next() const {
  if(!p_) return pnPetriNet();
  if(!p_->pdoc_) return pnPetriNet();
  set<string>::const_iterator it = p_->pdoc_->pn_set.find(p_->id);
  if(++it != p_->pdoc_->pn_set.end())
    return pnPetriNet(p_->pdoc_->get<ddPetriNet>(*it));
  else
    return pnPetriNet();
}

pnPetriNet cppPNML::pnPetriNet::pre() {
  if(!p_) return pnPetriNet();
  if(!p_->pdoc_) return pnPetriNet();
  set<string>::const_iterator it = p_->pdoc_->pn_set.find(p_->id);
  if(it != p_->pdoc_->pn_set.begin())
    return pnPetriNet(p_->pdoc_->get<ddPetriNet>(*(--it)));
  else
    return pnPetriNet();
}

const pnPetriNet cppPNML::pnPetriNet::pre() const {
  if(!p_) return pnPetriNet();
  if(!p_->pdoc_) return pnPetriNet();
  set<string>::const_iterator it = p_->pdoc_->pn_set.find(p_->id);
  if(it != p_->pdoc_->pn_set.begin())
    return pnPetriNet(p_->pdoc_->get<ddPetriNet>(*(--it)));
  else
    return pnPetriNet();
}

bool cppPNML::pnPetriNet::check_page_set() const {
  if(!p_) return false;
  if(!p_->pdoc_) return false;
  if(p_->page_set.empty()) return false;
  return true;
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

pnPetriNet cppPNML::pnPetriNetDoc::front() {
  if(!check_pn_set()) return pnPetriNet();
  set<string>::const_iterator it = p_->pn_set.begin();
  if(it != p_->pn_set.end()) 
    return pnPetriNet(p_->get<ddPetriNet>(*it));
  else
    return pnPetriNet();
}
  
const pnPetriNet cppPNML::pnPetriNetDoc::front() const{
  if(!check_pn_set()) return pnPetriNet();
  set<string>::const_iterator it = p_->pn_set.begin();
  if(it != p_->pn_set.end()) 
    return pnPetriNet(p_->get<ddPetriNet>(*it));
  else
    return pnPetriNet();
}

pnPetriNet cppPNML::pnPetriNetDoc::back() {
  if(!check_pn_set()) return pnPetriNet();
  set<string>::const_reverse_iterator it = p_->pn_set.rbegin();
  if(it != p_->pn_set.rend()) 
    return pnPetriNet(p_->get<ddPetriNet>(*it));
  else
    return pnPetriNet();
}

const pnPetriNet cppPNML::pnPetriNetDoc::back() const {
  if(!check_pn_set()) return pnPetriNet();
  set<string>::const_reverse_iterator it = p_->pn_set.rbegin();
  if(it != p_->pn_set.rend()) 
    return pnPetriNet(p_->get<ddPetriNet>(*it));
  else
    return pnPetriNet();
}

unsigned int cppPNML::pnPetriNetDoc::size() const {
  if(!check_pn_set()) return 0;
  return p_->pn_set.size();
}

bool cppPNML::pnPetriNetDoc::check_pn_set() const {
  if(!p_) return false;
  if(p_->pn_set.empty()) return false;
  return true;
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

// writeGML and writeSVG are implemented in details.definitions.cpp 
// as they require the OGDF library

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

bool cppPNML::readPNML(istream& os, pnPetriNetDoc& pn) {
  return pn.get_()->read_pnml(os);
}

bool cppPNML::readPNML(const string& fname, pnPetriNetDoc& pn) {
  std::ifstream fhandler(fname.c_str());
  bool rv = pn.get_()->read_pnml(fhandler);
  fhandler.close();
  return rv;
}

pnPetriNetDoc cppPNML::readPNML(const string& fname) {
  pnPetriNetDoc pndoc;
  std::ifstream fhandler(fname.c_str());
  pndoc.get_()->read_pnml(fhandler);
  fhandler.close();
  return pndoc;
}

#undef WRAPPER_POINTER_CHECK
