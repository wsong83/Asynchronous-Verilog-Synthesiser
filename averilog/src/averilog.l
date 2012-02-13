%option reentrant bison-bridge bison-locations
%option never-interactive
%option noyywrap nounput
%option yylineno
%option prefix="av"
%option outfile="averilog.lex.c" header-file="averilog.lex.h"

%{
/*
 * Copyright (c) 2011 Wei Song <songw@cs.man.ac.uk> 
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
 * Flex grammer file for Asynchronous Verilog
 * 29/01/2012   Wei Song
 *
 *
 */
 
#include <cstdio>
#include <iostream>
#include "averilog_util.h"

/* handle locations */
#define YY_USER_ACTION yylloc->step(); yylloc->columns(yyleng);

%}

ws          [ \t\b\f\r]+
dig_d       [0-9]
dig_h       [a-fxzA-FXZ0-9]
dec         {dig_d}({dig_d}*_?)*
fnum        {dec}?\'[bdohBDOH]{dig_h}({dig_h}*_?)*
sid         [a-zA-Z_][a-zA-Z0-9_$]*
escid	    \\[^ \t\f\r\n]+
identifier  {sid}|{escid}
operator    [}{;:\[\],()#=@&!?<>|^~+*/-]

%x          COMMENT

%%

      
"**"        { return token::oPower; }
">="        { return token::oGe;    }
"<="        { return token::oLe;    }
"&&"        { return token::oLAnd;  }
"||"        { return token::oLOr;   }
"=="        { return token::oEq;    }
"==="       { return token::oCEq;   }
"!="        { return token::oNeq;   }
"!=="       { return token::oCNeq;  }
"<<"        { return token::oLLsh;  }
">>"        { return token::oLRsh;  }
"<<<"       { return token::oLLsh; /* equivalent to << */ }
">>>"       { return token::oARsh;  }
"-:"        { return token::oNColon; }
"+:"        { return token::oPColon; }
"~&"        { return token::oRNand; }
"~|"        { return token::oRNor;  }
"~^"        { return token::oRXnor; }
"^~"        { return token::oRXnor; /* equivalent to ~^  */ }

"always"    { return token::kAlways; }
"assign"    { return token::kAssign; }
"automatic" { return token::kAutomatic; }    
"begin"     { return token::kBegin; }
"case"      { return token::kCase;  }
"else"      { return token::kElse;  }
"end"       { return token::kEnd;   }
"endcase"   { return token::kEndcase; }
"endfunction"  { return token::kEndfunction; }
"endgenerate"  { return token::kEndgenerate; }
"endmodule" { return token::kEndmodule; }
"for"       { return token::kFor;   }
"function"  { return token::kFunction; }
"generate"  { return token::kGenerate; }
"genvar"    { return token::kGenvar; }
"if"        { return token::kIf;    }
"input"     { return token::kInput; }
"integer"   { return token::kInteger; }
"module"    { return token::kModule; }
"negedge"   { return token::kNegedge; }
"output"    { return token::kOutput; }
"parameter" { return token::kParameter; }
"posedge"   { return token::kPosedge; }
"reg"       { return token::kReg;   }
"while"     { return token::kWhile; }
"wire"      { return token::kWire;  }
"and"       { return token::pAnd;   }
"or"        { return token::pOr;    }
"not"       { return token::pNot;   }
"nand"      { return token::pNand;  }
"nor"       { return token::pNor;   }
"xor"       { return token::pXor;   }
"xnor"      { return token::pXnor;  }
"buf"       { return token::pBuf;   }
{operator}  { return yytext[0]; }
{fnum}    { 
              yylval->tNumber = new netlist::Number(yytext, yyleng);
              if(!yylval->tNumber->is_valid()) {
                error_report("Wrong number format.", yylloc);
                yyterminate();
              }
              return token::number; 
            }
{dec}    { 
              yylval->tNumber = new netlist::Number(yytext, yyleng);
              return token::number; 
            }
{identifier}  { 
                yylval->tID = new averilog::avID(yytext, yyleng);
                return token::identifier; 
              }
"//"[^\n]*[\n] { yylloc->lines();    }
"/*"        { BEGIN(COMMENT);        }
<COMMENT>[^\n] { /* do nothing */    }
<COMMENT><<EOF>> { error_report("EOF in multi-line comment.", yylloc); yyterminate(); }
<COMMENT>"\n"  { yylloc->lines();    }
"*/"        { BEGIN(0);              } 
{ws}        { /* white space, do nothing */ }
"\n"        { yylloc->lines();       }

%%
