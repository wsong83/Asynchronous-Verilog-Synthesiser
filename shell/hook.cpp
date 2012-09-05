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

static bool cmd_create_tmp_path(const path& p, Env * pEnv, bool init = false ) {
  try {
    if(!exists(p)) {
      if(!create_directory(p)) {
        throw Tcl::tcl_error("Error! Fail to create the temporary work directory \"" + p.string() + "\".");
      }
    } else if(!init && !is_empty(p)) {
      throw Tcl::tcl_error("Error! The temporary work directory \"" + p.string() + "\" already exists and is not empty.");
    } else if(init)
      remove_all(p);
  } catch (Tcl::tcl_error& e) {
    if(init) throw e;           // if it is init, no old tmp path exist, just throw
    pEnv->stdOs << "[Tcl] " << e.what() << std::endl;
    return false;
  } catch (const std::exception& e) {
    pEnv->errOs << "[OS Exception] " << e.what() << endl;
    return false;
  }
  return true;
}
  

static vector<string> CMDHook_SEARCH_PATH(const vector<string>& new_search_path, Env * pEnv ) {
  pEnv->macroDB[MACRO_SEARCH_PATH] = new_search_path; // just update the copy in C++
  return new_search_path;
}

static string CMDHook_TMP_PATH(const string& new_path, Env * pEnv ) {
  string old_path = pEnv->macroDB[MACRO_TMP_PATH];
  if(new_path == old_path) return new_path; // same, nothing to do
  
  path tmp_old_path(old_path);
  path tmp_new_path(new_path);
  
  // create new path
  if(!cmd_create_tmp_path(tmp_new_path, pEnv))
    return old_path;

  // remove old path
  try {
    if(exists(tmp_old_path)) {
      remove_all(tmp_old_path);
    }
  } catch (std::exception& e) {
    pEnv->errOs << e.what() << std::endl;
    pEnv->errOs << "[Tcl] Error! Fail to remove the current temporary work directory\"" + old_path + "\"." << std::endl;
    remove_all(tmp_new_path);
    return old_path;
  }
  
  // run to here means ok
  pEnv->macroDB[MACRO_TMP_PATH] = new_path;
  return new_path;
}

static string CMDHook_CURRENT_DESIGN(const string& new_design, Env * pEnv ) {
  string old_design = pEnv->macroDB[MACRO_CURRENT_DESIGN];
  if(new_design == old_design) return new_design; // same, nothing to do

  // try to locate the module in all link libraries
  map<string, shared_ptr<netlist::Library> >::iterator it, end;
  shared_ptr<netlist::Module> tarModule;
  netlist::MIdentifier mName(new_design); // convert the string to MIdentifier first
  for(it=pEnv->link_lib.begin(), end=pEnv->link_lib.end(); it!=end; it++) {
    tarModule = it->second->find(mName);
    if(tarModule.use_count() != 0) break;
  }
  
  if(tarModule.use_count() == 0) { // fail to find the design
    pEnv->stdOs << "Error: target design \"" << new_design << "\" not found." << endl;
    return old_design;
  } else { //change the current design
    pEnv->curDgn = tarModule;
    pEnv->macroDB[MACRO_CURRENT_DESIGN] = new_design;
    return new_design;
  }
}

