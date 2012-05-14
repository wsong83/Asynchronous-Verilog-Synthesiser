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
 * Lexer for command line parser
 * 11/05/2012   Wei Song
 *
 *
 */

#ifndef AV_CMD_LEXER_
#define AV_CMD_LEXER_

#include<stack>
using std::stack;

#include<map>
using std::map;
using std::pair;

#include<fstream>
#include<iostream>
using std::istream;
using std::cin;
using std::ifstream;

#include "cmd_token.h"

// the initial size of the line buffer
#define AV_CMD_LEXER_BUF_SIZE 512

// the maximal size of a single simple string
#define AV_CMD_LEXER_MAX_STRING_SIZE 128

namespace shell { 
  namespace CMD {

    struct cmd_token_type;
  
    class CMDLexer {
    public:
      CMDLexer();               /* constructor */
      virtual ~CMDLexer();      /* destructor */

      // helpers
      int yylex (cmd_token_type *); /* the interface with the Bison parser */
      void push (ifstream *);        /* source another file */
      void pop();                    /* pop the current istream when finished */
      void set_env(Env *);           /* set the env pointer */

    private:
      
      istream& current();       /* return the current stream */
      bool is_cin() const;      /* true if the current stream is cin */
      
      Env* gEnv;
      stack<ifstream *> fstack;
      char* lex_buf;
      char *rp, *fp;      /* read pointer and full position */
      stack<pair<int, cmd_token_type> > tstack;
      map<string, int> tDB;    /* token database */

    };
  }
}

#endif
