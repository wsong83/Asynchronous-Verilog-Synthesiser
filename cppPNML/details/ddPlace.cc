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
cppPNML::details::ddPlace::ddPlace(const string& i, const string& n)
  : ddNode(PN_Place, i, n), nToken(0), tokenOffset(PLACE_W,-PLACE_W)
{
  shape = pair<double,double>(PLACE_W,PLACE_W);
}

bool cppPNML::details::ddPlace::set_initial_mark(unsigned int nt) {
  nToken = nt;
  return true;
}

void cppPNML::details::ddPlace::write_dot(ostream& os) const {
  os << "[";
  os << "shape=circle, ";
  if(nToken != 0) os << "label=\"" << nToken << "\", ";
  else            os << "label=\"\", ";
  os << "xlabel=\"" << get_display_name() << "\"";
  os << "]";
}

void cppPNML::details::ddPlace::write_pnml(pugi::xml_node& xnode) const {
  xnode.set_name("place");
  xnode.append_attribute("id") = id.c_str();
  
  // place name
  if(!name.empty()) {
    xnode.append_child("name").append_child("text").text() = get_display_name().c_str();
  }

  // token
  if(nToken) {
    pugi::xml_node token = xnode.append_child("initialMarking");
    token.append_child("text").text() = boost::str(boost::format("%d") % nToken).c_str();
    pugi::xml_node graphics = token.append_child("graphics");
    pugi::xml_node offset = graphics.append_child("offset");
    offset.append_attribute("x") = boost::str(boost::format("%f") % tokenOffset.first).c_str();
    offset.append_attribute("y") = boost::str(boost::format("%f") % tokenOffset.second).c_str();
  }

  // other
  {
    pugi::xml_node graphics = xnode.append_child("graphics");
    pugi::xml_node position = graphics.append_child("position");
    position.append_attribute("x") = boost::str(boost::format("%f") % pos.first).c_str();
    position.append_attribute("y") = boost::str(boost::format("%f") % pos.second).c_str();
    pugi::xml_node dimension = graphics.append_child("dimension");
    dimension.append_attribute("x") = boost::str(boost::format("%f") % shape.first).c_str();
    dimension.append_attribute("y") = boost::str(boost::format("%f") % shape.second).c_str();
  }
}

bool cppPNML::details::ddPlace::read_pnml(const pugi::xml_node& xnode, ddObj *pf) {
  // find out the hash id
  if(0 == 1) {
    return true;
  }

  // get the name if any
  pugi::xml_node xname = xnode.child("name");
  if(xname)
    set_name(xname.child("text").text().as_string());

  // token
  pugi::xml_node token = xnode.child("initialMarking");
  if(token) {
    nToken = token.child("text").text().as_uint();
    pugi::xml_node graphics = xnode.child("graphics");
    if(graphics) {
      pugi::xml_node offset = graphics.child("offset");
      if(offset) {
        tokenOffset.first = offset.attribute("x").as_double();
        tokenOffset.second = offset.attribute("y").as_double();
      }
    }
  }

  // graphics
  pugi::xml_node graphics = xnode.child("graphics");
  if(graphics) {
    // position
    pugi::xml_node position = graphics.child("position");
    if(position) {
      pos.first = position.attribute("x").as_double();
      pos.second = position.attribute("y").as_double();
    }

    // shape
    pugi::xml_node dimension = graphics.child("dimension");
    if(dimension) {
      shape.first = dimension.attribute("x").as_double();
      shape.second = dimension.attribute("y").as_double();
    }
  }

  // add the Graph to the document
  if(!static_cast<ddGraph *>(pf)
     ->add(
           static_pointer_cast<ddPlace>
           (shared_from_this())
           ) 
     )return false;
  
  return true;
}

void cppPNML::details::ddPlace::write_ogdf(ogdf::Graph *pg, ogdf::GraphAttributes *pga,
                                           map<string, void *>& i2n) const {
  assert(pg != NULL && pga != NULL);
  ogdf::node pnode = pg->newNode();
  i2n[id] = pnode;
  pga->labelNode(pnode) = id.c_str();
  pga->width(pnode) = shape.first;
  pga->height(pnode) = shape.second;
  pga->x(pnode) = pos.first;
  pga->y(pnode) = pos.second;
  pga->shapeNode(pnode) = ogdf::GraphAttributes::oval;
}

void cppPNML::details::ddPlace::read_ogdf(void *p, ogdf::GraphAttributes *pga) {
  assert(p != NULL && pga != NULL);
  ogdf::node pnode = static_cast<ogdf::node>(p);
  
  // double check the id is matched
  if(id != pga->labelNode(pnode).cstr()) return;

  shape.first = pga->width(pnode);
  shape.second = pga->height(pnode);
  pos.first = pga->x(pnode);
  pos.second = pga->y(pnode);
}
