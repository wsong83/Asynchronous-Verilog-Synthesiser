/*
 * Copyright (c) 2014-2014 Wei Song <songw@cs.man.ac.uk> 
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
 * report the hierarchy of current design
 * 24/03/2014   Wei Song
 *
 *
 */

// uncomment it when need to debug Spirit.Qi
//#define BOOST_SPIRIT_QI_DEBUG

#include "cmd_define.h"
#include "cmd_parse_base.h"
#include "shell/env.h"
#include "shell/macro_name.h"
#include "sdfg/sdfg.hpp"

#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
using namespace boost::filesystem;

#include <boost/foreach.hpp>

using std::endl;
using boost::shared_ptr;
using std::list;
using std::string;
using namespace shell::CMD;


namespace {
  namespace qi = boost::spirit::qi;
  namespace phoenix = boost::phoenix;
  namespace ascii = boost::spirit::ascii;

  struct Argument {
    bool bHelp;                 // show help information
    bool bVerbose;              // show internal information
    
    Argument() : 
      bHelp(false),
      bVerbose(false)
    {}
  };

  enum IO_TYPE {
    IO_MEM              = 0x00001, // memory interface
    IO_HS               = 0x00002  // handshake
  };

  void report_hierarchy(shared_ptr<SDFG::dfgGraph>, unsigned int, bool);
  unsigned int port_type(shared_ptr<SDFG::dfgGraph>, shared_ptr<SDFG::dfgGraph>, shared_ptr<SDFG::dfgNode>, unsigned int, bool verbose = false);
  void print_type(unsigned int);
}

BOOST_FUSION_ADAPT_STRUCT
(
 Argument,
 (bool, bHelp)
 (bool, bVerbose)
 )

namespace {
  typedef std::string::const_iterator SIter;

  struct ArgParser : qi::grammar<SIter, Argument()>, cmd_parse_base<SIter> {
    qi::rule<SIter, void(Argument&)> args;
    qi::rule<SIter, Argument()> start;
    
    ArgParser() : ArgParser::base_type(start) {
      using qi::lit;
      using ascii::char_;
      using ascii::space;
      using phoenix::at_c;
      using namespace qi::labels;

      args = lit('-') >> 
        ( (lit("help")    >> blanks)                        [at_c<0>(_r1) = true]  ||
          (lit("verbose") >> blanks)                        [at_c<1>(_r1) = true] 
          );
      
      start = *(args(_val));


#ifdef BOOST_SPIRIT_QI_DEBUG
      BOOST_SPIRIT_DEBUG_NODE(args);
      BOOST_SPIRIT_DEBUG_NODE(start);
      BOOST_SPIRIT_DEBUG_NODE(identifier);
#endif
    }
  };
}

const std::string shell::CMD::CMDReportHierarchy::name = "report_hierarchy"; 
const std::string shell::CMD::CMDReportHierarchy::description = 
  "report the hierarchy of the current design.";

void shell::CMD::CMDReportHierarchy::help(Env& gEnv) {
  gEnv.stdOs << name << ": " << description << endl;
  gEnv.stdOs << "    report_hierarchy [-help]" << endl;
  gEnv.stdOs << "Options:" << endl;
  gEnv.stdOs << "   -help                show this help information." << endl;
  gEnv.stdOs << "   -verbose             show internal information." << endl;
}

bool shell::CMD::CMDReportHierarchy::exec ( const std::string& str, Env * pEnv){

  using std::string;

  Env &gEnv = *pEnv;

  // parse
  string::const_iterator it = str.begin(), end = str.end();
  ArgParser parser;             // argument parser
  Argument arg;                 // argument struct
  bool r = qi::parse(it, end, parser, arg);

  if(!r || it != end) {
    gEnv.stdOs << "Error: Wrong command syntax error! See usage by report_hierarchy -help." << endl;
    gEnv.stdOs << "    report_hierarchy [-help]" << endl;
    return false;
  }

  if(arg.bHelp) {        // print help information
    help(gEnv);
    return true;
  }

  // find the target design
  string designName;
  shared_ptr<netlist::Module> tarDesign;
  designName = gEnv.macroDB[MACRO_CURRENT_DESIGN].get_string();
  tarDesign = gEnv.find_module(designName);
  if(tarDesign.use_count() == 0) {
    gEnv.stdOs << "Error: Failed to find the target design \"" << designName << "\"." << endl;
    return false;
  }

  // check DFG is ready
  if(!tarDesign->DFG) {
    gEnv.stdOs << "Error: DFG is not extracted for the target design \"" << designName << "\"." << endl;
    gEnv.stdOs << "       Use extract_sdfg before report partition." << endl;
    return false;
  } 

  // check Datapath DFG is ready
  if(!tarDesign->DataDFG) {
    gEnv.stdOs << "Error: Datapaths are not extracted for the target design \"" << designName << "\"." << endl;
    gEnv.stdOs << "       Use extract_datapath before report partition." << endl;
    return false;      
  }

  report_hierarchy(tarDesign->DFG, 0, arg.bVerbose);

  return true;
}

namespace {
  void report_hierarchy(shared_ptr<SDFG::dfgGraph> g, unsigned int indent, bool verbose) {
    // get the data path graph
    shared_ptr<SDFG::dfgGraph> dg = g->pModule->DataDFG;
    if(dg) {
      list<shared_ptr<SDFG::dfgNode> > plist = dg->get_list_of_nodes(SDFG::dfgNode::SDFG_PORT);
      list<shared_ptr<SDFG::dfgNode> > iports;
      list<shared_ptr<SDFG::dfgNode> > oports;
      BOOST_FOREACH(shared_ptr<SDFG::dfgNode> p, plist) {
        if(p->type == SDFG::dfgNode::SDFG_PORT || p->type == SDFG::dfgNode::SDFG_IPORT)
          iports.push_back(p);
        if(p->type == SDFG::dfgNode::SDFG_PORT || p->type == SDFG::dfgNode::SDFG_OPORT)
          oports.push_back(p);
      }
      list<shared_ptr<SDFG::dfgNode> > mlist = g->get_list_of_nodes(SDFG::dfgNode::SDFG_MODULE);

      if(iports.size()) {
        std::cout << string(indent, ' ') << "[I]" << endl;
        BOOST_FOREACH(shared_ptr<SDFG::dfgNode> p, iports) {
          unsigned int ptype = port_type(g, dg, p, indent+2, verbose);
          if(ptype || verbose) {
            std::cout << string(indent, ' ') << p->get_hier_name();
            if(ptype) print_type(ptype);
            std::cout << endl;
          }
        }
      }
      
      if(oports.size()) {
        std::cout << string(indent, ' ') << "[O]" << endl;
        BOOST_FOREACH(shared_ptr<SDFG::dfgNode> p, oports) {
          unsigned int ptype = port_type(g, dg, p, indent+2, verbose);
          if(ptype || verbose) {
            std::cout << string(indent, ' ') << p->get_hier_name();
            if(ptype) print_type(ptype);
            std::cout << endl;
          }
        }
      }

      if(mlist.size()) {
        std::cout << string(indent, ' ') << "[M]" << endl;
        BOOST_FOREACH(shared_ptr<SDFG::dfgNode> m, mlist) {
          std::cout << string(indent, ' ') << "**" << m->get_hier_name() << endl;
          report_hierarchy(m->child, indent + 4, verbose);
        }
      }
    }
  }

  unsigned int port_type(shared_ptr<SDFG::dfgGraph> g, shared_ptr<SDFG::dfgGraph> dg, 
                         shared_ptr<SDFG::dfgNode> dport, unsigned int indent, bool verbose) {
    unsigned int ptype = 0;
   
    // exam for memory interfaces
    if(dport->type == SDFG::dfgNode::SDFG_OPORT) {
      shared_ptr<SDFG::dfgNode> port = g->get_node(SDFG::divide_signal_name(dport->get_hier_name()));
      if(port->get_in_edges_type() == SDFG::dfgEdge::SDFG_ASS)
        port = port->get_in_nodes().front();
      
      list<shared_ptr<SDFG::dfgPath> > ipaths = port->get_in_paths_fast_in(g.get());
      // get the combined type
      unsigned int comb_itype = 0;
      BOOST_FOREACH(shared_ptr<SDFG::dfgPath> p, ipaths)
        comb_itype |= p->type;
      
      if(comb_itype & SDFG::dfgEdge::SDFG_ADR) { // possible a memory output
        list<shared_ptr<SDFG::dfgNode> > adr;
        list<shared_ptr<SDFG::dfgNode> > mem;
        BOOST_FOREACH(shared_ptr<SDFG::dfgPath> p, ipaths) {
          if(p->type & SDFG::dfgEdge::SDFG_ADR)
            adr.push_back(p->src);
          else if(p->type & SDFG::dfgEdge::SDFG_DAT_MASK && p->src != port) {
            if(p->src->get_in_edges_type() & SDFG::dfgEdge::SDFG_ADR)
              mem.push_back(p->src);
          }
        }
        if(adr.size() > 0 && mem.size() > 0) {
          if(verbose) {
            std::cout << "M: " << dport->get_full_name() << " [MEM DOUT]: " << std::endl;
            std::cout << std::string(4, ' ');
            BOOST_FOREACH(shared_ptr<SDFG::dfgNode> m, mem)
              std::cout << m->get_full_name() << " ";
            BOOST_FOREACH(shared_ptr<SDFG::dfgNode> a, adr)
              std::cout << "[" << a->get_full_name() << "]";
            std::cout << endl;
          }
          ptype = IO_MEM;
        }
      }
    }

    if(!(ptype & IO_MEM) && dport->pg->father != NULL) {     // check for handshake, new
      shared_ptr<SDFG::dfgNode> port = g->get_node(SDFG::divide_signal_name(dport->get_hier_name()));

      // find the connected flip-flops
      std::set<shared_ptr<SDFG::dfgNode> > connNodes; // connect nodes in other modules
      std::set<shared_ptr<SDFG::dfgNode> > locNodes;  // the nodes in current modules
      list<shared_ptr<SDFG::dfgPath> > mLocPaths;     // local paths
      list<shared_ptr<SDFG::dfgPath> > mConnPaths;    // connect paths
      
      // assembly the sets
      if(port->type == SDFG::dfgNode::SDFG_IPORT) {
        mLocPaths = dport->get_out_paths_fast_in(dport->pg);
        mConnPaths = dport->get_in_paths_fast_cb();
        BOOST_FOREACH(shared_ptr<SDFG::dfgPath> p, mLocPaths)
          locNodes.insert(p->tar->get_synonym(p->tar->pg->pModule->DFG.get()));
        BOOST_FOREACH(shared_ptr<SDFG::dfgPath> p, mConnPaths)
          connNodes.insert(p->src->get_synonym(p->src->pg->pModule->DFG.get()));
      } else {
        mLocPaths = dport->get_in_paths_fast_in(dport->pg);
        mConnPaths = dport->get_out_paths_fast_cb();          
        BOOST_FOREACH(shared_ptr<SDFG::dfgPath> p, mLocPaths)
          locNodes.insert(p->src->get_synonym(p->src->pg->pModule->DFG.get()));
        BOOST_FOREACH(shared_ptr<SDFG::dfgPath> p, mConnPaths)
          connNodes.insert(p->tar->get_synonym(p->tar->pg->pModule->DFG.get()));
      }

      if(locNodes.size() == 1 && connNodes.size() == 1) { // only consider when it is one to one case
        shared_ptr<SDFG::dfgNode> connNode = *(connNodes.begin());
        shared_ptr<SDFG::dfgNode> locNode = *(locNodes.begin());

        // find out the local and connected module
        shared_ptr<SDFG::dfgGraph> connModule = locNode->get_connected_module(connNode);
        shared_ptr<SDFG::dfgGraph> locModule = connNode->get_connected_module(locNode);
        
        if(connModule && locModule 
           //&& locModule.get() == port->pg
           ) {

          // find out the common control
          std::set<shared_ptr<SDFG::dfgNode> > locCtls, connCtls, commCtls;
          list<shared_ptr<SDFG::dfgPath> > ctlPaths = locNode->get_in_paths_fast_cb();
          BOOST_FOREACH(shared_ptr<SDFG::dfgPath> p, ctlPaths) {
            if(p->src->belong_to(locModule.get()))
              locCtls.insert(p->src);
            else if(p->src->belong_to(connModule.get()))
              connCtls.insert(p->src);
          }
          ctlPaths = connNode->get_in_paths_fast_cb();
          BOOST_FOREACH(shared_ptr<SDFG::dfgPath> p, ctlPaths) {
            if(p->src->belong_to(locModule.get()) && locCtls.count(p->src))
              commCtls.insert(p->src);
            else if(p->src->belong_to(connModule.get()) && connCtls.count(p->src))
              commCtls.insert(p->src);
          }
          
          if(commCtls.size() > 0) { // probably handshake
            if(verbose) {
              std::cout << "H: " << dport->get_full_name() << " [Hand Shake]: " << std::endl;
              std::cout << std::string(4, ' ');
              BOOST_FOREACH(shared_ptr<SDFG::dfgNode> l, commCtls)
                std::cout << l->get_full_name() << " ";
              std::cout << endl;
            }
            ptype = IO_HS;
          }
        }
      }
    }

    return ptype;
  }

  void print_type(unsigned int t) {
    switch(t) {
    case IO_MEM: std::cout << "[Mem]"; break;
    case IO_HS:  std::cout << "[Handshake]"; break;
    default:     std::cout << "[Unkown]";
    }
  }

}
