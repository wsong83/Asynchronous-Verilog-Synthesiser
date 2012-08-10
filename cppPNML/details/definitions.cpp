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
 * 06/08/2012   Wei Song
 *
 *
 */

#include "cpp_pnml.hpp"
#include "definitions.hpp"

#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <sstream>
#include <fstream>
//#include <cmath>

// OGDF
#include <ogdf/layered/SugiyamaLayout.h>
#include <ogdf/layered/FastHierarchyLayout.h>
#include <ogdf/layered/LongestPathRanking.h>
//#include <ogdf/layered/OptimalRanking.h>
//#include <ogdf/layered/CoffmanGrahamRanking.h>
#include <ogdf/layered/GreedyCycleRemoval.h>
//#include <ogdf/layered/SplitHeuristic.h>
//#include <ogdf/layered/MedianHeuristic.h>
//#include <ogdf/layered/GreedyInsertHeuristic.h>

using boost::shared_ptr;
using boost::static_pointer_cast;
using std::map;
using std::pair;
using std::list;
using std::string;
using std::ostream;
using std::istream;
using namespace cppPNML;
using namespace cppPNML::details;

static const double PLACE_W = 15.0;
static const double TRANSITION_H = 17.6;
static const double TRANSITION_H_EMPTY = 5.0;
static const double TRANSITION_W = 40.0;
static const double FONT_RATIO = 3.6;
static const double LAYER_DIST = 0.8;
static const double NODE_DIST = 1.5;

namespace cppPNML {
  string cppPNML_errMsg;        // simple error report technique
}

///////////////////////////////////////////////////////////////////////////
// details

/*-------- ddObj -------------*/
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
      if(!name.empty()) pg_->name_map->erase(name);
      if(!n.empty()) (*(pg_->name_map))[n] = id;
    }
    name = n;
  }

  return true;
}

void cppPNML::details::ddObj::write_dot(ostream&) const {
  // the write_dot of base class should not be called
  assert(0 == "Error: the write_dot() of the base class is called.");
}

void cppPNML::details::ddObj::write_pnml(ostream&, unsigned int) const {
  // the write_pnml of base class should not be called
  assert(0 == "Error: the write_pnml() of the base class is called.");
}

void cppPNML::details::ddObj::write_gml(ostream&, unsigned int) const {
  // the write_gml of base class should not be called
  assert(0 == "Error: the write_gml() of the base class is called.");
}

/*-------- ddNode -----------*/
cppPNML::details::ddNode::ddNode(pn_t t, const string& i, const string& n)
  : ddObj(t, i, n), ref_set(new std::set<string>()), fBGL(false), pos(0,0), shape(0,0) {}

bool cppPNML::details::ddNode::set_ref_node(const string& nrefId) {
  // check old
  if(pdoc_ != NULL && !ref.empty()) {
    assert(pdoc_->count_id(ref));
    assert(static_pointer_cast<ddNode>(pdoc_->get_obj(ref))->ref_set->count(id));
  }

  // check new
  if(pdoc_ != NULL && !nrefId.empty() && ref != nrefId) {
    if(!pdoc_->count_id(nrefId)) { // ref node not found
      cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
        ": fail to find the node id \"" + nrefId + "\" in current Petri-Net document.";
      return false;
    } else {
      shared_ptr<ddObj> orig = pdoc_->get_obj(nrefId);
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
      if(!ref.empty()) {
        shared_ptr<ddNode> orig = static_pointer_cast<ddNode>(pdoc_->get_obj(ref));
        orig->ref_set->erase(id);
      }
      if(!nrefId.empty()) {
        shared_ptr<ddNode> orig = static_pointer_cast<ddNode>(pdoc_->get_obj(nrefId));
        orig->ref_set->insert(id);
      }
    }
    ref = nrefId;
  }
  
  return true;
}


string cppPNML::details::ddNode::get_display_name() const {
  string rv = name;
  if(!ref.empty()) {
    ddObj& orig = *(pdoc_->id_map->find(ref)->second);
    rv += "(r:" + orig.id;                        // append the orignal node id to ref nodes
    if(!orig.name.empty()) rv += "," + orig.name; // append name to it if any
    rv += ")";
  }
  return rv;
}

/*-------- ddPlace -----------*/
cppPNML::details::ddPlace::ddPlace(const string& i, const string& n)
  : ddNode(PN_Place, i, n), nToken(0) 
{
  shape = pair<double,double>(PLACE_W,PLACE_W);
}

bool cppPNML::details::ddPlace::set_initial_mark(unsigned int nt, bool force) {
  if(pg_ == NULL) {             // this place has not been added to a graph yet
    nToken = nt;
    return true;
  } else {
    if(nt > 1 && pg_->fOneSafe) { // multi-token in a one safe net
      if(force) {                 // force it by change it to multi-token
        pg_->fOneSafe = false;
        nToken = nt;
        return true;
      } else {                  // error
        cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
          ": fail to set multiple initial token to a place in a one-safe net.";
        return false;
      }
    } else {
      nToken = nt;
      return true;
    }
  }
}

void cppPNML::details::ddPlace::write_dot(ostream& os) const {
  os << "[";
  if(pg_->fOneSafe) {
    if(nToken == 0) 
         os << "shape=circle, ";
    else os << "shape=doublecircle, style=\"filled\", fillcolor=blue, ";

    if(get_display_name().size() < 3 && nToken == 0)
      os << "label=\"" << get_display_name() << "\"";
    else
      os << "label=\"\", xlabel=\"" << get_display_name() << "\"";
  } else {                      // multi-token
    os << "shape=circle, ";
    if(nToken != 0) os << "label=\"" << nToken << "\", ";
    else            os << "label=\"\", ";
    os << "xlabel=\"" << get_display_name() << "\"";
  }
  os << "]";
}

void cppPNML::details::ddPlace::write_pnml(ostream& os, unsigned int indent) const {
  os << string(indent, '\t') << "<place id=\"" << id << "\">\n";
  
  // place name
  if(!name.empty()) {
    os <<  string(indent+1, '\t') << "<name>\n";
    os <<  string(indent+2, '\t') << "<text>" << get_display_name() << "</text>\n";
    os <<  string(indent+1, '\t') << "</name>\n";
  }

  // other

  os << string(indent, '\t') << "</place>\n";
}

void cppPNML::details::ddPlace::write_gml(ostream& os, unsigned int indent) const {
  os << string(indent, '\t') << "node [\n"; indent++;
  os << string(indent, '\t') << "id " << vd_ << "\n";
  os << string(indent, '\t') << "label \"" << get_display_name() << "\"\n";
  os << string(indent, '\t') << "graphics [\n"; indent++;
  os << string(indent, '\t') << boost::format("x %1$.5f\n") % pos.first;
  os << string(indent, '\t') << boost::format("y %1$.5f\n") % pos.second;
  os << string(indent, '\t') << boost::format("w %1$.5f\n") % shape.first;
  os << string(indent, '\t') << boost::format("h %1$.5f\n") % shape.second;
  os << string(indent, '\t') << "type \"oval\"\n";
  indent--;
  os << string(indent, '\t') << "]\n";
  indent--;
  os << string(indent, '\t') << "]\n"; 
}

/*-------- ddTransition -----------*/
cppPNML::details::ddTransition::ddTransition(const string& i, const string& n)
  : ddNode(PN_Transition, i, n)
{
  shape = pair<double,double>(TRANSITION_W,TRANSITION_H_EMPTY);
  if(!n.empty()) {
    shape.first = TRANSITION_H * n.size() / FONT_RATIO;
    shape.second = TRANSITION_H;
  }
}

void cppPNML::details::ddTransition::write_dot(ostream& os) const {
  os << "[";
  os << "label=\"" << get_display_name() << "\"" <<     ", ";
  os << "shape=box";
  os << "]";
}

void cppPNML::details::ddTransition::write_pnml(ostream& os, unsigned int indent) const {
  os << string(indent, '\t') << "<transition id=\"" << id << "\">\n";
  
  // place name
  if(!name.empty()) {
    os <<  string(indent+1, '\t') << "<name>\n";
    os <<  string(indent+2, '\t') << "<text>" << get_display_name() << "</text>\n";
    os <<  string(indent+1, '\t') << "</name>\n";
  }

  // other

  os << string(indent, '\t') << "</transition>\n";

}

void cppPNML::details::ddTransition::write_gml(ostream& os, unsigned int indent) const {
  os << string(indent, '\t') << "node [\n"; indent++;
  os << string(indent, '\t') << "id " << vd_ << "\n";
  os << string(indent, '\t') << "label \"" << get_display_name() << "\"\n";
  os << string(indent, '\t') << "graphics [\n"; indent++;
  os << string(indent, '\t') << boost::format("x %1$.5f\n") % pos.first;
  os << string(indent, '\t') << boost::format("y %1$.5f\n") % pos.second;
  os << string(indent, '\t') << boost::format("w %1$.5f\n") % shape.first;
  os << string(indent, '\t') << boost::format("h %1$.5f\n") % shape.second;
  os << string(indent, '\t') << "type \"rectangle\"\n";
  indent--;
  os << string(indent, '\t') << "]\n"; 
  indent--;
  os << string(indent, '\t') << "]\n"; 
}

/*-------- ddArc -----------*/
cppPNML::details::ddArc::ddArc(const string& i, const string& n, const string& s, const string& t)
  : ddObj(PN_Arc, i, n), source(s), target(t), arc_type(ARC_Normal), fBGL(false) {}

void cppPNML::details::ddArc::write_dot(ostream& os) const {
  os << "[";
  if(!name.empty()) {
    os << "label=\"" << name << "\"" <<     ", ";
    os << "decorate=true, ";
  }
  switch(arc_type) {
  case ARC_Normal: os << "dir=\"forward\""; break;
  case ARC_Read:   os << "dir=\"both\""; break;
  default:;
  }
  os << "]";
}

void cppPNML::details::ddArc::write_pnml(ostream& os, unsigned int indent) const {
  os << string(indent, '\t') << "<arc id=\"" << id << "\" source=\"" << source <<"\" target=\"" << target << "\">\n";
  
  // place name
  if(!name.empty()) {
    os <<  string(indent+1, '\t') << "<name>\n";
    os <<  string(indent+2, '\t') << "<text>" << name << "</text>\n";
    os <<  string(indent+1, '\t') << "</name>\n";
  }

  // other

  os << string(indent, '\t') << "</arc>\n";


}

void cppPNML::details::ddArc::write_gml(ostream& os, unsigned int indent) const {
  os << string(indent, '\t') << "edge [\n"; indent++;
  os << string(indent, '\t') << "label \"" << name << "\"\n";
  os << string(indent, '\t') << "source " << static_pointer_cast<ddNode>(pdoc_->get_obj(source))->vd_ << "\n";
  os << string(indent, '\t') << "target " << static_pointer_cast<ddNode>(pdoc_->get_obj(target))->vd_ << "\n";
  os << string(indent, '\t') << "graphics [\n"; indent++;
  os << string(indent, '\t') << "type \"line\"\n";
  os << string(indent, '\t') << "arrow \"last\"\n"; 
  os << string(indent, '\t') << "Line [\n"; indent++; // why it is Line instead of line?
  typedef pair<double, double> point_t;
  BOOST_FOREACH(const point_t& m, curve) {
    os << string(indent, '\t') << boost::format("point [ x %1$.5f y %2$.5f ]\n") % m.first % m.second;
  }
  indent--;
  os << string(indent, '\t') << "]\n"; 
  indent--;
  os << string(indent, '\t') << "]\n"; 
  indent--;
  os << string(indent, '\t') << "]\n";      
}

void cppPNML::details::ddArc::set_arc_type(arc_t atype) {
  arc_type = atype;
}

ddArc::arc_t cppPNML::details::ddArc::get_arc_type() const {
  return arc_type;
}

/*-------- ddGraph -----------*/
cppPNML::details::ddGraph::ddGraph(const string& i, const string& n)
  : ddObj(PN_Graph, i, n), ppg_(NULL), 
    name_map(new map<string, string>()),
    page_set(new std::set<string>()),
    fOneSafe(true),
    bg_(new GType()),
    vn_(boost::get(boost::vertex_name, *bg_)),
    en_(boost::get(boost::edge_name, *bg_))
{}

bool cppPNML::details::ddGraph::check(shared_ptr<ddObj> obj) const {
  if(obj.use_count() == 0) {    // object empty
    cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
      ": object is empty.";
    return false;
  }
  if(pdoc_ == NULL) {           // error, doc not assigned
    cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
      ": link the Graph to a Petri-Net document before adding an object.";
    return false;
  }
  if(ppg_ == NULL && ppn_ == NULL) { // no parent 
    cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
      ": link the Graph to a parent Graph or a Petri-Net before adding an object.";
    return false;
  }
  if(pdoc_->count_id(obj->id)) {   // error, id existed
    cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
      ": The id \"" + obj->id + "\" is already existed in current Petri-Net document.";
    return false;
  } 
  if(!obj->name.empty() && name_map->count(obj->name)) { // error name existed
    cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
      ": node \"" + obj->name + "\" is already existed in current graph.";
    return false;
  }
  switch(obj->type) {
  case PN_Place: {
    shared_ptr<ddPlace> p = static_pointer_cast<ddPlace>(obj);
    if(p->nToken > 0) {
      if(p->nToken > 1 && fOneSafe) { 
        /* it is illegal to set the initial number of token of a place to > 1
           when the grapgh is a one-safe graph.
        */
        cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
          ": fail to set initial marking to > 1 when it is a one-safe graph.";
        return false;
      }
    }
  } // continue to check the common check of nodes
  case PN_Transition: {
    shared_ptr<ddNode> n = static_pointer_cast<ddNode>(obj);
    if(!n->ref.empty()) {
      if(!pdoc_->count_id(n->ref)) {
        cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
          ": reference original id \"" + n->ref + "\" not existed.";
        return false;
      } else if(n->type != pdoc_->get_obj(n->ref)->type) {
        cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
          ": reference node and current node have different node types.";
        return false;
      }
    }     
    break;
  }
  case PN_Arc: {
    shared_ptr<ddArc> a = static_pointer_cast<ddArc>(obj);
    // check the source and the target
    if(!pdoc_->count_id(a->source)) {
      cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
        ": source id \"" + a->source + "\" not existed.";
      return false;
    }
    if(!pdoc_->count_id(a->target)) {
      cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
        ": target id \"" + a->target + "\" not existed.";
      return false;
    }
    shared_ptr<ddObj> src = pdoc_->get_obj(a->source);
    if(src.use_count() == 0 || 
       (src->type != PN_Transition && src->type != PN_Place)) { // source type wrong
      cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
        ": illegal source object type.";
      return false;
    }
    shared_ptr<ddObj> tar = pdoc_->get_obj(a->target);
    if(tar.use_count() == 0 || 
       (tar->type != PN_Transition && tar->type != PN_Place)) { // target type wrong
      cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
        ": illegal target object type.";
      return false;
    }
    break;
  }
  case PN_Graph:
    break;
  default:
    cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
      ": illegal object type";
    return false;
  }
  return true;
}

bool cppPNML::details::ddGraph::add(shared_ptr<ddTransition> t) {
  if(!check(t)) return false;
  
  // put it into the document
  if(!t->name.empty()) (*name_map)[t->name] = t->id;
  t->pg_ = this;
  pdoc_->add_obj(t);

  // BGL
  t->vd_ = boost::add_vertex(*bg_);
  t->fBGL = true;
  vn_[t->vd_] = t->id;

  // reference
  if(!t->ref.empty())
    static_pointer_cast<ddNode>(pdoc_->get_obj(t->ref))->ref_set->insert(t->id);

  return true;
}
 
bool cppPNML::details::ddGraph::add(shared_ptr<ddPlace> p) {
  if(!check(p)) return false;
  
  // put it into the document
  if(!p->name.empty()) (*name_map)[p->name] = p->id;
  p->pg_ = this;
  pdoc_->add_obj(p);

  // BGL
  p->vd_ = boost::add_vertex(*bg_);
  p->fBGL = true;
  vn_[p->vd_] = p->id;

  // reference
  if(!p->ref.empty())
    static_pointer_cast<ddNode>(pdoc_->get_obj(p->ref))->ref_set->insert(p->id);

  return true;
}

bool cppPNML::details::ddGraph::add(shared_ptr<ddArc> a) {
  if(!check(a)) return false;

  shared_ptr<ddNode> src = static_pointer_cast<ddNode>(pdoc_->get_obj(a->source));
  shared_ptr<ddNode> tar = static_pointer_cast<ddNode>(pdoc_->get_obj(a->target));
  
  // put it into the document
  if(!a->name.empty()) (*name_map)[a->name] = a->id;
  a->pg_ = this;
  pdoc_->add_obj(a);

  bool added;
  boost::tie(a->ed_, added) = boost::add_edge(src->vd_, tar->vd_, *bg_);

  if(!added) {                  // already existed in the graph
    cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
      ": edge is already existed in the graph.";
    return false;
  }    

  en_[a->ed_] = a->id;

  return true;
}
 
bool cppPNML::details::ddGraph::add(shared_ptr<ddGraph> g) {
  if(!check(g)) return false;
  
  // put it into the document
  if(!g->name.empty()) (*name_map)[g->name] = g->id;
  page_set->insert(g->id);
  g->ppg_ = this;
  pdoc_->add_obj(g);
  return true;
}

unsigned int cppPNML::details::ddGraph::count_name(const string& n) const {
  return name_map->count(n);
}

shared_ptr<ddObj> cppPNML::details::ddGraph::operator() (const string& n) {
  if(name_map->count(n) && pdoc_ != NULL && pdoc_->count_id((*name_map)[n]))
    return pdoc_->id_map->find(name_map->find(n)->second)->second;
  else
    return shared_ptr<ddObj>();
}

shared_ptr<const ddObj> cppPNML::details::ddGraph::operator() (const string& n) const {
  if(name_map->count(n) && pdoc_ != NULL && pdoc_->count_id(name_map->find(n)->second))
    return pdoc_->id_map->find(name_map->find(n)->second)->second;
  else
    return shared_ptr<const ddObj>();
}

string cppPNML::details::ddGraph::get_id(const string& n) const {
  if(name_map->count(n))
    return name_map->find(n)->second;
  else
    return "";
}

template<>
shared_ptr<const ddObj> cppPNML::details::ddGraph::get<ddObj> (const string& id) const{
  if(pdoc_ != NULL) return pdoc_->get_obj(id);
  else              return shared_ptr<const ddObj>(); 
}

template<>
shared_ptr<ddObj> cppPNML::details::ddGraph::get<ddObj> (const string& id){
  if(pdoc_ != NULL) return pdoc_->get_obj(id);
  else              return shared_ptr<ddObj>(); 
}

template<>
shared_ptr<const ddNode> cppPNML::details::ddGraph::get<ddNode> (const string& id) const{
  shared_ptr<const ddObj> obj = get<ddObj>(id);
  if(obj.use_count() && (obj->type == PN_Place || obj->type == PN_Transition)) 
    return static_pointer_cast<const ddNode>(obj);
  else
    return shared_ptr<const ddNode>();
}

template<>
shared_ptr<const ddPlace> cppPNML::details::ddGraph::get<ddPlace> (const string& id) const{
  shared_ptr<const ddObj> obj = get<ddObj>(id);
  if(obj.use_count() && obj->type == PN_Place) 
    return static_pointer_cast<const ddPlace>(obj);
  else
    return shared_ptr<const ddPlace>();
}

template<>
shared_ptr<const ddTransition> cppPNML::details::ddGraph::get<ddTransition> (const string& id) const{
  shared_ptr<const ddObj> obj = get<ddObj>(id);
  if(obj.use_count() && obj->type == PN_Transition) 
    return static_pointer_cast<const ddTransition>(obj);
  else 
    return shared_ptr<const ddTransition>();
}

template<>
shared_ptr<const ddArc> cppPNML::details::ddGraph::get<ddArc> (const string& id) const{
  shared_ptr<const ddObj> obj = get<ddObj>(id);
  if(obj.use_count() && obj->type == PN_Arc) 
    return static_pointer_cast<const ddArc>(obj);
  else
    return shared_ptr<const ddArc>();
}

template<>
shared_ptr<const ddGraph> cppPNML::details::ddGraph::get<ddGraph> (const string& id) const{
  shared_ptr<const ddObj> obj = get<ddObj>(id);
  if(obj.use_count() && obj->type == PN_Graph) 
    return static_pointer_cast<const ddGraph>(obj);
  else 
    return shared_ptr<const ddGraph>();
}

template<>
shared_ptr<ddNode> cppPNML::details::ddGraph::get<ddNode> (const string& id){
  shared_ptr<ddObj> obj = get<ddObj>(id);
  if(obj.use_count() && (obj->type == PN_Place || obj->type == PN_Transition)) 
    return static_pointer_cast<ddNode>(obj);
  else
    return shared_ptr<ddNode>();
}

template<>
shared_ptr<ddPlace> cppPNML::details::ddGraph::get<ddPlace> (const string& id){
  shared_ptr<ddObj> obj = get<ddObj>(id);
  if(obj.use_count() && obj->type == PN_Place) 
    return static_pointer_cast<ddPlace>(obj);
  else
    return shared_ptr<ddPlace>();
}

template<>
shared_ptr<ddTransition> cppPNML::details::ddGraph::get<ddTransition> (const string& id){
  shared_ptr<ddObj> obj = get<ddObj>(id);
  if(obj.use_count() && obj->type == PN_Transition) 
    return static_pointer_cast<ddTransition>(obj);
  else 
    return shared_ptr<ddTransition>();
}

template<>
shared_ptr<ddArc> cppPNML::details::ddGraph::get<ddArc> (const string& id) {
  shared_ptr<ddObj> obj = get<ddObj>(id);
  if(obj.use_count() && obj->type == PN_Arc) 
    return static_pointer_cast<ddArc>(obj);
  else
    return shared_ptr<ddArc>();
}

template<>
shared_ptr<ddGraph> cppPNML::details::ddGraph::get<ddGraph> (const string& id){
  shared_ptr<ddObj> obj = get<ddObj>(id);
  if(obj.use_count() && obj->type == PN_Graph) 
    return static_pointer_cast<ddGraph>(obj);
  else 
    return shared_ptr<ddGraph>();
}

bool cppPNML::details::ddGraph::set_one_safe(bool b) {
  if(pdoc_ == NULL) {           // has not been added to a petri-net document
    fOneSafe = b;
    return true;
  } else if(!fOneSafe && b) {   // reset a multi-token net to a one-safe net
    // need check
    typename GraphTraits::vertex_iterator i, end;
    for(boost::tie(i,end) = boost::vertices(*bg_); i!=end; ++i) {
      string vid = boost::get(vn_, *i);
      assert(pdoc_->count_id(vid)); 
      shared_ptr<ddObj> obj = pdoc_->get_obj(vid);
      if(obj->type == PN_Place) { // a place node
        shared_ptr<ddPlace> p = static_pointer_cast<ddPlace>(obj);
        if(p->nToken > 1) {
          cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
            ": fail to set the graph to one-safe as place \"" + p->id +"\" has more than on1 initial tokens.";
          return false;
        }
      }
    }
  }
  return true;
}

void cppPNML::details::ddGraph::write_dot(std::ostream& os) const {
  boost::write_graphviz(os, *bg_, 
                        boost::bind(&ddGraph::vertexWriter, boost::ref(*this), _1, _2), // vertex writer
                        boost::bind(&ddGraph::edgeWriter, boost::ref(*this), _1, _2), // edge writer
                        boost::bind(&ddGraph::graphWriter, boost::ref(*this), _1), // graph writer
                        vn_);   // the vertex name map
  
}

void cppPNML::details::ddGraph::write_pnml(ostream& os, unsigned int indent) const {
  os << string(indent, '\t') << "<page id=\"" << id << "\">\n";

  // page name
  if(!name.empty()) {
    os <<  string(indent+1, '\t') << "<name>\n";
    os <<  string(indent+2, '\t') << "<text>" << name << "</text>\n";
    os <<  string(indent+1, '\t') << "</name>\n";
  }

  // the internals of a page
  assert(pdoc_ != NULL);

  // vertices
  {
    typename GraphTraits::vertex_iterator i, end;
    for(boost::tie(i,end) = boost::vertices(*bg_); i!=end; ++i) {
      string vid = boost::get(vn_, *i);
      assert(pdoc_->count_id(vid)); 
      static_pointer_cast<ddNode>(pdoc_->get_obj(vid))->write_pnml(os, indent + 1);
    }
  }

  // edges
  {
    typename GraphTraits::edge_iterator i, end;
    for(boost::tie(i,end) = boost::edges(*bg_); i!=end; ++i) {
      string vid = boost::get(en_, *i);
      assert(pdoc_->count_id(vid)); 
      static_pointer_cast<ddArc>(pdoc_->get_obj(vid))->write_pnml(os, indent + 1);
    }
  }

  // sub-pages
  {
    BOOST_FOREACH(const string& i, *page_set) {
      // write out all child pages
      // the page should alway be available as it is check when added
      assert(pdoc_->count_id(i)); 
      static_pointer_cast<ddGraph>(pdoc_->get_obj(i))->write_pnml(os, indent + 1);
    }
  }

  os << string(indent, '\t') << "</page>" << std::endl;
}

void cppPNML::details::ddGraph::write_gml(ostream& os, unsigned int indent) const{
  os << string(indent, '\t') << "Creator \"c++/PNML\"\n";
  os << string(indent, '\t') << "graph [\n";
  os << string(indent+1, '\t') << "directed 1\n";
  // vertices
  {
    typename GraphTraits::vertex_iterator i, end;
    for(boost::tie(i,end) = boost::vertices(*bg_); i!=end; ++i) {
      string vid = boost::get(vn_, *i);
      assert(pdoc_->count_id(vid)); 
      static_pointer_cast<ddNode>(pdoc_->get_obj(vid))->write_gml(os, indent + 1);
    }
  }

  // edges
  {
    typename GraphTraits::edge_iterator i, end;
    for(boost::tie(i,end) = boost::edges(*bg_); i!=end; ++i) {
      string vid = boost::get(en_, *i);
      assert(pdoc_->count_id(vid)); 
      static_pointer_cast<ddArc>(pdoc_->get_obj(vid))->write_gml(os, indent + 1);
    }
  }
  os << string(indent, '\t') << "]" << std::endl;
}

bool cppPNML::details::ddGraph::layout() {
  std::stringstream os;
  write_gml(os);

  ogdf::Graph g;
  ogdf::GraphAttributes ga 
    (g,
     ogdf::GraphAttributes::nodeGraphics | ogdf::GraphAttributes::edgeGraphics |
     ogdf::GraphAttributes::nodeLabel    | ogdf::GraphAttributes::edgeStyle    | 
     ogdf::GraphAttributes::nodeStyle    | ogdf::GraphAttributes::edgeArrow    );
    
  if(!ga.readGML(g, os)) {
    cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
            ": fail to read the internal gml format!";
    return false;
  }

  // Sugiyama Layout
  ogdf::SugiyamaLayout SL;

  ogdf::LongestPathRanking *ranking = new ogdf::LongestPathRanking();
  //ogdf::OptimalRanking * ranking = new ogdf::OptimalRanking();
  //ogdf::CoffmanGrahamRanking *ranking = new ogdf::CoffmanGrahamRanking();
  //ranking->width(4);
  ogdf::GreedyCycleRemoval * subGrapher = new ogdf::GreedyCycleRemoval();
  ranking->setSubgraph(subGrapher);
  SL.setRanking(ranking);

  //ogdf::SplitHeuristic *crossMiner = new ogdf::SplitHeuristic();
  //ogdf::MedianHeuristic *crossMiner = new ogdf::MedianHeuristic();
  //ogdf::GreedyInsertHeuristic *crossMiner = new ogdf::GreedyInsertHeuristic();
  //SL.setCrossMin(crossMiner);

  ogdf::FastHierarchyLayout * layouter = new ogdf::FastHierarchyLayout();
  layouter->layerDistance(TRANSITION_H * LAYER_DIST);
  layouter->nodeDistance(TRANSITION_H * NODE_DIST);
  SL.setLayout(layouter);

  SL.runs(30);

  try {
    SL.call(ga);
  } catch(std::exception e) {
    cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
            ": layout engine SugiyamaLayout failed.";
    return false;
  }

  //ga.writeGML(std::cout);

  // save the graphic information to the BGL graph
  {
    ogdf::node n;
    forall_nodes(n, g) {
      shared_ptr<ddNode> pnode = get<ddNode>(boost::get(vn_, n->index()));
      assert(pnode.use_count());
      pnode->pos =  pair<double, double>(ga.x(n), ga.y(n));
    }
  }
  
  {
    ogdf::edge e;
    forall_edges(e, g) {
      edge_descriptor ed;
      bool finded;
      boost::tie(ed, finded) = boost::edge(e->source()->index(), e->target()->index(), *bg_);
      assert(finded);
      shared_ptr<ddArc> parc = get<ddArc>(boost::get(en_, ed));
      assert(parc.use_count());
      parc->curve.clear();
      for(ogdf::ListConstIterator<ogdf::DPoint> b = ga.bends(e).begin(); b.valid(); ++b) {
        parc->curve.push_back(pair<double, double>((*b).m_x, (*b).m_y));
      }
    }
  }
  return true;
}

void cppPNML::details::ddGraph::vertexWriter(ostream& os, const vertex_descriptor& v) const {
  static_pointer_cast<ddNode>(pdoc_->get_obj(boost::get(vn_, v)))->write_dot(os);
}

void cppPNML::details::ddGraph::edgeWriter(ostream& os, const edge_descriptor& e) const {
  static_pointer_cast<ddArc>(pdoc_->get_obj(boost::get(en_, e)))->write_dot(os);
}

void cppPNML::details::ddGraph::graphWriter(ostream&) const {
  // nothing to do right now
  // keep as a future feature
}

/*-------- ddPetriNet -----------*/
cppPNML::details::ddPetriNet::ddPetriNet(const string& i, const string& n, pnml_t t)
  : ddObj(PN_PetriNet, i, n),
    name_map(new map<string, string>()),
    page_set(new std::set<string>()),
    pnml_type(t) {}

unsigned int cppPNML::details::ddPetriNet::count_name(const string& n) const {
  return name_map->count(n);
}

shared_ptr<ddGraph> cppPNML::details::ddPetriNet::operator() (const string& n) {
  if(name_map->count(n) && pdoc_ != NULL && pdoc_->count_id((*name_map)[n]))
    return static_pointer_cast<ddGraph>(pdoc_->id_map->find(name_map->find(n)->second)->second);
  else
    return shared_ptr<ddGraph>();
}

shared_ptr<const ddGraph> cppPNML::details::ddPetriNet::operator() (const string& n) const {
  if(name_map->count(n) && pdoc_ != NULL && pdoc_->count_id(name_map->find(n)->second))
    return static_pointer_cast<ddGraph>(pdoc_->id_map->find(name_map->find(n)->second)->second);
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
  if(!g->name.empty() && name_map->count(g->name)) { // error name existed
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
  if(!g->name.empty()) (*name_map)[g->name] = g->id;
  page_set->insert(g->id);
  g->ppn_ = this;
  pdoc_->add_obj(g);
  return true;
}

string cppPNML::details::ddPetriNet::get_id(const string& n) const {
  if(name_map->count(n))
    return name_map->find(n)->second;
  else
    return "";
}

void cppPNML::details::ddPetriNet::write_pnml(ostream& os, unsigned int indent) const {
  os << string(indent, '\t') << "<net type=\"http://www.pnml.org/version-2009/grammar/";
  switch(pnml_type) {
  case PNML_HLPNG: os << "highlevelnet"; break;
  case PNML_SN:    os << "symmetricnet"; break;
  case PNML_PT:    os << "ptnet";        break;
  default:         os << "highlevelnet"; // I believe high-level Petri-Net is the most compatible net format
  }
  os << "\" id=\"" << id << "\">\n";

  // net name
  if(!name.empty()) {
    os <<  string(indent+1, '\t') << "<name>\n";
    os <<  string(indent+2, '\t') << "<text>" << name << "</text>\n";
    os <<  string(indent+1, '\t') << "</name>\n";
  }

  BOOST_FOREACH(const string& i, *page_set) {
    // write out all child pages
    // the page should alway be available as it is check when added
    assert(pdoc_ != NULL);
    assert(pdoc_->id_map->find(i) != pdoc_->id_map->end()); 
    static_pointer_cast<ddGraph>(pdoc_->id_map->find(i)->second)->write_pnml(os, indent + 1);
  }

  os << string(indent, '\t') << "</net>" << std::endl;
}

/*-------- ddPetriNetDoc -----------*/
cppPNML::details::ddPetriNetDoc::ddPetriNetDoc() 
  : ddObj(PN_PetriNetDoc, "", ""),
    id_map(new map<string, shared_ptr<ddObj> >()), 
    name_map(new map<string, string>()),
    pn_set(new std::set<string>()) {}

bool cppPNML::details::ddPetriNetDoc::add_petriNet(shared_ptr<ddPetriNet> pn) {
  assert(pn.use_count() != 0);
  if(id_map->count(pn->id)) {   // error, id existed
    cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
      ": The id \"" + pn->id + "\" of Petri-Net \"" + 
      pn->name + "\" is already existed in current Petri-Net document.";
    return false;
  }
  if(!pn->name.empty() && name_map->count(pn->name)) { // error name existed
    cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
      ": Petri-Net \"" + pn->name + 
      "\" is already existed in current Petri-Net document.";
    return false;
  }
  
  // put it into the document
  if(!pn->name.empty()) (*name_map)[pn->name] = pn->id;
  pn_set->insert(pn->id);
  (*id_map)[pn->id] = pn;
  pn->pdoc_ = this;
  return true;
}

bool cppPNML::details::ddPetriNetDoc::add_obj(shared_ptr<ddObj> obj) {
  assert(obj.use_count() != 0);
  if(id_map->count(obj->id)) { // error, id existed
    cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
      ": The id \"" + obj->id + "\" of Petri-Net object \"" + obj->name + 
      "\" is already existed in current Petri-Net document.";
    return false;
  }

  (*id_map)[obj->id] = obj;
  obj->pdoc_ = this;
  return true;
}

shared_ptr<ddPetriNet> cppPNML::details::ddPetriNetDoc::operator() (const string& n) {
  if(name_map->count(n) && count_id((*name_map)[n]))
    return static_pointer_cast<ddPetriNet>(id_map->find(name_map->find(n)->second)->second);
  else
    return shared_ptr<ddPetriNet>();
}

shared_ptr<const ddPetriNet> cppPNML::details::ddPetriNetDoc::operator() (const string& n) const {
  if(name_map->count(n) && count_id(name_map->find(n)->second))
    return static_pointer_cast<ddPetriNet>(id_map->find(name_map->find(n)->second)->second);
  else
    return shared_ptr<const ddPetriNet>();
}

unsigned int cppPNML::details::ddPetriNetDoc::count_id(const string& i) const {
  return id_map->count(i);
}

unsigned int cppPNML::details::ddPetriNetDoc::count_name(const string& n) const {
  return name_map->count(n);
}

string cppPNML::details::ddPetriNetDoc::get_name(const string& i) const {
  if(id_map->count(i))
    return id_map->find(i)->second->name;
  else 
    return "";
}

string cppPNML::details::ddPetriNetDoc::get_id(const string& n) const {
  if(name_map->count(n))
    return name_map->find(n)->second;
  else
    return "";
}

shared_ptr<ddObj> cppPNML::details::ddPetriNetDoc::get_obj(const string& i) {
  if(id_map->count(i)) 
    return id_map->find(i)->second;
  else
    return shared_ptr<ddObj>();
}

shared_ptr<const ddObj> cppPNML::details::ddPetriNetDoc::get_obj(const string& i) const{
  if(id_map->count(i)) 
    return id_map->find(i)->second;
  else
    return shared_ptr<const ddObj>();
}

void cppPNML::details::ddPetriNetDoc::write_pnml(ostream& os, unsigned int indent) const {
  os << string(indent, '\t') << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  os << string(indent, '\t') << "<pnml xmlns=\"http://www.pnml.org/version-2009/grammar/pnml\">\n";

  BOOST_FOREACH(const string& i, *pn_set) {
    // write out all child petri-net
    // the petri-net should alway be available as it is check when added
    assert(id_map->find(i) != id_map->end()); 
    static_pointer_cast<ddPetriNet>(id_map->find(i)->second)->write_pnml(os, indent + 1);
  }

  os << string(indent, '\t') << "</pnml>" << std::endl;
}

