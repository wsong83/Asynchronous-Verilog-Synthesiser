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
cppPNML::details::ddGraph::ddGraph(const string& i, const string& n)
  : ddObj(PN_Graph, i, n), ppg_(NULL), 
    //fOneSafe(true),
    vn_(boost::get(boost::vertex_name, bg_)),
    en_(boost::get(boost::edge_name, bg_))
{}

bool cppPNML::details::ddGraph::check(shared_ptr<ddObj> obj) const {
  if(!obj) {    // object empty
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
  if(!obj->name.empty() && name_map.count(obj->name)) { // error name existed
    cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
      ": node \"" + obj->name + "\" is already existed in current graph.";
    return false;
  }
  switch(obj->type) {
  case PN_Place: {
    shared_ptr<ddPlace> p = static_pointer_cast<ddPlace>(obj);
    /*
    if(p->nToken > 0) {
      if(p->nToken > 1 && fOneSafe) { 
        cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
          ": fail to set initial marking to > 1 when it is a one-safe graph.";
        return false;
      }
    }
    */
  } // continue to check the common check of nodes
  case PN_Transition: {
    shared_ptr<ddNode> n = static_pointer_cast<ddNode>(obj);
    if(!n->ref.empty()) {
      if(!pdoc_->count_id(n->ref)) {
        cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
          ": reference original id \"" + n->ref + "\" not existed.";
        return false;
      } else if(n->type != pdoc_->get<ddObj>(n->ref)->type) {
        cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
          ": illegal reference node.";
        return false;
      }
    }     
    break;
  }
  case PN_Arc: {
    shared_ptr<ddArc> a = static_pointer_cast<ddArc>(obj);
    // check the source and the target
    if(!pdoc_->get<ddNode>(a->source)) { // source wrong
      cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
        ": illegal source object.";
      return false;
    }

    if(!pdoc_->get<ddNode>(a->target)) { // target wrong
      cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
        ": illegal target object.";
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
  if(!t->name.empty()) name_map[t->name] = t->id;
  t->pg_ = this;
  pdoc_->add_obj(t);

  // BGL
  t->vd_ = boost::add_vertex(bg_);
  t->fBGL = true;
  vn_[t->vd_] = t->id;
  node_map[t->id] = t->vd_;

  // reference
  if(!t->ref.empty())
    pdoc_->get<ddTransition>(t->ref)->ref_set.insert(t->id);

  return true;
}
 
bool cppPNML::details::ddGraph::add(shared_ptr<ddPlace> p) {
  if(!check(p)) return false;
  
  // put it into the document
  if(!p->name.empty()) name_map[p->name] = p->id;
  p->pg_ = this;
  pdoc_->add_obj(p);

  // BGL
  p->vd_ = boost::add_vertex(bg_);
  p->fBGL = true;
  vn_[p->vd_] = p->id;
  node_map[p->id] = p->vd_;

  // reference
  if(!p->ref.empty())
    pdoc_->get<ddPlace>(p->ref)->ref_set.insert(p->id);

  return true;
}

bool cppPNML::details::ddGraph::add(shared_ptr<ddArc> a) {
  if(!check(a)) return false;

  shared_ptr<ddNode> src = pdoc_->get<ddNode>(a->source);
  shared_ptr<ddNode> tar = pdoc_->get<ddNode>(a->target);
  
  // put it into the document
  if(!a->name.empty()) name_map[a->name] = a->id;
  a->pg_ = this;
  pdoc_->add_obj(a);

  bool added;
  boost::tie(a->ed_, added) = boost::add_edge(src->vd_, tar->vd_, bg_);

  if(!added) {                  // already existed in the graph
    cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
      ": edge is already existed in the graph.";
    return false;
  }    

  en_[a->ed_] = a->id;
  arc_map[a->id] = a->ed_;

  return true;
}
 
bool cppPNML::details::ddGraph::add(shared_ptr<ddGraph> g) {
  if(!check(g)) return false;
  
  // put it into the document
  if(!g->name.empty()) name_map[g->name] = g->id;
  page_set.insert(g->id);
  g->ppg_ = this;
  pdoc_->add_obj(g);
  return true;
}

unsigned int cppPNML::details::ddGraph::count_name(const string& n) const {
  return name_map.count(n);
}

shared_ptr<ddObj> cppPNML::details::ddGraph::operator() (const string& n) {
  if(name_map.count(n) && pdoc_ != NULL && pdoc_->count_id(name_map[n]))
    return pdoc_->id_map.find(name_map.find(n)->second)->second;
  else
    return shared_ptr<ddObj>();
}

shared_ptr<const ddObj> cppPNML::details::ddGraph::operator() (const string& n) const {
  if(name_map.count(n) && pdoc_ != NULL && pdoc_->count_id(name_map.find(n)->second))
    return pdoc_->id_map.find(name_map.find(n)->second)->second;
  else
    return shared_ptr<const ddObj>();
}

string cppPNML::details::ddGraph::get_id(const string& n) const {
  if(name_map.count(n))
    return name_map.find(n)->second;
  else
    return "";
}

string cppPNML::details::ddGraph::get_id(const vertex_descriptor& vd) const {
  return boost::get(vn_, vd);
}

string cppPNML::details::ddGraph::get_id(const edge_descriptor& ed) const {
  return boost::get(en_, ed);
}

/*
bool cppPNML::details::ddGraph::set_one_safe(bool b) {
  if(pdoc_ == NULL) {           // has not been added to a petri-net document
    fOneSafe = b;
    return true;
  } else if(!fOneSafe && b) {   // reset a multi-token net to a one-safe net
    // need check
    typedef pair<const string, vertex_descriptor> node_map_type;
    BOOST_FOREACH(const node_map_type& pid, node_map) {
      shared_ptr<ddPlace> p = pdoc_->get<ddPlace>(pid.first);
      if(p && p->nToken > 1) {
        cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
          ": fail to set the graph to one-safe as place \"" + p->id +"\" has more than on1 initial tokens.";
        return false;
      }
    }
  }
  return true;
}
*/

void cppPNML::details::ddGraph::write_dot(std::ostream& os) const {
  boost::write_graphviz(os, bg_, 
                        boost::bind(&ddGraph::vertexWriter, boost::ref(*this), _1, _2), // vertex writer
                        boost::bind(&ddGraph::edgeWriter, boost::ref(*this), _1, _2), // edge writer
                        boost::bind(&ddGraph::graphWriter, boost::ref(*this), _1), // graph writer
                        vn_);   // the vertex name map
  
}

void cppPNML::details::ddGraph::write_pnml(pugi::xml_node& xnode) const {
  xnode.append_attribute("id") = id.c_str();

  // page name
  if(!name.empty()) {
    xnode.append_child("name").append_child("text").text() = name.c_str();
  }

  // the internals of a page
  assert(pdoc_ != NULL);

  // vertices
  typedef pair<const string, vertex_descriptor> node_map_type;
  BOOST_FOREACH(const node_map_type& nid, node_map) {
    assert(pdoc_->count_id(nid.first)); 
    pugi::xml_node xvertex = xnode.append_child();
    pdoc_->get<ddNode>(nid.first)->write_pnml(xvertex);
  }

  // edges
  typedef pair<const string, edge_descriptor> arc_map_type;
  BOOST_FOREACH(const arc_map_type& aid, arc_map) {
    assert(pdoc_->count_id(aid.first)); 
    pugi::xml_node xedge = xnode.append_child("arc");      
    pdoc_->get<ddArc>(aid.first)->write_pnml(xedge);
  }

  // sub-pages
  BOOST_FOREACH(const string& i, page_set) {
    // write out all child pages
    // the page should alway be available as it is check when added
    assert(pdoc_->count_id(i));
    pugi::xml_node xpage = xnode.append_child("page");      
    pdoc_->get<ddGraph>(i)->write_pnml(xpage);
  }
}

bool cppPNML::details::ddGraph::read_pnml(const pugi::xml_node& xnode, ddObj *pf) {
  // find out the hash id
  if(0 == 1) {
    return true;
  }

  // get the name if any
  pugi::xml_node xname = xnode.child("name");
  if(xname)
    set_name(xname.child("text").text().as_string());

  // add the Graph to the document
  if(!static_cast<ddPetriNet *>(pf)
     ->add(
           static_pointer_cast<ddGraph>
           (shared_from_this())
           ) 
     )return false;
    
  // process all childs
  for(pugi::xml_node xplace = xnode.child("place"); xplace; xplace = xplace.next_sibling("place")) {
    string xid = xplace.attribute("id").as_string();
    
    if(xid.empty()) {
      cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
        ": the <place> node does not have an identifier.";
      return false;
    }
    
    if(pdoc_->count_id(xid)) {
      cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
        ": the <place> \"" + xid + "\" is already existed in the document.";
      return false;
    }
    
    // make a new one and read in data
    shared_ptr<ddPlace> nPlace(new ddPlace(xid, ""));
    if(!nPlace->read_pnml(xplace, this)) 
      return false;
  }

  for(pugi::xml_node xtran = xnode.child("transition"); xtran; xtran = xtran.next_sibling("transition")) {
    string xid = xtran.attribute("id").as_string();
    
    if(xid.empty()) {
      cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
        ": the <transition> node does not have an identifier.";
      return false;
    }
    
    if(pdoc_->count_id(xid)) {
      cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
        ": the <transition> \"" + xid + "\" is already existed in the document.";
      return false;
    }
    
    // make a new one and read in data
    shared_ptr<ddTransition> nTran(new ddTransition(xid, ""));
    if(!nTran->read_pnml(xtran, this)) 
      return false;
  }

  for(pugi::xml_node xarc = xnode.child("arc"); xarc; xarc = xarc.next_sibling("arc")) {
    string xid = xarc.attribute("id").as_string();
    
    if(xid.empty()) {
      cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
        ": the <arc> node does not have an identifier.";
      return false;
    }
    
    if(pdoc_->count_id(xid)) {
      cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
        ": the <arc> \"" + xid + "\" is already existed in the document.";
      return false;
    }

    // checl source and target
    string source = xarc.attribute("source").as_string();
    string target = xarc.attribute("target").as_string();
    
    if(!pdoc_->count_id(source)) {
      cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
        ": the source \"" + source + "\" of <arc> \"" + xid + "\" does not exist.";
      return false;
    }
    
    if(!pdoc_->count_id(target)) {
      cppPNML_errMsg = string(__PRETTY_FUNCTION__) + 
        ": the target \"" + target + "\" of <arc> \"" + xid + "\" does not exist.";
      return false;
    }

    // make a new one and read in data
    shared_ptr<ddArc> nArc(new ddArc(xid, "", source, target, pnArc::Normal));
    if(!nArc->read_pnml(xarc, this)) 
      return false;
  }

  for(pugi::xml_node xpage = xnode.child("page"); xpage; xpage = xpage.next_sibling("page")) {
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

bool cppPNML::details::ddGraph::layout() {

  ogdf::Graph g;
  ogdf::GraphAttributes ga;
  write_ogdf(&g, &ga);

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

  read_ogdf(&g, &ga);

  return true;
}

void cppPNML::details::ddGraph::vertexWriter(ostream& os, const vertex_descriptor& v) const {
  pdoc_->get<ddNode>(boost::get(vn_, v))->write_dot(os);
}

void cppPNML::details::ddGraph::edgeWriter(ostream& os, const edge_descriptor& e) const {
  pdoc_->get<ddArc>(boost::get(en_, e))->write_dot(os);
}

void cppPNML::details::ddGraph::graphWriter(ostream&) const {
  // nothing to do right now
  // keep as a future feature
}

void cppPNML::details::ddGraph::write_ogdf(ogdf::Graph *pg, ogdf::GraphAttributes *pga) const{
  pga->init(*pg,
            ogdf::GraphAttributes::nodeGraphics | ogdf::GraphAttributes::edgeGraphics |
            ogdf::GraphAttributes::nodeLabel    | ogdf::GraphAttributes::edgeStyle    | 
            ogdf::GraphAttributes::nodeStyle    | ogdf::GraphAttributes::edgeArrow    |
            ogdf::GraphAttributes::nodeWeight   | ogdf::GraphAttributes::edgeLabel    );
  map<string, void *> i2n;      // id to node map

  // vertices
  typedef pair<const string, vertex_descriptor> node_map_type;
  BOOST_FOREACH(const node_map_type& nid, node_map) {
    assert(pdoc_->count_id(nid.first)); 
    pdoc_->get<ddNode>(nid.first)->write_ogdf(pg, pga, i2n);
  }

  // edges
  typedef pair<const string, edge_descriptor> arc_map_type;
  BOOST_FOREACH(const arc_map_type& aid, arc_map) {
    assert(pdoc_->count_id(aid.first)); 
    pdoc_->get<ddArc>(aid.first)->write_ogdf(pg, pga, i2n);
  }
}

void cppPNML::details::ddGraph::read_ogdf(ogdf::Graph *pg, ogdf::GraphAttributes *pga) {
  // nodes
  {
    ogdf::node n;
    forall_nodes(n, *pg) {
      pdoc_->get<ddNode>(pga->labelNode(n).cstr())->read_ogdf(n, pga);
    }
  }

  {
    ogdf::edge e;
    forall_edges(e, *pg) {
      pdoc_->get<ddArc>(pga->labelEdge(e).cstr())->read_ogdf(e, pga);
    }
  }
}
