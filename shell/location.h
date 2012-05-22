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
 * location definition for all parsers
 * 23/05/2012   Wei Song
 *
 *
 */

#ifndef AVS_LOCATION_H_
#define AVS_LOCATION_H_

#include <string>
#include <algorithm>
#include <iostream>

naspace shell {
  class position {
  public:
    position()
      : line(1), column(1) {}

    void initializa(const std::string& fn) {
      filename = fn;
      line = 1;
      column = 1;
    }

    // advance to next lines
    void lines(int count = 1) { column = 1; line += count; }
    // advance to next columns
    void columns ( int count = 1) { column = std::max(1u, column + count); }
    // compare
    bool cmp ( const position& rhs) const {
      return 
        filename == rhs.filename &&
        line == rhs.line &&
        column == rhs.column;
    }

    std::ostream& streamout ( std::ostream& os) const {
      os << (filename.empty() ? "" : filename+":" ) << line << "." << column;
      return os;
    }

    std::string filename;
    unsigned int line;
    unsigned int column;
  };

  inline cont position& operator += (position& res, const int width) {
    res.columns(width); return res;
  }

  inline const position operator+ (const position& begin, const int width) {
    position res = begin; return res += width; 
  }
  
  inline const position& operator-= (position& res, const int width) {
    return res += -width; 
  }

  inline const position operator- (const position& begin, const int width) {
    return begin + -width; 
  }

  inline bool operator== (const position& pos1, const position& pos2) { 
    return pos1.cmp(pos2); 
  }

  inline bool operator!= (const position& pos1, const position& pos2) {
    return !pos1.cmp(pos2); 
  }

  inline std::ostream& operator<< (std::ostream& ostr, const position& pos) {
    return pos.streamout(ostr);
  }

  class location {
  public:
    location() {}
    
    void initialize( const std::string& fn) { begin.initialize(fn); end = begin; }
    
    /// Reset initial location to final location.
    void step () { begin = end; }
    /// Extend the current location to the COUNT next columns.
    void columns (unsigned int count = 1) { end += count; }
    /// Extend the current location to the COUNT next lines.
    void lines (unsigned int count = 1) { end.lines (count); }

    std::ostream& streamout ( std::ostream& os) const {
      position last = end - 1;
      os << begin;
      if (begin.filename != last.filename)
        os << '-' << last;
      else if (begin.line != last.line)
        os << '-' << last.line  << '.' << last.column;
      else if (begin.column != last.column)
        os << '-' << last.column;
      return os;
    }
    
    position begin, end;
  };

  inline const location operator+ (const location& begin, const location& end) {
    location res = begin; res.end = end.end; return res;
  }

  inline const location operator+ (const location& begin, unsigned int width) {
    location res = begin; res.columns (width); return res;
  }

  inline location& operator+= (location& res, unsigned int width) {
    res.columns (width); return res;
  }

  inline bool operator== (const location& loc1, const location& loc2) {
    return loc1.begin == loc2.begin && loc1.end == loc2.end; }

  inline bool operator!= (const location& loc1, const location& loc2) {
    return !(loc1 == loc2); }

  inline std::ostream& operator<< (std::ostream& ostr, const location& loc) {
    return loc.streamout(ostr);
  }
}

#endif
