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
 * read a saif file for a design
 * 18/02/2014   Wei Song
 *
 *
 */

// uncomment it when need to debug Spirit.Qi
//#define BOOST_SPIRIT_QI_DEBUG

#include "cmd_define.h"
#include "cmd_parse_base.h"
#include "shell/env.h"
#include "shell/macro_name.h"

#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
using namespace boost::filesystem;

#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include "cppsaif/saif_parser.hpp"
#include "cppsaif/saif_db.hpp"

using std::endl;
using boost::shared_ptr;
using std::list;
using std::vector;
using std::string;
using namespace shell::CMD;


namespace {
  namespace qi = boost::spirit::qi;
  namespace phoenix = boost::phoenix;
  namespace ascii = boost::spirit::ascii;

  struct Argument {
    bool bHelp;                 // show help information
    std::string sDesign;        // target design to be written out
    std::string sSaifTop;       // matched saif top
    std::string sInput;         // output file name
    
    Argument() : 
      bHelp(false),
      sDesign(""),
      sSaifTop(""),
      sInput("") {}
  };
}

BOOST_FUSION_ADAPT_STRUCT
(
 Argument,
 (bool, bHelp)
 (std::string, sDesign)
 (std::string, sSaifTop)
 (std::string, sInput)
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
        ( (lit("help")              >> blanks) [at_c<0>(_r1) = true]         ||
          (lit("top") >> blanks >> identifier >> blanks) [at_c<1>(_r1) = _1] ||
          (lit("saif_top") >> blanks >> text >> blanks)  [at_c<2>(_r1) = _1]
          );
      
      start = 
        *(args(_val))
        >> -(filename >> blanks) [at_c<3>(_val) = _1] 
        >> *(args(_val))
        ;

#ifdef BOOST_SPIRIT_QI_DEBUG
      BOOST_SPIRIT_DEBUG_NODE(args);
      BOOST_SPIRIT_DEBUG_NODE(start);
      BOOST_SPIRIT_DEBUG_NODE(text);
      BOOST_SPIRIT_DEBUG_NODE(blanks);
      BOOST_SPIRIT_DEBUG_NODE(identifier);
      BOOST_SPIRIT_DEBUG_NODE(filename);
#endif
    }
  };

  // annotate an instance
  void annotate(
                shared_ptr<netlist::Module>, shared_ptr<saif::SaifInstance>, 
                unsigned long&, unsigned long&, shell::Env * );

  // annotate a signal
  void annotate_signal(
                       shared_ptr<netlist::Module>,
                       const string&, shared_ptr<saif::SaifSignal>,
                       unsigned long&, unsigned long&);
  
}

const std::string shell::CMD::CMDReadSaif::name = "read_saif"; 
const std::string shell::CMD::CMDReadSaif::description = 
  "read a saif file for a design.";

void shell::CMD::CMDReadSaif::help(Env& gEnv) {
  gEnv.stdOs << name << ": " << description << endl;
  gEnv.stdOs << "    read_saif [options] saif_file" << endl;
  gEnv.stdOs << "    saif_file           the saif file to be read." << endl;
  gEnv.stdOs << "Options:" << endl;
  gEnv.stdOs << "   -help                show this help information." << endl;
  gEnv.stdOs << "   -top name            specify the design top" << endl;
  gEnv.stdOs << "                        (in default is the top module)." << endl;
  gEnv.stdOs << "   -saif_top name       specify the matched saif top." << endl;
}

bool shell::CMD::CMDReadSaif::exec ( const std::string& str, Env * pEnv){

  using std::string;

  Env &gEnv = *pEnv;

  // parse
  string::const_iterator it = str.begin(), end = str.end();
  ArgParser parser;             // argument parser
  Argument arg;                 // argument struct
  bool r = qi::parse(it, end, parser, arg);

  if(!r || it != end) {
    gEnv.stdOs << "Error: Wrong command syntax error! See usage by read_saif -help." << endl;
    gEnv.stdOs << "    read_saif [options] saif_file" << endl;
    return false;
  }

  if(arg.bHelp) {        // print help information
    help(gEnv);
    return true;
  }

  // get the top design
  string designName;
  shared_ptr<netlist::Module> tarDesign;
  if(arg.sDesign.empty()) {
    designName = gEnv.macroDB[MACRO_CURRENT_DESIGN].get_string();
  } else {
    designName = arg.sDesign;
  }
  tarDesign = gEnv.find_module(designName);
  if(tarDesign.use_count() == 0) {
    gEnv.stdOs << "Error: Failed to find the target design \"" << designName << "\"." << endl;
    return false;
  }

  // get the saif file
  if(arg.sInput.empty()) {
    gEnv.stdOs << "Error: A saif file must be specified!" << endl;
    return false;
  }

  if(!exists(system_complete(arg.sInput)) || !is_regular_file(system_complete(arg.sInput))) {
    gEnv.stdOs << "Error: Specified saif file does not existed!" << endl;
    return false;
  }

  // open the file
  ifstream fhandler;
  fhandler.open(system_complete(arg.sInput), std::ios_base::in);

  // parse the file
  saif::SaifParser sparser;
  saif::SaifDB sDB;
  if(!sparser(&sDB, &fhandler)) {
    gEnv.stdOs << "Error: Saif parser failed!" << endl;
    return false;
  }

  // locate the target in saif
  vector<string> hier;
  boost::split(hier, arg.sSaifTop, boost::is_any_of("/"), boost::token_compress_on);
  shared_ptr<saif::SaifInstance> tarSaif = sDB.top;
  BOOST_FOREACH(const string& str, hier) {
    if(!tarSaif->instances.count(str)) {
      gEnv.stdOs << "Error: Fail to find the design name " << str << " in the saif file!" << endl;
      return false;
    } else {
      tarSaif = tarSaif->instances[str];
    }
  }

  unsigned long annotated = 0; 
  unsigned long total = 0;
  annotate(tarDesign, tarSaif, annotated, total, pEnv);

  gEnv.stdOs << "Successfully annotated " << annotated << " in the total of " << total << " signals." << std::endl;

  return true;
}

namespace {
  void annotate(shared_ptr<netlist::Module> tar, shared_ptr<saif::SaifInstance> saif, unsigned long& annotated, unsigned long& total, shell::Env * pEnv) {
    if(!saif->signals.empty()) {
      typedef std::pair<const string&, shared_ptr<saif::SaifSignal> > signal_type;
      BOOST_FOREACH(signal_type sig, saif->signals) {
        annotate_signal(tar, sig.first, sig.second, annotated, total);
      }
    }

    if(!saif->instances.empty()) {
      typedef std::pair<const string&, shared_ptr<saif::SaifInstance> > instance_type;
      BOOST_FOREACH(instance_type inst, saif->instances) {
        shared_ptr<netlist::Module> m_tar;
        if(tar) {
          shared_ptr<netlist::Instance> m_inst = tar->find_instance(inst.first);
          if(m_inst) m_tar = pEnv->find_module(m_inst->mname);
        }
        annotate(m_tar, inst.second, annotated, total, pEnv);
      }
    }
  }

  void annotate_signal(shared_ptr<netlist::Module> tar,
                       const string& name, shared_ptr<saif::SaifSignal> sig,
                       unsigned long& annotated, unsigned long& total) {
    if(sig->bits.empty()) {
      total++;
      bool is_annotated = false;
      if(tar) {
        shared_ptr<netlist::Variable> var = tar->find_var(name);
        if(var) {
          var->annotate(sig->data->TC);
          annotated++;
          is_annotated = true;
        }
      }
      //if(!is_annotated)
      //  std::cout << "signal " << name << " is not annotated." << std::endl;
    } else {
      typedef std::pair<const int, shared_ptr<saif::SaifSignal> > bit_type;
      BOOST_FOREACH(bit_type bit, sig->bits) {
        annotate_signal(tar, name, bit.second, annotated, total);
      }
    }
  }
}
