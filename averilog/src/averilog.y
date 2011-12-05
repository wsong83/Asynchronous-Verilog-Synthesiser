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
 * Bison grammer file for Asynchronous Verilog
 * Some grammers are coming from Verilator 3.824 http://www.veripool.org/verilator
 * 02/12/2011   Wei Song
 *
 *
 */

%{
#include <cstdio>
#include <cstdlib>


%}

%union {
  int avOp;			/* operator, dummy value, not suppose to be used */
  int avKW;			/* keyword, dummy value, not suppose to be used */
  int avPri;			/* Primitive gate, dummy value, not suppose to be used */
}


///////////////////////////////////////////////////
// token definitions

// operators
%token <avOp>                  '{'
%token <avOp>                  '}'
%token <avOp>                  '+'
%token <avOp>                  '-'
%token <avOp>                  '*'
%token <avOp>                  '/'
%token <avOp>   avOpPower      "**"
%token <avOp>                  '%'
%token <avOp>                  '>'
%token <avOp>                  '<'
%token <avOp>   avOpGe         ">="
%token <avOp>   avOpLe         "<="
%token <avOp>                  '!'
%token <avOp>   avOpLAnd       "&&"
%token <avOp>   avOpLOr        "||"
%token <avOp>   avOpEq         "=="
%token <avOp>   avOpNeq        "!="
%token <avOp>                  '~'
%token <avOp>                  '&'
%token <avOp>                  '|'
%token <avOp>                  '^'
%token <avOp>  avOpLLsh        "<<"
%token <avOp>  avOpLRsh        ">>"
%token <avOp>  avOpALsh        "<<<"
%token <avOp>  avOpARsh        ">>>"
%token <avOp>  avOpQuestion    '?'
%token <avOp>  avOpColon       ':'
%token <avOp>                  '['
%token <avOp>                  ']'
%token <avOp>                  '('
%token <avOp>                  ')'
%token <avOp>                  '@'
%token <avOp>  avOpNColon      "-:"
%token <avOp>  avOpPColon      "+:"
%token <avOp>  avOpRNand       "~&"
%token <avOp>  avOpRNor        "~|"
%token <avOp>  avOpRXnor       "~^"


 // predence
%right '?' ':'
%left  avOpLor
%left  avOpLAnd
%left  '|' avOpRNor
%left  '^' avOpRXnor
%left  '&' avOpRNand
%left  avOpEq avOpNeq
%left  '>' '<' avOpGe avOpLe
%left  avOpLLsh avOpLRsh avOpALsh avOpARsh
%left  '+' '-'
%left  '*' '/' '%'
%left  avOpPower
%left  avOpPos avOpNeg avOpLNot avOpNot

 // keywords
%token <avKW> avAlways         "always"
%token <avKW> avAssign         "assign"
%token <avKW> avAutomatic      "automatic"    /* not supported yet */
%token <avKW> avBegin          "begin"
%token <avKW> avCase           "case"
%token <avKW> avCasex          "casex"        /* not supported yet */
%token <avKW> avCasez          "casez"        /* not supported yet */
%token <avKW> avCell           "cell"         /* not supported yet */
%token <avKW> avConfig         "config"       /* not supported yet */
%token <avKW> avDeassign       "deassign"     /* not supported yet */
%token <avKW> avDefault        "default"      /* not supported yet */
%token <avKW> avDefparam       "defparam"     /* not supported yet */
%token <avKW> avDesign         "design"	      /* not supported yet */
%token <avKW> avDisable        "disable"      /* not supported yet */
%token <avKW> avEdge           "edge"         /* not supported yet */
%token <avKW> avElse           "else"
%token <avKW> avEnd            "end"
%token <avKW> avEndcase        "endcase"
%token <avKW> avEndconfig      "endconfig"    /* not supported yet */
%token <avKW> avEndfunction    "endfunction"
%token <avKW> avEndgenerate    "endgenerate"
%token <avKW> avEndmodule      "endmodule"
%token <avKW> avEndprimitive   "endprimitive" /* not supported yet */
%token <avKW> avEndspecify     "endspecify"   /* not supported yet */
%token <avKW> avEndtable       "endtable"     /* not supported yet */
%token <avKW> avEndtask        "endtask"      /* not supported yet */
%token <avKW> avEvent          "event"        /* not supported yet */
%token <avKW> avFor            "for"
%token <avKW> avForce          "force"        /* not supported yet */
%token <avKW> avForever        "forever"      /* not supported yet */
%token <avKW> avFork           "fork"         /* not supported yet */
%token <avKW> avFunction       "function"
%token <avKW> avGenerate       "generate"
%token <avKW> avGenvar         "genvar"
%token <avKW> avHighz0         "highz0"       /* not supported yet */
%token <avKW> avHighz1         "highz1"       /* not supported yet */
%token <avKW> avIf             "if"
%token <avKW> avIfnone         "ifnone"       /* not supported yet */
%token <avKW> avIncdir         "incdir"       /* not supported yet */
%token <avKW> avInclude        "include"      /* not supported yet */
%token <avKW> avInitial        "initial"      /* not supported yet */
%token <avKW> avInout          "inout"
%token <avKW> avInput          "input"
%token <avKW> avInstance       "instance"     /* not supported yet */
%token <avKW> avInteger        "integer"
%token <avKW> avJoin           "join"         /* not supported yet */
%token <avKW> avLarge          "large"        /* not supported yet */
%token <avKW> avLiblist        "liblist"      /* not supported yet */
%token <avKW> avLibrary        "library"      /* not supported yet */
%token <avKW> avLocalparam     "localparam"   /* not supported yet */
%token <avKW> avMacromodule    "macromodule"  /* not supported yet */
%token <avKW> avMedium         "medium"       /* not supported yet */
%token <avKW> avModule         "module"
%token <avKW> avNegedge        "negedge"
%token <avKW> avNoshowcancelled "noshowcancelled"  /* not supported yet */
%token <avKW> avOutput         "output"
%token <avKW> avParameter      "parameter"
%token <avKW> avPosedge        "posedge"
%token <avKW> avPrimitive      "primitive"    /* not supported yet */
%token <avKW> avPulsestyle_onevent "pulsestyle_onevent"  /* not supported yet */
%token <avKW> avPulsestyle_ondetect "pulsestyle_ondetect"  /* not supported yet */
%token <avKW> avReal           "real"         /* not supported yet */
%token <avKW> avRealtime       "realtime"     /* not supported yet */
%token <avKW> avReg            "reg"
%token <avKW> avRelease        "release"      /* not supported yet */
%token <avKW> avRepeat         "repeat"       /* not supported yet */
%token <avKW> avScalared       "scalared"     /* not supported yet */
%token <avKW> avShowcancelled  "showcancelled" /* not supported yet */
%token <avKW> avSigned         "signed"       /* not supported yet */
%token <avKW> avSmall          "small"        /* not supported yet */
%token <avKW> avSpecify        "specify"      /* not supported yet */
%token <avKW> avSpecparam      "specparam"    /* not supported yet */
%token <avKW> avStrong0        "strong0"      /* not supported yet */
%token <avKW> avStrong1        "strong1"      /* not supported yet */
%token <avKW> avSupply0        "supply0"      /* not supported yet */
%token <avKW> avSupply1        "supply1"      /* not supported yet */
%token <avKW> avTable          "table"        /* not supported yet */
%token <avKW> avTask           "task"         /* not supported yet */
%token <avKW> avTime           "time"         /* not supported yet */
%token <avKW> avTriand         "triand"       /* not supported yet */
%token <avKW> avTrior          "trior"        /* not supported yet */
%token <avKW> avTrireg         "trireg"       /* not supported yet */
%token <avKW> avUnsigned       "unsigned"     /* not supported yet */
%token <avKW> avUse            "use"          /* not supported yet */
%token <avKW> avVectored       "vectored"     /* not supported yet */
%token <avKW> avWait           "wait"         /* not supported yet */
%token <avKW> avWand           "wand"         /* not supported yet */
%token <avKW> avWeak0          "weak0"        /* not supported yet */
%token <avKW> avWeak1          "weak1"        /* not supported yet */
%token <avKW> avWhile          "while"        /* not supported yet */
%token <avKW> avWire           "wire"
%token <avKW> avWor            "wor"          /* not supported yet */

 // primitives
%token <avPri> avAnd           "and"
%token <avPri> avOr            "or"
%token <avPri> avNot           "not"
%token <avPri> avNand          "nand"
%token <avPri> avNor           "nor"
%token <avPri> avXor           "xor"
%token <avPri> avXnor          "xnor"
%token <avPri> avBuf           "buf"
%token <avPri> avBufif0        "bufif0"   /* not supported yet */
%token <avPri> avBufif1        "bufif1"   /* not supported yet */
%token <avPri> avNotif0        "notif0"   /* not supported yet */
%token <avPri> avnotif1        "notif1"   /* not supported yet */
%token <avPri> avPulldown      "pulldown" /* not supported yet */
%token <avPri> avPullup        "pullup"   /* not supported yet */
%token <avPri> avCmos          "cmos"     /* not supported yet */
%token <avPri> avNmos          "nmos"     /* not supported yet */
%token <avPri> avPmos          "pmos"     /* not supported yet */
%token <avPri> avRcmos         "rcmos"    /* not supported yet */
%token <avPri> avRnmos         "rnmos"    /* not supported yet */
%token <avPri> avRpmos         "rpmos"    /* not supported yet */
%token <avPri> avRtran         "rtran"    /* not supported yet */
%token <avPri> avRtranif0      "rtranif0" /* not supported yet */
%token <avPri> avRtranif1      "rtranif1" /* not supported yet */
%token <avPri> avTran          "tran"     /* not supported yet */
%token <avPri> avTranif0       "tranif0"  /* not supported yet */
%token <avPri> avTranif1       "tranif1"  /* not supported yet */



%%
