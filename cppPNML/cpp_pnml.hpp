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
 * 23/07/2012   Wei Song
 *
 *
 */

#ifndef CPPPNML_CPPPNML_
#define CPPPNML_CPPPNML_

#include <string>
#include <iostream>
#include <list>
#include <boost/shared_ptr.hpp>

namespace cppPNML {

  // helper classes (forward declaration to avoid the complexity)
  namespace details {
    class ddObj;
    class ddNode;
    class ddPlace;
    class ddTransition;
    class ddArc;
    class ddGraph;
    class ddPetriNet;
    class ddPetriNetDoc;
  }

  // base class for all Petri-Net objects
  class pnObj {
  public:
    pnObj();
    void init(details::ddObj *);

    // helpers
    virtual bool valid() const { return p_ != NULL; }
    bool setName(const std::string& n); // reset the name of this node
    std::string getName() const;
    details::ddObj * get_() { return p_; } // not to be used by user
    const details::ddObj * get_() const { return p_; } // not to be used by user
    
  private:
    details::ddObj * p_;  
  };
  
  class pnNode : public pnObj {
  public:
    pnNode();
    void init(details::ddNode *);
    
    //helpers
    virtual bool valid() const { return pnObj::valid() && p_ != NULL; }
    bool setRefNode(const std::string& ref_id);
    std::string getRefNode() const;
    
  private:
    details::ddNode * p_;
  };

  class pnPlace : public pnNode {
  public:
    pnPlace();
    pnPlace(const std::string& id, const std::string& name_or_ref_id = std::string(), bool is_ref = false);
    pnPlace(boost::shared_ptr<details::ddPlace> p);
    void init(const std::string& id, const std::string& name = std::string());
    void initRef(const std::string& id, const std::string& ref_id);
    
    // helpers
    // set the initial number of token
    /* it is illegal to set a token number to > 1 if it is a one-safe graph
       by setting force to true, the gragh will be converted to a multi-token graph
       otherwise change the graph type before set multi-token
    */
    bool setInitMarking(unsigned int, bool force = false); 
    // fast marking set, user need to make sure the correctness
    pnPlace& setInitMarkingF(unsigned int t, bool force = true) { 
      setInitMarking(t, force); return *this;
    }
    virtual bool valid() const { return pnNode::valid() && p_.use_count(); }

    // library methods (only for internal use)
    boost::shared_ptr<details::ddPlace> get_() { return p_; } // not to be used by user
    const boost::shared_ptr<details::ddPlace> get_() const { return p_; } // not to be used by user

  private:
    boost::shared_ptr<details::ddPlace> p_;
  };

  class pnTran : public pnNode {
  public:
    pnTran();
    pnTran(const std::string& id, const std::string& name_or_ref_id = std::string(), bool is_ref = false);
    pnTran(boost::shared_ptr<details::ddTransition> p);
    void init(const std::string& id, const std::string& name = std::string());
    void initRef(const std::string& id, const std::string& ref_id);

    // helpers
    virtual bool valid() const { return pnNode::valid() && p_.use_count(); }

    // library methods (only for internal use)
    boost::shared_ptr<details::ddTransition> get_() { return p_; } // not to be used by user
    const boost::shared_ptr<details::ddTransition> get_() const { return p_; } // not to be used by user

  private:
    boost::shared_ptr<details::ddTransition> p_;
  };


  class pnArc : public pnObj {
  public:
    pnArc();
    pnArc(const std::string& id, // id of this arc
          const std::string& source, // source id
          const std::string& target, // target id
          const std::string& name = std::string() // name of the arc (optional)
          );
    pnArc(boost::shared_ptr<details::ddArc> p);

    //helpers
    void init(const std::string& id, // id of this arc
              const std::string& source, // source id
              const std::string& target, // target id
              const std::string& name = std::string() // name of the arc (optional)
              );

    // helpers
    virtual bool valid() const { return pnObj::valid() && p_.use_count(); }

    // library methods (only for internal use)
    boost::shared_ptr<details::ddArc> get_() { return p_; } // not to be used by user
    const boost::shared_ptr<details::ddArc> get_() const { return p_; } // not to be used by user

  private:
    boost::shared_ptr<details::ddArc> p_;

  };

  class pnGraph : public pnObj {
  public:
    pnGraph();                  // graph should have a default constructor
    pnGraph(const std::string& id, const std::string& name = std::string());
    pnGraph(boost::shared_ptr<details::ddGraph> p);
    void init(const std::string& id, const std::string& name = std::string());

    //helpers
    virtual bool valid() const { return pnObj::valid() && p_.use_count(); }
    // add objects to the graph (place, trans, arc and graph)
    template<typename PT> bool add(const PT&);
    // fast add, user need to make sure the correctness
    template<typename PT> pnGraph& addF(const PT& m) { add(m); return *this;}
    // get object from the graph
    template<typename PT> const PT get (const std::string&) const;
    template<typename PT> PT get (const std::string&);

    bool isOneSafe() const;   // true if this gragh is set one-safe
    // change the one-safe type of this grapgh
    /* a grapgh is one-safe in default
       it is possible to reset it by pass a false to this method
       but a token check will be performed 
     */
    bool setOneSafe(bool);
    pnGraph& setOneSafeF(bool osf) { setOneSafe(osf); return *this;}
    bool layout();

    // library methods (only for internal use)
    boost::shared_ptr<details::ddGraph> get_() { return p_; } // not to be used by user
    const boost::shared_ptr<details::ddGraph> get_() const { return p_; } // not to be used by user

  private:
    boost::shared_ptr<details::ddGraph> p_;
  };

  template<> bool pnGraph::add<pnPlace> (const pnPlace&);
  template<> bool pnGraph::add<pnTran>  (const pnTran&);
  template<> bool pnGraph::add<pnArc>   (const pnArc&);
  template<> bool pnGraph::add<pnGraph> (const pnGraph&);
  template<> const pnPlace  pnGraph::get<pnPlace> (const std::string&) const;
  template<> const pnTran   pnGraph::get<pnTran>  (const std::string&) const;
  template<> const pnArc    pnGraph::get<pnArc>   (const std::string&) const;
  template<> const pnGraph  pnGraph::get<pnGraph> (const std::string&) const;
  template<> pnPlace  pnGraph::get<pnPlace> (const std::string&);
  template<> pnTran   pnGraph::get<pnTran>  (const std::string&);
  template<> pnArc    pnGraph::get<pnArc>   (const std::string&);
  template<> pnGraph  pnGraph::get<pnGraph> (const std::string&);


  typedef pnGraph pnPage;       // a non-hierarchical Graph is called a page in PNML

  class pnPetriNet: public pnObj {
  public:
    pnPetriNet();
    pnPetriNet(const std::string& id, const std::string& name = std::string());
    pnPetriNet(boost::shared_ptr<details::ddPetriNet> p);
    
    // helpers
    virtual bool valid() const { return pnObj::valid() && p_.use_count(); }
    void init(const std::string& id, const std::string& name = std::string());
    bool add(const pnGraph&);
    pnPetriNet& addF(const pnGraph& g) { add(g); return *this; }
    // "HLPNG" (High-Level PN Graph) (default), "SN" (Sym Net) or "PT" (Place/Transition)
    bool setPNMLType(const std::string&); // if check is implemented, return false when the check failed
    pnPetriNet& setPNMLTypeF(const std::string& t) { setPNMLType(t); return *this; }
    std::string getPNMLType() const;

    // library methods (only for internal use)
    boost::shared_ptr<details::ddPetriNet> get_() { return p_; } // not to be used by user
    const boost::shared_ptr<details::ddPetriNet> get_() const { return p_; } // not to be used by user

  private:
    boost::shared_ptr<details::ddPetriNet> p_;
  };
  
  class pnPetriNetDoc {
  public:
    pnPetriNetDoc();
    pnPetriNetDoc(boost::shared_ptr<details::ddPetriNetDoc> p);

    // helpers
    virtual bool valid() const { return p_.use_count(); }
    bool add(const pnPetriNet&);
    pnPetriNetDoc& addF(const pnPetriNet& p) { add(p); return *this;}

    // library methods (only for internal use)
    boost::shared_ptr<details::ddPetriNetDoc> get_() { return p_; } // not to be used by user
    const boost::shared_ptr<details::ddPetriNetDoc> get_() const { return p_; } // not to be used by user
   
  private:
    boost::shared_ptr<details::ddPetriNetDoc> p_;
  };

  // global functions
  bool writeDot(const std::string& fname, const pnGraph& g);           // write out a graph to a dot file 
  bool writeDot(std::ostream& os, const pnGraph& g);                   // write out a graph to a dot file
  bool writeGML(const std::string& fname, const pnGraph& g);           // write out the whole petri-net to a GML file
  bool writeGML(std::ostream& os, const pnGraph& g);                   // write out the whole petri-net to a GML file
  bool writePNML(const std::string& fname, const pnPetriNetDoc& pn);   // write out the whole petri-net to a PNML file
  bool writePNML(std::ostream& os, const pnPetriNetDoc& pn);           // write out the whole petri-net to a PNML file
  bool readPNML(const std::string& fname, pnPetriNetDoc& pn);          // read in a PNML file
  bool readPNML(std::istream& in, pnPetriNetDoc& pn);                  // read in a PNML file


  // global data
  extern std::string cppPNML_errMsg;

}

#endif // CPPPNML_CPPPNML_

// Local Variables:
// mode: c++
// End:
