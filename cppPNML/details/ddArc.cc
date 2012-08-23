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
cppPNML::details::ddArc::ddArc(const string& i, const string& n, const string& s, const string& t, pnArc::pnArcT atype)
  : ddObj(PN_Arc, i, n), source(s), target(t), arc_type(atype), fBGL(false) {}

void cppPNML::details::ddArc::write_dot(ostream& os) const {
  os << "[";
  if(!name.empty()) {
    os << "label=\"" << name << "\"" <<     ", ";
    os << "decorate=true, ";
  }
  switch(arc_type) {
  case pnArc::Normal: os << "dir=\"forward\""; break;
  case pnArc::Read:   os << "dir=\"both\""; break;
  default:;
  }
  os << "]";
}

void cppPNML::details::ddArc::write_pnml(pugi::xml_node& xnode) const {
  xnode.append_attribute("id") = id.c_str();
  xnode.append_attribute("source") = source.c_str();
  xnode.append_attribute("target") = target.c_str();
  
  // place name
  if(!name.empty()) {
    xnode.append_child("name").append_child("text").text() = name.c_str();
  }

  // other
  if(!curve.empty()) {
    pugi::xml_node graphics = xnode.append_child("graphics");
    typedef pair<double, double> PType;
    BOOST_FOREACH(const PType& p, curve) {
      pugi::xml_node pos = graphics.append_child("position");
      pos.append_attribute("x") = p.first;
      pos.append_attribute("y") = p.second;
    }
  } 
}

bool cppPNML::details::ddArc::read_pnml(const pugi::xml_node& xnode, ddObj *pf) {
  // find out the hash id
  if(0 == 1) {
    return true;
  }

  // get the name if any
  pugi::xml_node xname = xnode.child("name");
  if(xname)
    set_name(xname.child("text").text().as_string());

  // graphics
  pugi::xml_node graphics = xnode.child("graphics");
  if(graphics) {
    curve.clear();
    for(pugi::xml_node pos = graphics.child("position");
        pos; pos = pos.next_sibling("position")) {
      curve.push_back(pair<double, double>
                      (pos.attribute("x").as_double(),
                       pos.attribute("y").as_double())
                      );
    }
  }

  // add the Graph to the document
  if(!static_cast<ddGraph *>(pf)
     ->add(
           static_pointer_cast<ddArc>
           (shared_from_this())
           ) 
     )return false;

  return true;
}

void cppPNML::details::ddArc::write_ogdf(ogdf::Graph *pg, ogdf::GraphAttributes *pga,
                                         map<string, void *>& i2n) const {
  assert(pg != NULL && pga != NULL);

  // get the source and target node pointers
  ogdf::node ps = static_cast<ogdf::node>(i2n[source]);
  ogdf::node pt = static_cast<ogdf::node>(i2n[target]);
  assert(ps != NULL && pt != NULL);

  ogdf::edge pedge = pg->newEdge(ps, pt);
  pga->labelEdge(pedge) = id.c_str();

  // arrow type
  switch(arc_type) {
  case pnArc::Normal: pga->arrowEdge(pedge) = ogdf::GraphAttributes::last; break;
  case pnArc::Read:   pga->arrowEdge(pedge) = ogdf::GraphAttributes::both; break;
  default:            pga->arrowEdge(pedge) = ogdf::GraphAttributes::last;
  }

  typedef pair<double,double> CPoint;
  BOOST_FOREACH(const CPoint& point, curve) {
    pga->bends(pedge).pushBack(ogdf::DPoint(point.first, point.second));
  }

}

void cppPNML::details::ddArc::read_ogdf(void *p, ogdf::GraphAttributes *pga) {
  assert(p != NULL && pga != NULL);
  ogdf::edge pedge = static_cast<ogdf::edge>(p);
  
  // double check the id is matched
  if(id != pga->labelEdge(pedge).cstr()) return;

  // read in bend points
  curve.clear();
  for(ogdf::ListConstIterator<ogdf::DPoint> b = pga->bends(pedge).begin(); b.valid(); ++b) {
    curve.push_back(pair<double,double>((*b).m_x, (*b).m_y));
  }
}

void cppPNML::details::ddArc::set_arc_type(pnArc::pnArcT atype) {
  arc_type = atype;
}

