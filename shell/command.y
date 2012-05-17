// -*- Bison -*-
%skeleton "lalr1.cc"
%defines
%define namespace "shell::CMD"
%define parser_class_name "cmd_parser"
%language "c++"
%output "command.cc"
%parse-param {shell::Env* cmd_env}
%{
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
 * Bison grammer file for shell command lines
 * 10/05/2012   Wei Song
 *
 *
 */

#include <string>
#include <iostream>
  using std::string;
  using std::endl;

#include "env.h"
  //make sure the location of command.y is included
#undef BISON_LOCATION_HH
#undef BISON_POSITION_HH
 
// as it is very simple, lex is not used
#define yylex cmd_env->lexer.yylex
  
#define YYSTYPE shell::CMD::cmd_token_type

#include "cmd_token.h"

%}


%initial-action
{

}


///////////////////////////////////////////////////
// token definitions

// operators

 // command named
%token CMDAnalyze              "analyze"
%token CMDHelp                 "help"
%token CMDSource               "source"
%token CMDExit                 "exit"
%token CMDQuit                 "quit"

 // other
 /* the final return of a command line */
%token         CMD_END

%token<tStr>   simple_string


 // type definitions
%type <tStr>       argument_name
%type <tStr>       complex_string
%type <tStrVec>    argument
%type <tStrVec>    argument_list
%type <tStrVec>    argument_parameter
%type <tStrVec>    string_list


%start command_text

%%

command_text 
    : /* empty */ 
    | command_list
    ;

command_list
    : command_description CMD_END
    | command_list command_description CMD_END
    ;

command_description
    : /* empty */
    | "analyze" argument_list     { shell::CMD::CMDAnalyze::exec(*cmd_env, $2);            }
    | "exit"    argument_list     { if(shell::CMD::CMDQuit::exec(*cmd_env, $2)) YYACCEPT;  }
    | "help"    argument_list     { shell::CMD::CMDHelp::exec(*cmd_env, $2);               }
    | "quit"    argument_list     { if(shell::CMD::CMDQuit::exec(*cmd_env, $2)) YYACCEPT;  }
    | "source"  argument_list     { shell::CMD::CMDSource::exec(*cmd_env, $2);             }
    | simple_string argument_list 
    {
      cmd_env->errOs << "Unrecognizable command \"" << $1 << "\"!" << endl;
    }
    | error
    {
      cmd_env->errOs << "Unrecognizable patterns!" << endl;
    }
    ;

argument_list
    : /* empty */                 { $$.clear();                                }
    | argument                    
    | argument_list argument      { $$.insert($$.end(), $2.begin(), $2.end()); }
    ;

argument
    : argument_parameter                        
    | '-' argument_name                             { $$.push_back(string("-") + $2);  }
    | '-' argument_name argument_parameter      
    { 
      $$.push_back(string("--") + $2);
      $$.insert($$.end(), $3.begin(), $3.end());
    }
    | '-' '-' argument_name                         { $$.push_back(string("-") + $3);  }
    | '-' '-' argument_name argument_parameter 
    { 
      $$.push_back(string("-") + $3);
      $$.insert($$.end(), $4.begin(), $4.end());
    }
    ;

argument_name
    : simple_string
    ;

argument_parameter
    : complex_string                { $$.push_back($1); }
    | '{' string_list '}'           { $$ = $2;          }
    ;

string_list
    : /* empty */                   { $$.clear();        }
    | complex_string                { $$.push_back($1); }
    | string_list complex_string    { $$.push_back($2); }
    ;

complex_string
    : simple_string                 
    ;

%%
      
    void shell::CMD::cmd_parser::error (const shell::CMD::cmd_parser::location_type& loc, const string& msg) {
      //av_env.error(loc, "PARSER-0");
      //cout << msg << endl;
    }
      
