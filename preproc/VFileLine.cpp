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
 * Embed the Verilog-Perl verilog preprocessor to AVS
 * 22/05/2012   Wei Song
 *
 *
 */
// -*- C++ -*-
//*************************************************************************
//
// Copyright 2000-2012 by Wilson Snyder.  This program is free software;
// you can redistribute it and/or modify it under the terms of either the GNU
// Lesser General Public License Version 3 or the Perl Artistic License Version 2.0.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//*************************************************************************
/// \file
/// \brief Verilog::Preproc: Error handling implementation
///
/// Authors: Wilson Snyder
///
/// Code available from: http://www.veripool.org/verilog-perl
///
//*************************************************************************

#include <cstdio>
#include <cstdlib>

#include "VFileLine.h"

using namespace std;
using namespace VPPreProc;

int VPPreProc::VFileLine::s_numErrors = 0;		///< Number of errors detected

//============================================================================

void VPPreProc::VFileLine::init(const string& filename, int lineno) {
    m_filename = filename;
    m_lineno = lineno;
}

const string VPPreProc::VFileLine::filebasename () const {
    string name = filename();
    string::size_type slash;
    if ((slash = name.rfind("/")) != string::npos) {
	name.erase(0,slash+1);
    }
    return name;
}

void VPPreProc::VFileLine::fatal(const string& msg) {
    error(msg);
    error("Fatal Error detected");
    abort();
}
void VPPreProc::VFileLine::error(const string& msg) {
    VFileLine::s_numErrors++;
    if (msg[msg.length()-1] != '\n') {
	fprintf (stderr, "%%Error: %s", msg.c_str());
    } else {
	fprintf (stderr, "%%Error: %s\n", msg.c_str());	// Append newline, as user omitted it.
    }
}

const char* VPPreProc::VFileLine::itoa(int i) {
    static char buf[100];
    sprintf(buf,"%d",i);
    return buf;
}

string VPPreProc::VFileLine::lineDirectiveStrg(int enterExit) const {
    char numbuf[20]; sprintf(numbuf, "%d", lineno());
    char levelbuf[20]; sprintf(levelbuf, "%d", enterExit);
    return ((string)"`line "+numbuf+" \""+filename()+"\" "+levelbuf+"\n");
}

VFileLine* VPPreProc::VFileLine::lineDirective(const char* textp, int& enterExitRef) {
    // Handle `line directive
    // Skip `line
    while (*textp && isspace(*textp)) textp++;
    while (*textp && !isspace(*textp)) textp++;
    while (*textp && (isspace(*textp) || *textp=='"')) textp++;

    // Grab linenumber
    int lineno = this->lineno();
    const char *ln = textp;
    while (*textp && !isspace(*textp)) textp++;
    if (isdigit(*ln)) {
	lineno = atoi(ln);
    }
    while (*textp && (isspace(*textp) || *textp=='"')) textp++;

    // Grab filename
    string filename = this->filename();
    const char* fn = textp;
    while (*textp && !(isspace(*textp) || *textp=='"')) textp++;
    if (textp != fn) {
	string strfn = fn;
	strfn = strfn.substr(0, textp-fn);
	filename = strfn;
    }

    // Grab level
    while (*textp && (isspace(*textp) || *textp=='"')) textp++;
    if (isdigit(*textp)) enterExitRef = atoi(textp);
    else enterExitRef = 0;

    return create(filename,lineno);
}

//======================================================================
// Global scope

ostream& VPPreProc::operator<<(ostream& os, VFileLine* flp) {
    if (flp->filename()!="") {
	os <<flp->filename()<<":"<<dec<<flp->lineno()<<": "<<hex;
    }
    return(os);
}
