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
using std::map;
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

  double report_hierarchy(shared_ptr<SDFG::dfgGraph>, unsigned int, bool);
  void report_partition(shared_ptr<SDFG::dfgGraph>, unsigned int, bool, double);
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

  report_hierarchy(tarDesign->DataDFG, 0, arg.bVerbose);
  
  std::cout << "Summary:" << std::endl;
  report_partition(tarDesign->DataDFG, 0, true, 0.7);

  return true;
}

namespace {
  using namespace SDFG;
  typedef map<string, list<shared_ptr<dfgNode> > > info_map;

  void print_node_list(list<shared_ptr<dfgNode> >& nlist) {
    BOOST_FOREACH(shared_ptr<dfgNode> n, nlist) {
      if(n)
        std::cout << n->get_full_name() << " ";
      else
        std::cout << "@ ";
    }
  }

  double get_port_score(info_map& imap) {
    if(imap.count("MEM") || imap.count("BUS") || imap.count("WIRE"))
      return 1.0;
    else if(imap.count("HAND")) {
      if(imap["HAND"].size() < 2)
        return 1.0;
      else
        return 0.5;
    } else
      return 0.0;
  }

  void print_ptype(info_map& imap, unsigned int indent) {
    enum interface_type_t { 
      SDFG_INT_NONE       = 0x00000, // unknown yet
      SDFG_INT_WIRE       = 0x00001, // wire
      SDFG_INT_PIPE       = 0x00002, // pipeline
      SDFG_INT_MEM        = 0x00004, // memory
      SDFG_INT_BUS        = 0x00008, // bus
      SDFG_INT_HAND       = 0x00010, // handshake
      SDFG_INT_OTHER      = 0x80000  // currently cannot understand
    };

    if(imap.count("NONE")) {
      std::cout << string(indent, ' ') << "NONE: ";
      print_node_list(imap["None"]);
      std::cout << endl;
    }

    if(imap.count("WIRE")) {
      std::cout << string(indent, ' ') << "WIRE: ";
      print_node_list(imap["WIRE"]);
      std::cout << endl;
    }

    if(imap.count("PIPE")) {
      std::cout << string(indent, ' ') << "PIPE: ";
      print_node_list(imap["PIPE"]);
      std::cout << endl;
    }

    if(imap.count("MEM")) {
      std::cout << string(indent, ' ') << "MEM: ";
      print_node_list(imap["MEM"]);
      std::cout << endl;
    }

    if(imap.count("BUS")) {
      std::cout << string(indent, ' ') << "BUS: ";
      print_node_list(imap["BUS"]);
      std::cout << endl;
    }

    if(imap.count("HAND")) {
      std::cout << string(indent, ' ') << "HAND: ";
      print_node_list(imap["HAND"]);
      std::cout << endl;
    }

    if(imap.count("OTHER")) {
      std::cout << string(indent, ' ') << "OTHER: ";
      print_node_list(imap["OTHER"]);
      std::cout << endl;
    }
  }

  shared_ptr<dfgNode> get_SDFG_node(shared_ptr<dfgNode> n) {
    shared_ptr<dfgNode> m = n->pg->pModule->DFG->get_node(divide_signal_name(n->get_hier_name()));
    return m;
  }

  shared_ptr<dfgNode> get_DATA_node(shared_ptr<dfgNode> n) {
    shared_ptr<dfgNode> m = n->pg->pModule->DataDFG->get_node(divide_signal_name(n->get_hier_name()));
    return m;
  }


  shared_ptr<dfgNode> get_driving_node(shared_ptr<dfgNode> port) {
    shared_ptr<dfgNode> rv = get_SDFG_node(port);
    while(rv->size_in_edges_cb() == 1 && rv->get_in_edges_type_cb() == dfgEdge::SDFG_ASS) {
      shared_ptr<dfgNode> newDrive = rv->get_in_nodes_cb().front();
      if(get_DATA_node(newDrive)) rv = newDrive;
      else break;
    }
    return get_DATA_node(rv);
  }

  void interface_check_wire(shared_ptr<dfgNode> node, info_map& rv) {
    if(node->type & (dfgNode::SDFG_LATCH | dfgNode::SDFG_FF)) 
      return;

    list<shared_ptr<dfgPath> > iplist = node->get_in_paths_cb();
    unsigned int nitype = dfgNode::SDFG_IPORT;
    list<shared_ptr<dfgNode> > nilist;
    BOOST_FOREACH(shared_ptr<dfgPath> p, iplist) {
      nitype |= p->src->type;
      if(nitype == dfgNode::SDFG_IPORT)
        nilist.push_back(p->src);
      else
        break;
    }

    list<shared_ptr<dfgPath> > oplist = node->get_out_paths_cb();
    unsigned int notype = dfgNode::SDFG_OPORT;
    list<shared_ptr<dfgNode> > nolist;
    BOOST_FOREACH(shared_ptr<dfgPath> p, oplist) {
      notype |= p->tar->type;
      if(notype == dfgNode::SDFG_OPORT)
        nolist.push_back(p->tar);
      else
        break;
    }

    if(nitype != dfgNode::SDFG_IPORT && notype != dfgNode::SDFG_OPORT)
      return;
    else {
      if(nilist.empty())  rv["WIRE"].push_back(node);
      if(nitype == dfgNode::SDFG_IPORT)
        rv["WIRE"].insert(rv["WIRE"].end(), nilist.begin(), nilist.end());
      if(notype == dfgNode::SDFG_OPORT)
        rv["WIRE"].insert(rv["WIRE"].end(), nolist.begin(), nolist.end());
    }
  }

  void interface_check_pipe(shared_ptr<dfgNode> node, info_map& rv) {
    std::set<shared_ptr<dfgNode> > levelOne, levelTwo;

    // level two FFs -> level one FFs

    // get the level one
    if(node->type & (dfgNode::SDFG_LATCH | dfgNode::SDFG_FF))
      levelOne.insert(node);
    else {
      list<shared_ptr<dfgPath> > ipaths = node->get_in_paths_cb();
      BOOST_FOREACH(shared_ptr<dfgPath> p, ipaths) {
        if(p->src == node) return;
        if(p->src->type & (dfgNode::SDFG_LATCH | dfgNode::SDFG_FF)) {
          levelOne.insert(p->src);
        }
      }
    }

    // check level one anf form level two
    BOOST_FOREACH(shared_ptr<dfgNode> n, levelOne) {
      list<shared_ptr<dfgPath> > ipaths = n->get_in_paths_cb();
      BOOST_FOREACH(shared_ptr<dfgPath> p, ipaths) {
        if(p->src == n) return;
        levelTwo.insert(p->src);
      }
    }

    // check level two
    BOOST_FOREACH(shared_ptr<dfgNode> n, levelTwo) {
      list<shared_ptr<dfgPath> > ipaths = n->get_in_paths_cb();
      BOOST_FOREACH(shared_ptr<dfgPath> p, ipaths) {
        if(p->src == n) return;
      }
    }

    if(!levelOne.empty()) {
      rv["PIPE"].insert(rv["PIPE"].end(), levelOne.begin(), levelOne.end());
      rv["PIPE"].insert(rv["PIPE"].end(), levelTwo.begin(), levelTwo.end());
    }
  }

  void interface_check_mem(shared_ptr<dfgNode> node, info_map& rv) {
    list<shared_ptr<dfgPath> > plist = get_SDFG_node(node)->get_in_paths_fast_im();
    unsigned int ptype = 0;
    BOOST_FOREACH(shared_ptr<dfgPath> p, plist)
        ptype |= p->type;

    if(ptype & dfgEdge::SDFG_ADR) {
        list<shared_ptr<dfgNode> > adr;
        list<shared_ptr<dfgNode> > mem;
        BOOST_FOREACH(shared_ptr<dfgPath> p, plist) {
          if(p->type & dfgEdge::SDFG_ADR)
            adr.push_back(p->src);
          
          if(p->type & dfgEdge::SDFG_DAT_MASK && p->src != p->tar) {
            if(p->src->get_in_edges_type() & dfgEdge::SDFG_ADR)
              mem.push_back(p->src);
          }
        }
        if(mem.size() > 0) {
          rv["MEM"] = mem;
          rv["MEM"].push_back(shared_ptr<dfgNode>());
          rv["MEM"].insert(rv["MEM"].end(), adr.begin(), adr.end());
        }
    }
  }

  void interface_check_bus(shared_ptr<dfgNode> node, info_map& rv) {
    // currently have no idea about how to do it

  }

  void interface_check_hand(shared_ptr<dfgNode> node, info_map& rv) {
    list<shared_ptr<dfgPath> > iplist, oplist;
    std::set<shared_ptr<dfgNode> > srcs, tars, ctls;
    
    if(node->type & dfgNode::SDFG_FF)
      srcs.insert(get_SDFG_node(node));
    else
      iplist = node->get_in_paths_fast_cb();
    oplist = node->get_out_paths_fast_cb();
    
    BOOST_FOREACH(shared_ptr<dfgPath> p, iplist) {
      if(p->src->type & dfgNode::SDFG_FF)
        srcs.insert(get_SDFG_node(p->src));
    }

    BOOST_FOREACH(shared_ptr<dfgPath> p, oplist) {
      if(p->tar->type & dfgNode::SDFG_FF && !srcs.count(get_SDFG_node(p->tar))) {
        tars.insert(get_SDFG_node(p->tar));
      }
    }

    // current only linear handshake pipeline is considered
    //if(srcs.size() != 1 && tars.size() != 1) {
      //std::cout << "Checking Handshake for " << node->get_full_name() << " finds out it is not a linear pipeline." << std::endl;
      //return;
    //}


    BOOST_FOREACH(shared_ptr<dfgNode> src, srcs) {
      BOOST_FOREACH(shared_ptr<dfgNode> tar, tars) {
        //std::cout << "check handshake from "<< src->get_full_name() << " to " << tar->get_full_name() << std::endl;
        std::set<shared_ptr<dfgNode> > src_ctls, all_ctls;
        list<shared_ptr<dfgPath> > clist = src->get_in_paths_fast_cb();
        BOOST_FOREACH(shared_ptr<dfgPath> p, clist) {
          if(p->type & (dfgEdge::SDFG_EQU | dfgEdge::SDFG_CMP | dfgEdge::SDFG_LOG) &&
             !(p->type & dfgEdge::SDFG_ADR) && 
             !(p->type & dfgEdge::SDFG_DAT_MASK) &&
             p->src != p->tar)
            src_ctls.insert(p->src);
        }
        
        clist = tar->get_in_paths_fast_cb();
        BOOST_FOREACH(shared_ptr<dfgPath> p, clist) {
          if(p->type & (dfgEdge::SDFG_EQU | dfgEdge::SDFG_CMP | dfgEdge::SDFG_LOG) &&
             !(p->type & dfgEdge::SDFG_ADR) && 
             !(p->type & dfgEdge::SDFG_DAT_MASK) && 
             p->src != p->tar && 
             src_ctls.count(p->src)) {
            all_ctls.insert(p->src);
          }
        }  
        
        BOOST_FOREACH(shared_ptr<dfgNode> n, all_ctls) {
          bool bs = src->belong_to(n->pg);
          bool bt = tar->belong_to(n->pg);
          if((bs && !bt) || (!bs && bt)) {
            //if(!ctls.count(n))
            //  std::cout << "find a controller " << n->get_full_name() << " for path from " << src->get_full_name() << " to " << tar->get_full_name() << std::endl;
            ctls.insert(n);
          }
        }
      }
    }

    if(!ctls.empty())
      rv["HAND"].insert(rv["HAND"].end(), ctls.begin(), ctls.end());
  }

  

  info_map interface_type(shared_ptr<dfgNode> port_arg) {
    info_map rv;
    shared_ptr<dfgNode> node = get_driving_node(port_arg);
    if(!node->interface_map.empty())
      return node->interface_map;

    interface_check_wire(node, rv);
    interface_check_pipe(node, rv);
    interface_check_mem(node, rv);
    interface_check_bus(node, rv);
    interface_check_hand(node, rv);

    node->interface_map = rv;
    return rv;
  }

  double report_hierarchy(shared_ptr<SDFG::dfgGraph> dg, unsigned int indent, bool verbose) {
    double iprate = 0.0, oprate = 0.0, prate = 0.0;
    if(dg) {
      list<shared_ptr<dfgNode> > plist = dg->get_list_of_nodes(dfgNode::SDFG_PORT);
      list<shared_ptr<dfgNode> > iports;
      list<shared_ptr<dfgNode> > oports;
      BOOST_FOREACH(shared_ptr<dfgNode> p, plist) {
        if(p->type == dfgNode::SDFG_PORT || p->type == dfgNode::SDFG_IPORT)
          iports.push_back(p);
        if(p->type == dfgNode::SDFG_PORT || p->type == dfgNode::SDFG_OPORT)
          oports.push_back(p);
      }
      list<shared_ptr<dfgNode> > mlist = dg->get_list_of_nodes(dfgNode::SDFG_MODULE);

      if(iports.size()) {
        std::cout << string(indent, ' ') << "[I]" << endl;
        BOOST_FOREACH(shared_ptr<dfgNode> p, iports) {
          info_map ptype  = interface_type(p);
          std::cout << string(indent, ' ') << p->get_hier_name() << endl;
          print_ptype(ptype, indent+2);
          iprate += get_port_score(ptype);
        }
      }
      
      if(oports.size()) {
        std::cout << string(indent, ' ') << "[O]" << endl;
        BOOST_FOREACH(shared_ptr<dfgNode> p, oports) {
          info_map ptype  = interface_type(p);
          std::cout << string(indent, ' ') << p->get_hier_name() << endl;
          print_ptype(ptype, indent+2);
          oprate += get_port_score(ptype);
        }
      }

      if(mlist.size()) {
        std::cout << string(indent, ' ') << "[M]" << endl;
        BOOST_FOREACH(shared_ptr<dfgNode> m, mlist) {
          std::cout << string(indent, ' ') << "**" << m->get_hier_name() << "(" << m->child_name << ")" << endl;
          m->partition_rate = report_hierarchy(m->child, indent + 4, verbose);
        }
      }

      if(iprate != 0.0 && oprate != 0.0)
        prate = (iprate + oprate) / (iports.size() + oports.size());
    }
    return prate;
  }

  bool has_partition(shared_ptr<SDFG::dfgNode> dm, double threshold) {
    if(dm) {
      if(dm->partition_rate >= threshold) return true;
      
      shared_ptr<dfgGraph> dg = dm->child;
      if(!dg) return false;
      
      list<shared_ptr<dfgNode> > mlist = dg->get_list_of_nodes(dfgNode::SDFG_MODULE);
      BOOST_FOREACH(shared_ptr<dfgNode> m, mlist) {
        if(has_partition(m, threshold)) return true;
      }
    } 
    return false;
  }

  void report_partition(shared_ptr<SDFG::dfgGraph> dg, unsigned int indent, bool first_line, double threshold) {
    std::set<shared_ptr<dfgNode> > pset, npset;
    list<shared_ptr<dfgNode> > mlist = dg->get_list_of_nodes(dfgNode::SDFG_MODULE);
    BOOST_FOREACH(shared_ptr<dfgNode> m, mlist) {
      if(has_partition(m, threshold)) pset.insert(m);
      else npset.insert(m);
    }
    if(pset.size()) {
      BOOST_FOREACH(shared_ptr<dfgNode> m, pset) {
        if(m->partition_rate >= threshold) {
          std::cout << (first_line ? string(indent, ' ') : string()) << m->get_hier_name();
          std::cout << "(" << m->partition_rate << ")" << std::endl;
          report_partition(m->child, indent+m->get_hier_name().length(), true, threshold);
        } else {
          std::cout << (first_line ? string(indent, ' ') : string()) << m->get_hier_name() << "/";
          report_partition(m->child, indent+m->get_hier_name().length()+1, false, threshold);
        }
        first_line = true;
      }
      if(npset.size()) {
        std::cout << string(indent, ' ') << "[Other]";
        BOOST_FOREACH(shared_ptr<dfgNode> m, npset) {
          std::cout << m->get_hier_name() << ";";
        }
        std::cout << std::endl;
      }
    }
  }
}
