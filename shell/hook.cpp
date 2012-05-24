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
 * Command line variable hooks
 * 24/05/2012   Wei Song
 *
 *
 */

// no guard needed as this file is only included in env.cpp

namespace shell {
  namespace CMD {
    class CMDHook_TMP_PATH : public CMDVarHook {
    public:
      virtual bool operator() (Env& gEnv, CMDVar& var, const vector<string>& val) {
        if(val.size() < 1 || val.front().empty()) {
          gEnv.stdOs << "Error: Empty temporary path name." << endl;
          return false;
        }

        if(var.get_string() == val.front()) {
          // already set, nothing to do
          return true;
        }

        // remove the old tmp directory
        path tmp_old_path(var.get_string());
        path tmp_new_path(val.front());
        try {
          if(exists(tmp_old_path)) {
            remove_all(tmp_old_path);
          }
          if(!exists(tmp_new_path)) {
            if(!create_directory(tmp_new_path)) {
              throw std::exception();
            }
          }
        } catch (std::exception e) {
          throw("Error! problem with removing or creating the temporary work directory.");
        }
        
        var = val.front();
        return true;
      }
    }; 

    class CMDHook_CURRENT_DESIGN : public CMDVarHook {
    public:
      virtual bool operator() (Env& gEnv, CMDVar& var, const vector<string>& val) {
        if(val.size() < 1 || val.front().empty()) {
          gEnv.stdOs << "Error: design name empty." << endl;
          return false;
        }

        if(var.get_string() == val.front()) {
          // already set, nothing to do
          return true;
        }

        string designName = val.front();

        // try to locate the module in all link libraries
        map<string, shared_ptr<netlist::Library> >::iterator it, end;
        shared_ptr<netlist::Module> tarModule;
        netlist::MIdentifier mName(designName); // convert the string to MIdentifier first
        for(it=gEnv.link_lib.begin(), end=gEnv.link_lib.end(); it!=end; it++) {
          tarModule = it->second->find(mName);
          if(tarModule.use_count() != 0) break;
        }
        
        if(tarModule.use_count() == 0) { // fail to find the design
          gEnv.stdOs << "Error: target design \"" << designName << "\" not found." << endl;
          return false;
        } else { //change the current design
          var = designName;
          gEnv.curDgn = tarModule;
          return true;
        }
      }
    }; 
  }
}
