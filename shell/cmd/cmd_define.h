/*
 * Copyright (c) 2011-2014 Wei Song <songw@cs.man.ac.uk> 
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
 * argument definitions
 * 10/05/2012   Wei Song
 *
 *
 */

#ifndef AV_CMD_CMD_DEFINE_
#define AV_CMD_CMD_DEFINE_

#include <iostream>
#include <string>
#include <map>
#include <utility>
#include <boost/function.hpp>

#ifndef NEW_TCL_CMD
#define NEW_TCL_CMD(rt_type, cmd_name)                         \
struct cmd_name {                                              \
  static rt_type exec ( const std::string&, Env *);            \
  static void help ( Env& );                                   \
  static const std::string name;                               \
  static const std::string description;                        \
}
#endif                               

namespace shell {
  class Env;
  namespace CMD {
    NEW_TCL_CMD(bool, CMDAnalyze);
    NEW_TCL_CMD(bool, CMDAnnotateSaif);
    NEW_TCL_CMD(std::string, CMDCurrentDesign);
    NEW_TCL_CMD(std::string, CMDEcho);
    NEW_TCL_CMD(bool, CMDElaborate);
    NEW_TCL_CMD(void, CMDExit);
    NEW_TCL_CMD(void, CMDExtractDatapath);
    NEW_TCL_CMD(void, CMDExtractRRG);
    NEW_TCL_CMD(void, CMDExtractSDFG);
    NEW_TCL_CMD(bool, CMDPartition);
    NEW_TCL_CMD(bool, CMDReadSaif);
    NEW_TCL_CMD(bool, CMDReportDFGPath);
    NEW_TCL_CMD(bool, CMDReportFSM);
    NEW_TCL_CMD(bool, CMDReportHierarchy);
    NEW_TCL_CMD(bool, CMDReportNetlist);
    NEW_TCL_CMD(bool, CMDReportPartition);
    NEW_TCL_CMD(bool, CMDReportPorts);
    NEW_TCL_CMD(bool, CMDSetPortType);
    NEW_TCL_CMD(std::string, CMDShell);
    NEW_TCL_CMD(void, CMDStateSpaceAnalysis);
    NEW_TCL_CMD(bool, CMDSuppressMessage);
    NEW_TCL_CMD(bool, CMDUniquify);
    NEW_TCL_CMD(bool, CMDWrite);
    NEW_TCL_CMD(bool, CMDWriteSDFG);


    // help is different
    struct CMDHelp {
      static void exec ( const std::string&, Env *);
      static void help ( Env& );
      static const std::string name;
      static const std::string description;
      typedef std::pair<std::string, boost::function1<void, Env&> > cmd_record;
      static std::map<std::string, cmd_record> cmdDB;
    };

  }
}


#undef NEW_TCL_CMD
#endif

// Local Variables:
// mode: c++
// End:
