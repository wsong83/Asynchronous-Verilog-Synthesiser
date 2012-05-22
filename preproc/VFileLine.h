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
/// \brief Verilog::Preproc: Error handling
///
/// Authors: Wilson Snyder
///
/// Code available from: http://www.veripool.org/verilog-perl
///
//*************************************************************************

#ifndef _VFILELINE_H_
#define _VFILELINE_H_ 1
#include <string>
#include <iostream>
#include <sstream>

namespace VPPreProc {
//============================================================================
// VFileLine
/// User information and error reporting functions
////
/// Users can override this class to implement their own error handling

class VFileLine {
private:
    int		m_lineno;		///< Line number in file
    std::string	m_filename;		///< File name
    static int	s_numErrors;		///< Number of errors detected

protected:
    VFileLine(int called_only_for_default) {init("",0);}

public:
    // CONSTRUCTORS
    /// Create a new fileline, for a new file and/or line number.
    /// Member functions, so that if a user provides another class, a change in the
    /// filename/linenumber will create a new element using the derived class.
    virtual VFileLine* create(const std::string& filename, int lineno) = 0;
    /// Create with same filename, new line number; just calls create(fn,ln)
    virtual VFileLine* create(int lineno) { return create(filename(), lineno); }
    virtual void init(const std::string& filename, int lineno);
    virtual ~VFileLine() {}
    // ACCESSORS
    int lineno () const { return m_lineno; }	///< Return line number
    void linenoIncInPlace() { m_lineno++; }	///< Increment line IN PLACE; normally use create() instead
    const std::string filename () const { return m_filename; }	///< Return filename
    const std::string filebasename () const;	///< Filename with any directory stripped
    std::string lineDirectiveStrg(int enter_exit_level) const;
    // METHODS
    virtual void fatal(const std::string& msg);	///< Report a fatal error at given location
    virtual void error(const std::string& msg);	///< Report a error at given location
    VFileLine* lineDirective(const char* textp, int& enterExitRef);
    // STATIC METHODS
    static int numErrors() {return s_numErrors;}	///< Return total errors detected

    // Internal methods -- special use
    static const char* itoa(int i);	///< Internal: Not reentrant! - for fatalSrc() only
};
  std::ostream& operator<<(std::ostream& os, VFileLine* fileline);

/// Use this instead of fatal() to mention the source code line.
#define fatalSrc(msg) fatal((std::string)"Internal Error: "+__FILE__+":"+VFileLine::itoa(__LINE__)+": "+(msg))

template< class T> std::string cvtToStr (const T& t) {
  std::ostringstream os; os<<t; return os.str();
}

}

#endif // Guard
