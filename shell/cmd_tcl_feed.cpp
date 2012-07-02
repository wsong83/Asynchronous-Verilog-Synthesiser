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
 * Input feeder for the Tcl command line environment
 * 02/07/2012   Wei Song
 *
 *
 */

#include "cmd_tcl_feed.h"

#include<cstring>
#include<cctype>
#include<utility>

using std::stack;
using std::istream;
using std::cin;
using std::ifstream;
using boost::shared_ptr;
using namespace shell::CMD;

// the initial size of the line buffer
#define AV_CMD_LEXER_BUF_SIZE 512

shell::CMD::CMDTclFeed::CMDTclFeed()
 : lex_buf(new char[AV_CMD_LEXER_BUF_SIZE]),
   rp(lex_buf), fp(lex_buf)
{}

shell::CMD::CMDTclFeed::~CMDTclFeed() {
  delete[] lex_buf;
  rp = NULL;
  fp = NULL;
  if(file_stream != NULL) {
    file_stream->close();
    file_stream = NULL;
  }
}

istream& shell::CMD::CMDTclFeed::cstream() {
  if(file_stream != NULL) {
    if(file_stream->eof()) {    // end-of-a-file
      file_stream->close();
      file_stream = NULL;
      return cin;
    } else {
      return *file_stream;
    }
  } else
    return cin;
}
