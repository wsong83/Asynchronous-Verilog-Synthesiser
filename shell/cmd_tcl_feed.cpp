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
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
using namespace boost::filesystem;

using std::string;
using std::stack;
using std::istream;
using std::cin;
using std::endl;
using namespace shell::CMD;

// the initial size of the line buffer
#define AV_CMD_LEXER_BUF_SIZE 512

shell::CMD::CMDTclFeed::CMDTclFeed()
 : lex_buf(new char[AV_CMD_LEXER_BUF_SIZE]),
   rp(0), fp(0), bufsize(AV_CMD_LEXER_BUF_SIZE), file_stream(NULL), gEnv(NULL)
{}

shell::CMD::CMDTclFeed::~CMDTclFeed() {
  delete[] lex_buf;
  if(file_stream != NULL) {
    file_stream->close();
    file_stream = NULL;
  }
}

bool shell::CMD::CMDTclFeed::initialise(Env * mg_env, const std::string& fn) {
  // set the environment
  if(mg_env == NULL) return false;
  gEnv = mg_env;

  // set the initial file
  if(fn.empty()) return true;   // no initial file

  // check the file
  if(!exists(fn) || !is_regular_file(fn)) { // cannot open the file, error
    gEnv->stdOs << "Error: Cannot open script file \"" << fn << "\"!" << endl;
    return false;
  }

  // open the file
  file_stream = new std::ifstream(fn.c_str());
  if(!file_stream->good()) {   // unkown error, should not going here
    gEnv->stdOs << "Error: Cannot open script file \"" << fn << "\"!" << endl;
    return false;
  } else
    return true;
}

string shell::CMD::CMDTclFeed::getline() {
  if(rp == fp) {                // empty
    rp = 0;
    getline_priv();
  }

  unsigned int start = rp;      // record the start point
  int bracket_count = 0;
  bool quote = false;
  bool backslash = false;
  // record the position of backslash as it may be rewritten to blank
  unsigned int backslash_pos = 0;
  bool comment = false;

  while(true) {
    // analyse the buffer
    while(rp != fp) {
      if(backslash && quote) {  // backslash in quote is special char
        rp++; backslash = false;
        continue;
      }
      
      if(quote) {               // quote
        if(lex_buf[rp++] == '\"') quote = false;
        continue;
      }

      if(comment && lex_buf[rp] != '\n') { // comment
        lex_buf[rp++] = ' ';
        continue;
      }

      if(lex_buf[rp] == '[' || lex_buf[rp] == '(' || lex_buf[rp] == '{')
        bracket_count++;

      if(lex_buf[rp] == ']' || lex_buf[rp] == ')' || lex_buf[rp] == '}')
        bracket_count--;
      
      if(lex_buf[rp] == '\\') {
        backslash = true;
        backslash_pos = rp;
      }

      if(lex_buf[rp] == '#') {
        lex_buf[rp++] = ' '; comment = true;
      }

      if(lex_buf[rp] != ' ') backslash = false;

      if(lex_buf[rp] == '\"') quote = true;

      if(lex_buf[rp] == '\n') {
        if(backslash) {         // using backslash to continue a line
          lex_buf[backslash_pos] = ' ';
          rp++;
          continue;
        } else if(bracket_count > 0) { // bracket not even yet
          lex_buf[rp++] = ' ';
          continue;
        } else {
          // should return a string
          lex_buf[rp++] = ' ';
          return string(lex_buf + start, rp - start);
        }
      }

      rp++;
    }
    // read more
    getline_priv();    
  }
}

void shell::CMD::CMDTclFeed::getline_priv() {
  bool m_cin;
  istream& m_istm = cstream(m_cin);
  if(m_cin) gEnv->show_cmd();
  check_buf_size();
  m_istm.getline(lex_buf+rp, (AV_CMD_LEXER_BUF_SIZE >> 1) - 1);
  fp = rp + m_istm.gcount();    // record where buffer end
  lex_buf[fp++] = '\n';         // add a \n as it is removed by getline()
}

istream& shell::CMD::CMDTclFeed::cstream(bool& m_cin) {
  if(file_stream != NULL) {
    if(file_stream->eof()) {    // end-of-a-file
      file_stream->close();
      file_stream = NULL;
      m_cin = true;
      return cin;
    } else {
      m_cin = false;
      return *file_stream;
    }
  } else {
    m_cin = true;
    return cin;
  }
}

void shell::CMD::CMDTclFeed::check_buf_size() {
  if(bufsize - rp < (AV_CMD_LEXER_BUF_SIZE >> 1)) { // need increase the size
    char* origin_buf = lex_buf;
    lex_buf = new char[bufsize + AV_CMD_LEXER_BUF_SIZE];
    memcpy(lex_buf, origin_buf, sizeof(char)*bufsize);
    bufsize += AV_CMD_LEXER_BUF_SIZE;
    delete[] origin_buf;

    std::cout << "lex_buf size increased to " << bufsize << std::endl;

  }
}
