/*
 * Copyright (c) 2011-2012 Wei Song <songw@cs.man.ac.uk> 
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
 * uniquify command
 * 26/09/2012   Wei Song
 *
 *
 */


// uncomment it when need to debug Spirit.Qi
//#define BOOST_SPIRIT_QI_DEBUG

#include "cmd_define.h"
#include "cmd_parse_base.h"
#include "shell/env.h"
#include "shell/macro_name.h"

using std::endl;
using namespace shell;
using namespace shell::CMD;
using boost::shared_ptr;

namespace {

  namespace qi = boost::spirit::qi;
  namespace phoenix = boost::phoenix;

  struct Argument {
    bool bHelp;                 // show help information
    bool bForce;                // force to uniquify all
    bool bBBox;                 // rename even it is a black box
    bool bQuiet;                // suppress the information output
    
    Argument() : 
      bHelp(false),
      bForce(false),
      bBBox(false),
      bQuiet(false) {}
  };
}

BOOST_FUSION_ADAPT_STRUCT
(
 Argument,
 (bool, bHelp)
 (bool, bForce)
 (bool, bBBox)
 (bool, bQuiet)
 )

namespace {
  typedef std::string::const_iterator SIter;

  struct ArgParser : qi::grammar<SIter, Argument()>, cmd_parse_base<SIter> {
    qi::rule<SIter, void(Argument&)> args;
    qi::rule<SIter, Argument()> start;
    
    ArgParser() : ArgParser::base_type(start) {
      using qi::lit;
      using phoenix::at_c;
      using qi::_r1;
      using qi::_val;

      args = lit('-') >> 
        (
         ("help"   >> blanks ) [at_c<0>(_r1) = true] ||
         ("force"  >> blanks ) [at_c<1>(_r1) = true] ||
         ("dont_skip_empty_designs"  >> blanks ) [at_c<2>(_r1) = true] ||
         ("quiet"  >> blanks ) [at_c<3>(_r1) = true]
         );
      
      start = *args(_val);

#ifdef BOOST_SPIRIT_QI_DEBUG
      BOOST_SPIRIT_DEBUG_NODE(args);
      BOOST_SPIRIT_DEBUG_NODE(start);
      BOOST_SPIRIT_DEBUG_NODE(blanks);
#endif
    }
  };
}

const std::string shell::CMD::CMDUniquify::name = "uniquify"; 
const std::string shell::CMD::CMDUniquify::description = 
  "create unique names for sub-modules.";

void shell::CMD::CMDUniquify::help(Env& gEnv) {
  gEnv.stdOs << name << ": " << description << endl;
  gEnv.stdOs << "    uniquify [options]" << endl;
  gEnv.stdOs << "Options:" << endl;
  gEnv.stdOs << "   -help                usage information." << endl;
  gEnv.stdOs << "   -force               create a new name even the name is already unique." << endl;
  gEnv.stdOs << "   -dont_skip_empty_designs" << endl;
  gEnv.stdOs << "                        rename even it is a black box." << endl;
  gEnv.stdOs << "   -quiet               suppress the uniquifying information." << endl;
}

bool shell::CMD::CMDUniquify::exec(const std::string& str, Env * pEnv) {

  using std::string;

  Env &gEnv = *pEnv;

  // parse
  string::const_iterator it = str.begin(), end = str.end();
  ArgParser parser;             // argument parser
  Argument arg;                 // argument struct
  bool r = qi::parse(it, end, parser, arg);

  if(!r || it != end) {
    gEnv.stdOs << "Error: Wrong command syntax error! See usage by uniquify -help." << endl;
    gEnv.stdOs << "    uniquify [options]" << endl;
    return false;
  }

  if(arg.bHelp) {        // print help information
    help(gEnv);
    return true;
  }

  // get current design
  string cm_name = gEnv.macroDB[MACRO_CURRENT_DESIGN];
  shared_ptr<netlist::Module> cm = gEnv.find_module(cm_name);
  
  if(!cm) {
    gEnv.stdOs << "Error: Fail to find the current design \"" 
               << cm_name << "\"" << endl;
    return false;
  }

  // a set to record all existed modules
  std::set<netlist::MIdentifier> existed_module_set;
  // a map to record the new module name
  std::map<netlist::MIdentifier, netlist::MIdentifier> mname_map;
  //std::set<netlist::MIdentifier> delete_module_set;     // may be deleting unused module is not a good idea
  // a list to record modules waiting for process
  std::list<netlist::MIdentifier> process_module_list;

  process_module_list.push_back(cm->name);
  existed_module_set.insert(cm->name);

  // do the uniquifying
  while(!process_module_list.empty()) {
    shared_ptr<netlist::Module> cur_modu = gEnv.find_module(process_module_list.front());
    process_module_list.pop_front();
    for_each(cur_modu->db_instance.begin(), cur_modu->db_instance.end(),
             [&](std::pair<const netlist::IIdentifier, shared_ptr<netlist::Instance> >& m) {
               //std::cout << m.second->mname << endl;
               shared_ptr<netlist::Module> pm = gEnv.find_module(m.second->mname);
               if(existed_module_set.count(m.second->mname) || arg.bForce) { // existed or forced
                 if(!pm) { // black box
                   if(arg.bBBox) { // rename even it is a black box
                     // get a new name
                     if(!mname_map.count(m.second->mname))
                       mname_map[m.second->mname] = m.second->mname;
                     while(true) {
                       ++mname_map[m.second->mname];
                       if(gEnv.find_module(mname_map[m.second->mname]))
                         continue;
                       else
                         break;
                     }
                     
                     // report
                     if(!arg.bQuiet) {
                       gEnv.stdOs << "rename module \"" 
                                  << m.second->mname 
                                  << "\" to \"" 
                                  << mname_map[m.second->mname].name
                                  << "\"." << endl;
                     }
                     
                     // rename
                     m.second->set_mname(mname_map[m.second->mname]);      // update the name in the instance
                   }
                   
                   existed_module_set.insert(m.second->mname);
                 } else {     // normal module
                   shared_ptr<netlist::Module> new_pm(pm->deep_copy()); // make a duplicate
                   // get a new name
                   if(!mname_map.count(pm->name))
                     mname_map[pm->name] = pm->name;
                   while(true) {
                     ++mname_map[pm->name];
                     if(gEnv.find_module(mname_map[pm->name]))
                       continue;
                     else
                       break;
                   }
                   
                   if(!arg.bQuiet) {
                     gEnv.stdOs << "rename module \"" 
                                << new_pm->name.name 
                                << "\" to \"" 
                                << mname_map[pm->name].name
                                << "\"." << endl;
                   }
                   new_pm->set_name(mname_map[pm->name]);  // set a new name
                   gEnv.curLib->insert(new_pm);            // store it in current library
                   m.second->set_mname(new_pm->name);      // update the name in the instance
                   process_module_list.push_back(new_pm->name);
                   existed_module_set.insert(new_pm->name);
                 }
               } else {
                 process_module_list.push_back(m.second->mname);
                 existed_module_set.insert(m.second->mname);
               }
             });
  }
  return true;
}




