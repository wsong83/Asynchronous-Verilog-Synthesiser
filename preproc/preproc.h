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

#ifndef AVS_PREPROC_H_
#define AVS_PREPROC_H_

// the preprocessor core
#include "VPreProc.h"
#include "VMacro.h"
#include <deque>
#include <list>

namespace VPPreProc {

  class VFileLineXs;
  
  //**********************************************************************
  // Preprocessor derived classes, so we can override the callbacks to call perl.
  
  class VPreProcXs : public VPreProc {
  public:
    //SV* m_self;	// Class called from (the hash, not SV pointing to the hash)
    std::deque<VFileLineXs*> m_filelineps;
    
  VPreProcXs() : VPreProc() {}
    virtual ~VPreProcXs();
    
    // Callback methods
    virtual void comment(std::string filename);	// Comment for keepComments=>sub
    virtual void include(std::string filename);	// Request a include file be processed
    virtual void define(std::string name, std::string value, std::string params); // `define with parameters
    virtual void undef(std::string name);		// Remove a definition
    virtual void undefineall();			// Remove all non-command-line definitions
    virtual bool defExists(std::string name);	// Return true if define exists
    virtual std::string defParams(std::string name);	// Return parameter list if define exists
    virtual std::string defValue(std::string name);	// Return value of given define (should exist)
    virtual std::string defSubstitute(std::string substitute);	// Return value to substitute for given post-parameter value
    
    void unreadback(char* text);
    void define(std::string name, std::string value, std::string params, bool pre); // predefine a macro
    void add_incr(const std::string& m_path);
    
  private:
    std::map<std::string, VMacro*>  m_macroDB;
    std::list<std::string>          m_incrList;
  };
  
  class VFileLineXs : public VFileLine {
    VPreProcXs*	m_vPreprocp;		// Parser handling the errors
  public:
    VFileLineXs(VPreProcXs* pp) : VFileLine(true), m_vPreprocp(pp) { if (pp) pushFl(); }
    virtual ~VFileLineXs() { }
    virtual VFileLine* create(const std::string& filename, int lineno) {
      VFileLineXs* filelp = new VFileLineXs(m_vPreprocp);
      filelp->init(filename, lineno);
      return filelp;
    }
    virtual void error(const std::string& msg);	// Report a error at given location
    void setPreproc(VPreProcXs* pp) {
      m_vPreprocp=pp;
      pushFl(); // The very first construction used pp=NULL, as pp wasn't created yet so make it now
    }
    // Record the structure so we can delete it later
    void pushFl() { m_vPreprocp->m_filelineps.push_back(this); }
  };
  
}

#endif
