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
    std::string getID() const;

    // library functions, do not call directly
    details::ddObj * get_() { return p_; } // not to be used by user
    const details::ddObj * get_() const { return p_; } // not to be used by user
    bool check_bgl() const;
    
  private:
    details::ddObj * p_;  
  };
  
  class pnNode : public pnObj {
  public:
    pnNode();
    pnNode(boost::shared_ptr<details::ddNode>); // not safe, use carefully (shared_ptr is referenced to naked pointer)
    void init(details::ddNode *);
    
    //helpers
    virtual bool valid() const { return pnObj::valid() && p_ != NULL; }
    bool setRefNode(const std::string& ref_id);
    std::string getRefNode() const;
    std::pair<double, double> getPosition() const; // get the (x,y) position
    std::pair<double, double> getBBox() const; // get the bounding box (w,h) size
    pnNode& setPosition(const std::pair<double, double>&);
    pnNode& setBBox(const std::pair<double, double>&); 
    bool isPlace() const;
    bool isTransition() const;
    pnNode next();
    const pnNode next() const;
    pnNode pre();
    const pnNode pre() const;

    // library methods, do not call
    details::ddNode * get_() { return p_; } // not to be used by users
    const details::ddNode * get_() const { return p_; } // not to be used by users
    
  private:
    details::ddNode * p_;
  };

  class pnPlace : public pnNode {
  public:
    pnPlace();
    pnPlace(const std::string& id, const std::string& name_or_ref_id = std::string(), bool is_ref = false);
    pnPlace(boost::shared_ptr<details::ddPlace> p);
    pnPlace(const pnNode&);
    void init(const std::string& id, const std::string& name = std::string());
    void initRef(const std::string& id, const std::string& ref_id);
    
    // helpers
    // set the initial number of token
    /* it is illegal to set a token number to > 1 if it is a one-safe graph
       by setting force to true, the gragh will be converted to a multi-token graph
       otherwise change the graph type before set multi-token
    */
    // overlook multi-token problem
    // it is not an important feature in PNML and I am not going to enforce it 
    bool setInitMarking(unsigned int/*, bool force = false*/); 
    // fast marking set, user need to make sure the correctness
    pnPlace& setInitMarkingF(unsigned int t/*, bool force = true*/) { 
      setInitMarking(t/*, force*/); return *this;
    }
    virtual bool valid() const { return pnNode::valid() && p_.use_count(); }
    unsigned int getToken() const;

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
    pnTran(const pnNode&);
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
    enum pnArcT {
      Normal,                   // normal unidirectional arc
      Read,                     // read arc
      Inhibitor,                // 
      Reset                     // 
    };

    pnArc();
    pnArc(const std::string& id, // id of this arc
          const std::string& source, // source id
          const std::string& target, // target id
          const std::string& name = std::string(), // name of the arc (optional)
          const pnArcT atype = Normal
          );
    pnArc(boost::shared_ptr<details::ddArc> p);

    //helpers
    void init(const std::string& id, // id of this arc
              const std::string& source, // source id
              const std::string& target, // target id
              const std::string& name = std::string(), // name of the arc (optional)
              const pnArcT atype = Normal
              );

    // helpers
    virtual bool valid() const { return pnObj::valid() && p_; }
    pnArcT getArcType() const;
    void setArcType(pnArcT);
    std::list<std::pair<double, double> > getBends() const; // get the bend points of this arc
    pnNode getSource() const;
    pnNode getTarget() const;
    pnArc next();
    const pnArc next() const;
    pnArc pre();
    const pnArc pre() const;

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
    template<typename PT> PT front();
    template<typename PT> const PT front() const;
    template<typename PT> PT back();
    template<typename PT> const PT back() const;
    template<typename PT> unsigned int size(const PT& dummy = PT()) const;
    pnGraph next();
    const pnGraph next() const;
    pnGraph pre();
    const pnGraph pre() const;

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
  template<> pnNode   pnGraph::front<pnNode>  ();
  template<> pnArc    pnGraph::front<pnArc>   ();
  template<> pnGraph  pnGraph::front<pnGraph> ();
  template<> const pnNode   pnGraph::front<pnNode>  () const;
  template<> const pnArc    pnGraph::front<pnArc>   () const;
  template<> const pnGraph  pnGraph::front<pnGraph> () const;
  template<> pnNode   pnGraph::back<pnNode>  ();
  template<> pnArc    pnGraph::back<pnArc>   ();
  template<> pnGraph  pnGraph::back<pnGraph> ();
  template<> const pnNode   pnGraph::back<pnNode>  () const;
  template<> const pnArc    pnGraph::back<pnArc>   () const;
  template<> const pnGraph  pnGraph::back<pnGraph> () const;
  template<> unsigned int pnGraph::size<pnNode>  (const pnNode&) const;
  template<> unsigned int pnGraph::size<pnArc>   (const pnArc&) const;
  template<> unsigned int pnGraph::size<pnGraph> (const pnGraph&) const;

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
    pnGraph front();
    const pnGraph front() const;
    pnGraph back();
    const pnGraph back() const;
    unsigned int size() const;
    pnPetriNet next();
    const pnPetriNet next() const;
    pnPetriNet pre();
    const pnPetriNet pre() const;

    // library methods (only for internal use)
    boost::shared_ptr<details::ddPetriNet> get_() { return p_; } // not to be used by user
    const boost::shared_ptr<details::ddPetriNet> get_() const { return p_; } // not to be used by user

  private:
    bool check_page_set() const;
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
    pnPetriNet front();
    const pnPetriNet front() const;
    pnPetriNet back();
    const pnPetriNet back() const;
    unsigned int size() const;

    // library methods (only for internal use)
    boost::shared_ptr<details::ddPetriNetDoc> get_() { return p_; } // not to be used by user
    const boost::shared_ptr<details::ddPetriNetDoc> get_() const { return p_; } // not to be used by user
   
  private:
    bool check_pn_set() const;
    boost::shared_ptr<details::ddPetriNetDoc> p_;
  };

  // global functions
  bool writeDot(const std::string& fname, const pnGraph& g);           // write out a graph to a dot file 
  bool writeDot(std::ostream& os, const pnGraph& g);                   // write out a graph to a dot file
  bool writeGML(const std::string& fname, const pnGraph& g);           // write out the whole petri-net to a GML file
  bool writeGML(std::ostream& os, const pnGraph& g);                   // write out the whole petri-net to a GML file
  bool writeSVG(const std::string& fname, const pnGraph& g);           // write out the whole petri-net to a SVG file
  bool writeSVG(std::ostream& os, const pnGraph& g);                   // write out the whole petri-net to a SVG file
  bool writePNML(const std::string& fname, const pnPetriNetDoc& pn);   // write out the whole petri-net to a PNML file
  bool writePNML(std::ostream& os, const pnPetriNetDoc& pn);           // write out the whole petri-net to a PNML file
  bool readPNML(const std::string& fname, pnPetriNetDoc& pn);          // read in a PNML file
  pnPetriNetDoc readPNML(const std::string& fname);                    // read in a PNML file
  bool readPNML(std::istream& in, pnPetriNetDoc& pn);                  // read in a PNML file


  // global data
  extern std::string cppPNML_errMsg;
  
}

#endif // CPPPNML_CPPPNML_

// Local Variables:
// mode: c++
// End:
