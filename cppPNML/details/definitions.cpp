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
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp> 
#include <iostream>
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
using std::set;
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
static const double LAYER_DIST = 1.0;
static const double NODE_DIST = 1.5;

namespace cppPNML {
  string cppPNML_errMsg;        // simple error report technique

  namespace details {
    namespace {
      // local functions
      unsigned long shash(const string& str) {
        unsigned int id_size = 32; // assuming all system has a long larger than 4 bytes
        unsigned long rv = 0;
        for(unsigned int i=0; i<str.size(); i++) {
          unsigned long highbit = rv >> (id_size - 2);
          rv <<= 7;
          rv &= 0xffffffff;
          rv |= str[i];
          rv ^= highbit;
        }
        return rv;
      }

      // display the hash id of a tring
      void show_hash(const string& str) {
        std::cout << "hash id of \"" << str << "\":" << boost::format("0x%x") % shash(str) << std::endl;
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////
// details

#include "ddObj.cc"
#include "ddNode.cc"
#include "ddPlace.cc"
#include "ddTransition.cc"
#include "ddArc.cc"
#include "ddGraph.cc"
#include "ddPetriNet.cc"
#include "ddPetriNetDoc.cc"

// some functions of cppPNML because the requirement of OGDF
bool cppPNML::writeGML(ostream& os, const pnGraph& g) {
  ogdf::Graph gg;
  ogdf::GraphAttributes gga;
  g.get_()->write_ogdf(&gg, &gga);
  gga.writeGML(os);
  return true;
}

bool cppPNML::writeGML(const string& fname, const pnGraph& g) {
  ogdf::Graph gg;
  ogdf::GraphAttributes gga;
  g.get_()->write_ogdf(&gg, &gga);
  gga.writeGML(fname.c_str());
  return true;
}

bool cppPNML::writeSVG(ostream& os, const pnGraph& g) {
  ogdf::Graph gg;
  ogdf::GraphAttributes gga;
  g.get_()->write_ogdf(&gg, &gga);
  gga.writeSVG(os, 0, "#000000");
  return true;
}

bool cppPNML::writeSVG(const string& fname, const pnGraph& g) {
  ogdf::Graph gg;
  ogdf::GraphAttributes gga;
  g.get_()->write_ogdf(&gg, &gga);
  gga.writeSVG(fname.c_str());
  return true;
}
