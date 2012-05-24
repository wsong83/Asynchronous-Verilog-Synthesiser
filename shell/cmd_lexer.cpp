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

#include "env.h"
#include "cmd_lexer.h"

#define YYSTYPE shell::CMD::cmd_token_type
#undef BISON_LOCATION_HH
#undef BISON_POSITION_HH
#include "command.hh"
#undef YYSTYPE

#include<cstring>
#include<cctype>
#include<utility>

using std::pair;
using std::stack;
using std::deque;
using std::map;
using std::istream;
using std::cin;
using std::ifstream;
using boost::shared_ptr;
using namespace shell::CMD;

shell::CMD::CMDLexer::CMDLexer() 
  : lex_buf(new char[AV_CMD_LEXER_BUF_SIZE]),
    rp(lex_buf), fp(lex_buf)
{ 
  // build the token database
  // is there any better way to do this?
  // seems I need to modify this every time I add a new raw token
  tDB["analyze"]        = cmd_parser::token::CMDAnalyze;
  tDB["current_design"] = cmd_parser::token::CMDCurrentDesign;
  tDB["echo"]           = cmd_parser::token::CMDEcho;
  tDB["exit"]           = cmd_parser::token::CMDExit;
  tDB["help"]           = cmd_parser::token::CMDHelp;
  tDB["quit"]           = cmd_parser::token::CMDQuit;
  tDB["set"]            = cmd_parser::token::CMDSet;
  tDB["source"]         = cmd_parser::token::CMDSource;
  tDB["suppress_message"] = cmd_parser::token::CMDSuppressMessage;
  tDB["\n"]             = cmd_parser::token::CMD_END;
  tDB["simple_string"]  = cmd_parser::token::simple_string;

  // push a fifo into the tfifo stack
  tfstack.push(shared_ptr<deque<pair<int, cmd_token_type> > >(new deque<pair<int, cmd_token_type> >()));
}


shell::CMD::CMDLexer::~CMDLexer() {
  delete[] lex_buf;

  // clos all files that have not been closed
  while(fstack.size() != 0) {
    fstack.top()->close();
    delete fstack.top();
    fstack.pop();
  }

}

int shell::CMD::CMDLexer::yylex(cmd_token_type * yyval) {
 YYLEX_START:
  
  // return the token in the token stack if there is any
  // lexer only return token when it is sure that a whole line is read in
  // when \ is used to link multiple line or { is not matched
  // lexer should read in more line before return tokens
  // therefore the tokens read in previous read should be poped before read in new lines.
  if(current_tfifo().size() != 0) {
    int rv;
    rv = current_tfifo().front().first;
    *yyval = current_tfifo().front().second;
    current_tfifo().pop_front();
    //std::cout << "token: " << rv << ", " << yyval->tStr << std::endl;
    return rv;
  }
  
  // other wise read in lines for new tokens
  unsigned int level = 0;         // increase when [, (, or { is encountered 
  unsigned int level_str = 0;     // increase when [, (, or { inside a string is encountered 
  bool back_slash = false;        // true when a back slach is encountered
  bool quote = false;             // true when " is encountered
  bool comment = false;
  char tbuf[AV_CMD_LEXER_MAX_STRING_SIZE]; // token buffer for string
  unsigned int tp = 0;          // current pointer of the tbuf
  
  while(true) {
    if(rp == fp || *rp == '\000') { // empty
      
      if(rp != lex_buf) {
        *lex_buf = *(rp-1);     // for one back read
        rp = lex_buf + 1;
      }
      
      
      //make sure the current istream is readable
      while(!is_cin() && current_istream().eof()) {           // current file is finished
        pop();
      }
      
      // now current() must be a file with content or cin
      // read in a new line
      if(is_cin()) {
        gEnv->show_cmd(); // show the command line input sign;
      }
      current_istream().getline(rp, AV_CMD_LEXER_BUF_SIZE - 1);
      fp = rp + AV_CMD_LEXER_BUF_SIZE - 1;
    }
    
    // the lexer process
    while(rp != fp) {
      if(back_slash) {          // a back slash is encountered
        if(*rp != ' ' && *rp != '\000') { // not a blank or return, treat it as a special identifier
          tbuf[tp++] = *rp++;
        } else if(*(rp-1) != '\\' && *(rp-1) != ' ') { // a special string is read
          // push a token
          
          cmd_token_type mtoken;
          mtoken.tStr.assign(tbuf, tp);
          current_tfifo().push_back(pair<int,cmd_token_type> (tDB["simple_string"], mtoken));
          tp = 0;
          
          // clean status
          back_slash = false; // clear the back slash flag
          rp++;
        } else if(*rp == '\000') { // multi line encountered
          back_slash = false;
          tp = 0;
          break;
        } else {                // just blank
          rp++;
        }
      } else if(quote) {
        if(*rp != '\"') {
          tbuf[tp++] = *rp++;
        } else {
          cmd_token_type mtoken;
          mtoken.tStr.assign(tbuf, tp);
          current_tfifo().push_back(pair<int,cmd_token_type> (tDB["simple_string"], mtoken));
          tp = 0;
          quote = false;
          rp++;
        }
      } else if(comment) {
        if(*rp == '\000') {
          // push a return to the token fifo
          current_tfifo().push_back(pair<int,cmd_token_type> (tDB["\n"], cmd_token_type()));
          comment = false;
          if(level == 0) goto YYLEX_START;
          else break;
        } else {
          rp++;
        }
      } else {
        if(isalnum(*rp) || 
           (*rp == '$') || 
           (*rp == '/' && tp != 0) || 
           (*rp == '_') || 
           (*rp == '*' && tp != 0) ||
           (*rp == '-' && tp != 0) ||
           (*rp == '.') ||
           (*rp == '[' && tp != 0) ||
           (*rp == ']' && level_str != 0) ||
           (*rp == '{' && tp != 0 && tbuf[tp-1] == '$')||
           (*rp == '}' && level_str != 0)
           ) { // string
          tbuf[tp++] = *rp;
          
          // brackets inside a string
          if(*rp == '(' || *rp == '[' || *rp == '{')
            level_str++;
          else if(*rp == ')' || *rp == ']' || *rp == '}')
            level_str -= (level != 0 ? 1 : 0); // the extra ), ], } are ignored

          rp++;

        } else {
          if(tp != 0) {         // a string is read
            // push a token
            if( ( current_tfifo().size() == 0 || 
                  current_tfifo().back().first == tDB["\n"] || 
                  current_tfifo().back().first == '['
                )
                && tDB.find(string(tbuf, tp)) != tDB.end() 
              ) { // it is a command
              current_tfifo().push_back(pair<int,cmd_token_type> (tDB[string(tbuf, tp)], cmd_token_type()));
            } else {            // normal string
              cmd_token_type mtoken;
              mtoken.tStr.assign(tbuf, tp);
              current_tfifo().push_back(pair<int,cmd_token_type> (tDB["simple_string"], mtoken));
            }
            // clean the token buffer
            tp = 0;
          } // if(tp != 0)
          
          if(*rp != '\000') {
            if(*rp == '(' || *rp == '[' || *rp == '{') {
              level++;
              current_tfifo().push_back(pair<int,cmd_token_type> (*rp, cmd_token_type()));
            } else if(*rp == ')' || *rp == ']' || *rp == '}') {
              level -= (level != 0 ? 1 : 0); // the extra ), ], } are ignored
              current_tfifo().push_back(pair<int,cmd_token_type> (*rp, cmd_token_type()));
            } else if(*rp == '\"') {
              quote = true;
            } else if(*rp == '\\') {
              tbuf[tp++] = *rp;
              back_slash = true;
            } else if(*rp == '#') {
              comment = true;
            } else if(*rp != ' ')    // a mark token
              current_tfifo().push_back(pair<int,cmd_token_type> (*rp, cmd_token_type()));
            
            rp++;
          } else {                // '\n'
            current_tfifo().push_back(pair<int,cmd_token_type> (tDB["\n"], cmd_token_type()));
            
            if(level == 0) {      // a whole line is read
              goto YYLEX_START;
            } else {
              break;            // read more lines
            }
          }
        }
      }
    }
  }
}

void shell::CMD::CMDLexer::push(ifstream * fp) {
  fstack.push(fp);
  tfstack.push(shared_ptr<deque<pair<int, cmd_token_type> > >(new deque<pair<int, cmd_token_type> >()));
}

void shell::CMD::CMDLexer::pop() {
  if(fstack.size() != 0) {
    fstack.top()->close();
    delete fstack.top();
    fstack.pop();
    tfstack.pop();
  }
}

istream& shell::CMD::CMDLexer::current_istream() {
  if(fstack.size() == 0)
    return cin;
  else
    return *(fstack.top());
}

bool shell::CMD::CMDLexer::is_cin() const {
  return fstack.size() == 0;
}

void shell::CMD::CMDLexer::set_env(shell::Env * mEnv) {
  gEnv = mEnv;
}
