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
 * A wrapper of the Verilog-PreProcessor of Verilog Perl tool 3.314
 * 27/04/2012   Wei Song
 *
 *
 */

#include "preproc.h"

#include <boost/filesystem.hpp>
using namespace boost::filesystem;

using namespace VPPreProc;

#//**********************************************************************
#// Overrides error handling virtual functions to invoke callbacks

void VPPreProc::VFileLineXs::error(const string& msg) {
  cout << msg << endl;
}

#//**********************************************************************
#// VPreProcXs functions

VPPreProc::VPreProcXs::~VPreProcXs() {
    for (deque<VFileLineXs*>::iterator it=m_filelineps.begin(); it!=m_filelineps.end(); ++it) {
	delete *it;
    }
    
    // delete the macro database
    for (map<string, VMacro*>::iterator it=m_macroDB.begin(); it!=m_macroDB.end(); ++it) {
      delete it->second;
    }
}

#//**********************************************************************
#// Overrides of virtual functions to invoke callbacks

void VPPreProc::VPreProcXs::comment(string cmt) {
  // nothing
}

void VPPreProc::VPreProcXs::include(string filename) {
  path p(filename);
  if(exists(p)) {
    openFile(p.string());
    return;
  }

  list<string>::iterator it, end;
  for(it=m_incrList.begin(), end=m_incrList.end(); it!=end; it++) {
    path libp(*it + "/" + filename);
    if(exists(libp)) {
      openFile(libp.string());
      return;
    }
  }

  // fail to open any file
  return;

}
void VPPreProc::VPreProcXs::undef(string define) {
  map<string, VMacro*>::iterator it = m_macroDB.find(define);
  if(it != m_macroDB.end()) {
    delete it->second;
    m_macroDB.erase(it);
  }
}
void VPPreProc::VPreProcXs::undefineall() {
    // delete the macro database
    for (map<string, VMacro*>::iterator it=m_macroDB.begin(); it!=m_macroDB.end(); ++it) {
      if(!it->second->m_pre) {    // only delete macros defined in files
        delete it->second;
        m_macroDB.erase(it);
      }
    }  
}
void VPPreProc::VPreProcXs::define(string define, string value, string params) {
  this->define(define, value, params, false);
}
void VPPreProc::VPreProcXs::define(string define, string value, string params, bool pre) {
  m_macroDB.insert(pair<string, VMacro*>(define, new VMacro(define, value, params, pre)));
}
bool VPPreProc::VPreProcXs::defExists(string define) {
  return m_macroDB.find(define) != m_macroDB.end();
}
string VPPreProc::VPreProcXs::defParams(string define) {
  map<string, VMacro*>::iterator it = m_macroDB.find(define);
  
  if(it != m_macroDB.end()) 
    return it->second->m_para == "" ? "0" : it->second->m_para;
  else 
    return "";
}
string VPPreProc::VPreProcXs::defValue(string define) {
  return m_macroDB.find(define)->second->m_value;
}
string VPPreProc::VPreProcXs::defSubstitute(string subs) {
    return subs;
}

void VPPreProc::VPreProcXs::add_incr(const string& m_path) {
  m_incrList.push_back(m_path);
}
