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
 * internal classes definitions
 * 24/07/2012   Wei Song
 *
 *
 */

// although it is header guarded, it should be used only in cpp_pnml.cpp

#ifndef CPPPNML_DEFINITIONS_H_
#define CPPPNML_DEFINITIONS_H_

#include <string>
#include <map>
#include <set>
#include <iostream>
#include <boost/enable_shared_from_this.hpp>

// the BGL library
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>

// pugixml
#include "pugixml/pugixml.hpp"

// forward declaration
namespace ogdf {
  class Graph;
  class GraphAttributes;
}

namespace cppPNML {

  namespace details {

    // Boost Graph Library related
    typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS,
                                  boost::property<boost::vertex_name_t, std::string>,
                                  boost::property<boost::edge_name_t, std::string> > 
      GType;
    typedef boost::graph_traits<GType> GraphTraits;
    typedef typename GraphTraits::edge_descriptor edge_descriptor;
    typedef typename GraphTraits::vertex_descriptor vertex_descriptor;
    typedef typename GraphTraits::vertex_iterator vertex_iterator;
    typedef typename GraphTraits::edge_iterator edge_iterator;

    // forward declaration
    class ddPetriNetDoc;
    class ddPetriNet;
    class ddGraph;

    class ddObj : public boost::enable_shared_from_this<ddObj> {
    public:
      // data
      enum pn_t {
        PN_Obj,
        PN_Node,
        PN_Place,
        PN_Transition,
        PN_Arc,
        PN_Graph,
        PN_PetriNet,
        PN_PetriNetDoc
      };
      
      ddObj(pn_t, const std::string&, const std::string&);

      // data
      pn_t type;
      ddPetriNetDoc * pdoc_;
      ddGraph * pg_;
      std::string id;
      std::string name;

      // helpers
      bool set_name(const std::string& n);
      virtual void write_dot(std::ostream&) const;
      virtual void write_pnml(pugi::xml_node&) const;
      virtual bool read_pnml(const pugi::xml_node&, ddObj *);
      virtual void write_ogdf(ogdf::Graph *, ogdf::GraphAttributes *, 
                              std::map<std::string, void *>& // id to node map
                              ) const;
      virtual void read_ogdf(void *, ogdf::GraphAttributes *);
    };

    class ddNode : public ddObj {
    public:
      ddNode(pn_t, const std::string&, const std::string&);

      // record all ref nodes
      std::set<std::string> ref_set;

      // BGL
      bool fBGL;                // true when this object is registered in a BGL graph 
      vertex_descriptor vd_;    // vertex descriptor in the GBL graph
      std::string ref;
      std::pair<double,double> pos; // graphic position
      std::pair<double,double> shape; // graphic shape

      // helpers
      virtual bool set_ref_node(const std::string& refId);
      virtual std::string get_ref_node() const { return ref; }
      std::string get_display_name() const; // the display name for dot and pnml
    };

    class ddPlace : public ddNode {
    public:
      ddPlace(const std::string&, const std::string&);

      // data
      unsigned int nToken;      // number of initial tokens
      std::pair<double,double> tokenOffset; // the graphic offset of the token
      
      // helpers
      bool set_initial_mark(unsigned int/*, bool*/);
      virtual void write_dot(std::ostream&) const;
      virtual void write_pnml(pugi::xml_node&) const;
      virtual bool read_pnml(const pugi::xml_node&, ddObj*);
      virtual void write_ogdf(ogdf::Graph *, ogdf::GraphAttributes *, 
                              std::map<std::string, void *>& // id to node map
                              ) const;
      virtual void read_ogdf(void *, ogdf::GraphAttributes *);
    };

    class ddTransition : public ddNode {
    public:
      ddTransition(const std::string&, const std::string&);

      // helpers
      virtual void write_dot(std::ostream&) const;
      virtual void write_pnml(pugi::xml_node&) const;
      virtual bool read_pnml(const pugi::xml_node&, ddObj*);
      virtual void write_ogdf(ogdf::Graph *, ogdf::GraphAttributes *, 
                              std::map<std::string, void *>& // id to node map
                              ) const;
      virtual void read_ogdf(void *, ogdf::GraphAttributes *);
    };

    class ddArc : public ddObj {
    public:
      ddArc(const std::string&, const std::string&, const std::string&, const std::string&, pnArc::pnArcT);
      
      // data
      std::string source;       // source id
      std::string target;       // target id
      pnArc::pnArcT arc_type;   // arc type

      // BGL
      bool fBGL;                // true when this object is registered in a BGL graph 
      edge_descriptor ed_;      // vertex descriptor in the GBL graph
      std::list<std::pair<double, double> > curve; // the curve of this arc

      // helper
      void set_arc_type(pnArc::pnArcT);

      // helpers
      virtual void write_dot(std::ostream&) const;
      virtual void write_pnml(pugi::xml_node&) const;
      virtual bool read_pnml(const pugi::xml_node&, ddObj*);
      virtual void write_ogdf(ogdf::Graph *, ogdf::GraphAttributes *, 
                              std::map<std::string, void *>& // id to node map
                              ) const;
      virtual void read_ogdf(void *, ogdf::GraphAttributes *);
    };

    class ddGraph : public ddObj {
    public:
      ddGraph(const std::string&, const std::string&);

      // remember father
      ddGraph * ppg_;
      ddPetriNet * ppn_;

      // associate petri-net name to an id (some times id is not meaningful)
      std::map<std::string, std::string> name_map;
      // remember all child pages using a name set
      std::set<std::string> page_set;
      // map to convert id to descriptor
      std::map<std::string, vertex_descriptor> node_map;
      std::map<std::string, edge_descriptor> arc_map;
      // flags
      //bool fOneSafe;            // true if it is a one safe petri-net (default true)

      // helpers
      bool add(boost::shared_ptr<ddTransition>);
      bool add(boost::shared_ptr<ddPlace>);
      bool add(boost::shared_ptr<ddArc>);
      bool add(boost::shared_ptr<ddGraph>);
      unsigned int count_name(const std::string&) const;
      boost::shared_ptr<ddObj> operator() (const std::string&);
      boost::shared_ptr<const ddObj> operator() (const std::string&) const;
      std::string get_id(const std::string&) const;
      std::string get_id(const vertex_descriptor&) const;
      std::string get_id(const edge_descriptor&) const;
      template<typename PT> 
      boost::shared_ptr<const PT> get(const std::string&) const;
      template<typename PT> 
      boost::shared_ptr<PT> get(const std::string&);
      //bool set_one_safe(bool);

      // formats and layout
      virtual void write_dot(std::ostream&) const;
      virtual void write_pnml(pugi::xml_node&) const;
      virtual bool read_pnml(const pugi::xml_node&, ddObj*);
      void write_ogdf(ogdf::Graph *, ogdf::GraphAttributes *) const;
      void read_ogdf(ogdf::Graph *, ogdf::GraphAttributes *);
      virtual bool layout();    // generate the position information

    private:
      bool check(boost::shared_ptr<ddObj>) const;       // check the general condition before adding an object
      void vertexWriter(std::ostream&, const vertex_descriptor&) const; // vertex property writer
      void edgeWriter(std::ostream&, const edge_descriptor&) const;     // edge property writer
      void graphWriter(std::ostream&) const;                            // graph property writer

      // the BGL graph
      GType bg_;
      boost::property_map<GType, boost::vertex_name_t>::type vn_;
      boost::property_map<GType, boost::edge_name_t>::type en_;
    };

    class ddPetriNet : public ddObj {
    public:
      // right now the PNML type are recorded to generate the right type description in the PNML file
      // the objects in the PNML file are not checked for type compatibility during construction
      // although this can be done in the future by checking it when adding an object to a higher model
      enum pnml_t {             // the type of this petri-net
        PNML_HLPNG,             // High-level Petri-Net graph
        PNML_SN,                // Symmetric Net
        PNML_PT,                // place and transition net, default
        PNML_Other              // unknown future extension
      };
      
      ddPetriNet(const std::string&, const std::string& name, pnml_t t = PNML_PT);

      // associate petri-net name to an id
      std::map<std::string, std::string> name_map;
      // remember all child pages using a name set
      std::set<std::string> page_set;      

      // data
      pnml_t pnml_type;

      // helpers
      unsigned int count_name(const std::string&) const;
      boost::shared_ptr<ddGraph> operator() (const std::string&);
      boost::shared_ptr<const ddGraph> operator() (const std::string&) const;
      bool add(boost::shared_ptr<ddGraph>);
      std::string get_id(const std::string&) const;
      
      // pnml
      virtual void write_pnml(pugi::xml_node&) const;
      virtual bool read_pnml(const pugi::xml_node&, ddObj*);
    };

    class ddPetriNetDoc : public ddObj {
    public:
      ddPetriNetDoc();

      // associate id to petri-net objects
      std::map<std::string, boost::shared_ptr<ddObj> > id_map;
      // associate petri-net name to an id (some times id is not meaningful)
      std::map<std::string, std::string> name_map;
      // remember all child Petri_net using a name set
      std::set<std::string> pn_set;

      // helpers
      bool add_petriNet(boost::shared_ptr<ddPetriNet>);
      bool add_obj(boost::shared_ptr<ddObj>);
      boost::shared_ptr<ddPetriNet> operator() (const std::string&);
      boost::shared_ptr<const ddPetriNet> operator() (const std::string&) const;
      unsigned int count_id(const std::string&) const;
      unsigned int count_name(const std::string&) const;
      std::string get_name(const std::string&) const;
      std::string get_id(const std::string&) const;
      template<typename PT> 
      boost::shared_ptr<const PT> get(const std::string&) const;
      template<typename PT> 
      boost::shared_ptr<PT> get(const std::string&);

      // pnml
      void write_pnml(std::ostream&) const;
      void write_pnml(pugi::xml_document&) const;
      bool read_pnml(std::istream&);
      bool read_pnml(const pugi::xml_document&);
    };

    template <> 
    boost::shared_ptr<const ddObj>        ddPetriNetDoc::get<ddObj>        (const std::string&) const;
    template <> 
    boost::shared_ptr<const ddNode>       ddPetriNetDoc::get<ddNode>       (const std::string&) const;
    template <> 
    boost::shared_ptr<const ddPlace>      ddPetriNetDoc::get<ddPlace>      (const std::string&) const;
    template <> 
    boost::shared_ptr<const ddTransition> ddPetriNetDoc::get<ddTransition> (const std::string&) const;
    template <> 
    boost::shared_ptr<const ddArc>        ddPetriNetDoc::get<ddArc>        (const std::string&) const;
    template <> 
    boost::shared_ptr<const ddGraph>      ddPetriNetDoc::get<ddGraph>      (const std::string&) const;
    template <> 
    boost::shared_ptr<const ddPetriNet>   ddPetriNetDoc::get<ddPetriNet>   (const std::string&) const;
    template <> 
    boost::shared_ptr<ddObj>        ddPetriNetDoc::get<ddObj>        (const std::string&);
    template <> 
    boost::shared_ptr<ddNode>       ddPetriNetDoc::get<ddNode>       (const std::string&);
    template <> 
    boost::shared_ptr<ddPlace>      ddPetriNetDoc::get<ddPlace>      (const std::string&);
    template <> 
    boost::shared_ptr<ddTransition> ddPetriNetDoc::get<ddTransition> (const std::string&);
    template <> 
    boost::shared_ptr<ddArc>        ddPetriNetDoc::get<ddArc>        (const std::string&);
    template <> 
    boost::shared_ptr<ddGraph>      ddPetriNetDoc::get<ddGraph>      (const std::string&);
    template <> 
    boost::shared_ptr<ddPetriNet>   ddPetriNetDoc::get<ddPetriNet>   (const std::string&);

    template<typename PT> 
    inline boost::shared_ptr<const PT> ddGraph::get(const std::string& m) const {
      if(pdoc_) return pdoc_->get<PT>(m);
      else      return boost::shared_ptr<const PT>();
    }

    template<typename PT> 
    inline boost::shared_ptr<PT> ddGraph::get(const std::string& m) {
      if(pdoc_) return pdoc_->get<PT>(m);
      else      return boost::shared_ptr<PT>();
    }
  
  }
}

#endif /* CPPPNML_DEFINITIONS_H_ */
