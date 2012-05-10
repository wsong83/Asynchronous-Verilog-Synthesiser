// -*- Bison -*-
%skeleton "lalr1.cc"
%defines
%define namespace "shell::CMD"
%define parser_class_name "cmd_parser"
%language "c++"
%output "command.cc"
%parse-param {shell::Env& cmd_env}
%debug
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

#include "shell_top.h"
#include "cmd/cmd_define.h"
  
  // as it is very simple, lex is not used
#define yylex cmd_readin;
  
#define YYSTYPE cmd_token_type

  struct cmd_token_type {
    string          tStr;
    vector<string>  tStrVec;
  };

  
  
%}


%initial-action
{

}


///////////////////////////////////////////////////
// token definitions

// operators

 // command named
%token CMDAnalyze              "analyze"
%token CMDSource               "source"

 // other
 /* the final return of a command line */
%token         CMD_END

%token<tStr>   simple_string


 // type definitions
%type <tStr>       argument_name
%type <tStr>       complex_string
%type <tStr>       identifier
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
    : command_description
    | command_list command_description
    ;

command_description
    : "analyze" argument_list CMD_END
    | "source"  argument_list CMD_END
    | error
    ;

argument_list
    : /* empty */                 { $$.clear();                                }
    | argument                    
    | argument_list argument      { $$.insert($$.end(), $2.begin(), $2.end()); }
    ;

argument
    : identifier                                { $$.push_back($1);                }
    | '-' argument_name                         { $$.push_back(string("--") + $2); }
    | '-' argument_name argument_parameter 
    { 
      $$.push_back(string("--") + $2);
      $$.insert($$.end(), $3.begin(), $3.end());
    }
    ;

identifier
    : complex_string
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
