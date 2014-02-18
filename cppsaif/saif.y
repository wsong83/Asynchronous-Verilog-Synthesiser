// -*- Bison -*-
%skeleton "lalr1.cc"
%defines
%define namespace "saif"
%define parser_class_name "saif_parser"
%language "c++"
%output "saif.cc"
%parse-param {saif::SaifLexer* lexer}
%parse-param {saif::SaifDB* db}
%{

/*
 * Copyright (c) 2014-2014 Wei Song <songw@cs.man.ac.uk> 
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
 * Bison grammer file for Saif file
 * 06/02/2014   Wei Song
 *
 *
 */

#include "saif_util.hpp"
#include "saif_db.hpp"

#define yylex lexer->lexer

  using namespace saif;

%}


%initial-action
{
}


///////////////////////////////////////////////////
// token definitions

%token SKeySAIFILE                  "SAIFILE"
%token SKeySAIFVERSION              "SAIFVERSION"
%token SKeyDIRECTION                "DIRECTION"
%token SKeyDESIGN                   "DESIGN"
%token SKeyDATE                     "DATE"
%token SKeyVENDOR                   "VENDOR"
%token SKeyPROGRAM_NAME             "PROGRAM_NAME"
%token SKeyVERSION                  "VERSION"
%token SKeyDIVIDER                  "DIVIDER"
%token SKeyTIMESCALE                "TIMESCALE"
%token SKeyDURATION                 "DURATION"
%token SKeyINSTANCE                 "INSTANCE"
%token SKeyPORT                     "PORT"
%token SKeyNET                      "NET"
%token SKeyT0                       "T0"
%token SKeyT1                       "T1"
%token SKeyTZ                       "TZ"
%token SKeyTX                       "TX"
%token SKeyTC                       "TC"
%token SKeyIG                       "IG"
%token SKeyTB                       "TB"

%token<tStr>       SString
%token<tVar>       SVar
%token<tNum>       SNum

%type <tAct>       activity
%type <tRecord>    activities
%type <tSig>       signal
%type <tSigList>   signal_lists
%type <tSigList>   port_list
%type <tInst>      instance_contents
%type <tInstPair>  saif_instance
%type <tInstList>  saif_instances

// the saif file

%start saif_file


%%

saif_file
    : '(' "SAIFILE" saif_contents ')'
    ;

saif_contents
    : saif_line
    | saif_contents saif_line
    ;

saif_line
    : '(' "SAIFVERSION"  SString ')'   { db->version      = $3; }
    | '(' "DIRECTION"    SString ')'   { db->direction    = $3; }
    | '(' "DESIGN"')'               
    | '(' "DATE"         SString ')'   { db->date         = $3; }
    | '(' "VENDOR"       SString ')'   { db->vendor       = $3; }
    | '(' "PROGRAM_NAME" SString ')'   { db->program_name = $3; }
    | '(' "VERSION"      SString ')'   { db->tool_version = $3; }
    | '(' "DIVIDER"      SString ')'   { db->divider      = $3; }
    | '(' "TIMESCALE"    SNum SVar ')'    
    { 
      db->timescale    = std::pair<mpz_class, std::string>($3, $4); 
    }
    | '(' "DURATION"     SNum    ')'   { db->duration = $3;     }
    | saif_instance
    { 
      db->top = $1.second;
      db->top_name = $1.first;
    }
    ;

saif_instances
    : saif_instance                      { $$[$1.first] = $1.second; }
    | saif_instances saif_instance       { $$[$2.first] = $2.second; }
    ;

saif_instance
    : '(' "INSTANCE" SString SVar instance_contents ')'
    {
      $$.first = $4;
      $$.second = $5;
      $$.second->module_name = $3;
      //std::cout << $$.first << " " << *($$.second) << std::endl;
    }
    | '(' "INSTANCE" SVar instance_contents ')'
    {
      $$.first = $3;
      $$.second = $4;
      //std::cout << $$.first << " " << *($$.second) << std::endl;
    }
    ;

instance_contents
    : port_list
    { 
      $$.reset(new saif::SaifInstance());
      $$->signals = $1;
    }
    | port_list saif_instances
    {
      $$.reset(new saif::SaifInstance());
      $$->signals = $1;
      $$->instances = $2;
    }
    | saif_instances
    {
      $$.reset(new saif::SaifInstance());
      $$->instances = $1;
    }
    ;
      

port_list
    : '(' "PORT" signal_lists ')' { $$ = $3; }
    | '(' "NET" signal_lists ')'  { $$ = $3; }
    ;

signal_lists
    : signal               
    { 
      // parse the signal name
      std::list<int> dim;
      std::string sig_name = signal_name_parser(signal_name_normalizer($1.first), dim);
      
      if(!$$.count(sig_name))
        $$[sig_name].reset(new saif::SaifSignal()); 

      boost::shared_ptr<saif::SaifSignal> sig = $$[sig_name];
      while(!dim.empty()) {
        if(!sig->bits.count(dim.front()))
          sig->bits[dim.front()].reset(new saif::SaifSignal());
        sig = sig->bits[dim.front()];
        dim.pop_front();
      }
      
      // insert data
      sig->data = $1.second->data;
    }
    | signal_lists signal  
    { 
      // parse the signal name
      std::list<int> dim;
      std::string sig_name = signal_name_parser(signal_name_normalizer($2.first), dim);
      
      if(!$$.count(sig_name))
        $$[sig_name].reset(new saif::SaifSignal()); 

      boost::shared_ptr<saif::SaifSignal> sig = $$[sig_name];
      while(!dim.empty()) {
        if(!sig->bits.count(dim.front()))
          sig->bits[dim.front()].reset(new saif::SaifSignal());
        sig = sig->bits[dim.front()];
        dim.pop_front();
      }
      
      // insert data
      sig->data = $2.second->data;
    }
    ;

signal
    : '(' SVar activities ')'
    {
      $$.first = $2;
      $$.second.reset(new saif::SaifSignal());
      $$.second->data = $3;
    }
    ;

activities
    : activity
    { 
      $$.reset(new saif::SaifRecord());
      switch($1.first) {
      case 0:  $$->T0 = $1.second; break;
      case 1:  $$->T1 = $1.second; break;
      case 2:  $$->TX = $1.second; break;
      case 3:  $$->TZ = $1.second; break;
      case 4:  $$->TC = $1.second; break;
      case 5:  $$->IG = $1.second; break;
      default:
        assert(0 == "should not run to this");
      }
    }
    | activities activity
    {
      switch($2.first) {
      case 0:  $$->T0 = $2.second; break;
      case 1:  $$->T1 = $2.second; break;
      case 2:  $$->TX = $2.second; break;
      case 3:  $$->TZ = $2.second; break;
      case 4:  $$->TC = $2.second; break;
      case 5:  $$->IG = $2.second; break;
      case 6:  $$->TB = $2.second; break;
      default:
        assert(0 == "should not run to this");
      }
    }
    ;

activity
    : '(' "T0" SNum ')'     { $$.first = 0; $$.second = $3; }
    | '(' "T1" SNum ')'     { $$.first = 1; $$.second = $3; }
    | '(' "TX" SNum ')'     { $$.first = 2; $$.second = $3; }
    | '(' "TZ" SNum ')'     { $$.first = 3; $$.second = $3; }
    | '(' "TC" SNum ')'     { $$.first = 4; $$.second = $3; }
    | '(' "IG" SNum ')'     { $$.first = 5; $$.second = $3; }
    | '(' "TB" SNum ')'     { $$.first = 6; $$.second = $3; }
    ;

%%

void saif::saif_parser::error (const saif::location& loc, const std::string& msg) {
  std::cout << loc << ":" << msg << std::endl;
  }
