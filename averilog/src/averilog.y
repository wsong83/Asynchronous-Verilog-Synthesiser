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
%token <avOp>              '{'
%token <avOp>              '}'
%token <avOp>              '+'
%token <avOp>              '-'
%token <avOp>              '*'
%token <avOp>              '/'
%token <avOp>   avPower    "**"
%token <avOp>              '%'
%token <avOp>              '>'
%token <avOp>              '<'
%token <avOp>   avGe       ">="
%token <avOp>   avLe       "<="
%token <avOp>              '!'
%token <avOp>   avLAnd     "&&"
%token <avOp>   avLOr      "||"
%token <avOp>   avLEq      "=="
%token <avOp>   avLNeq     "!="
%token <avOp>              '~'
%token <avOp>              '&'
%token <avOp>              '|'
%token <avOp>              '^'
%token <avOp>  avLLsh      "<<"
%token <avOp>  avLRsh      ">>"
%token <avOp>  avALsh      "<<<"
%token <avOp>  avARsh      ">>>"
%token <avOp>  avQuestion  '?'
%token <avOp>  avColon     ':'
%token <avOp>              '['
%token <avOp>              ']'
%token <avOp>              '('
%token <avOp>              ')'
%token <avOp>              '@'

 // keywords
%token <avKW> avAlways     "always"
%token <avKW> avAssign     "assign"
%token <avKW> avAutomatic  "automatic"  /* not supported yet */
%token <avKW> avBegin      "begin"
%token <avKW> avCase       "case"
%token <avKW> avCasex      "casex"      /* not supported yet */
%token <avKW> avCasez      "casez"	/* not supported yet */
%token <avKW> avCell       "cell"       /* not supported yet */
%token <avKW> avConfig     "config"	/* not supported yet */
%token <avKW> avDeassign   "deassign"	/* not supported yet */
%token <avKW> avDefault    "default"	/* not supported yet */
%token <avKW> avDefparam   "defparam"	/* not supported yet */
%token <avKW> avDesign     "design"	/* not supported yet */
%token <avKW> avDisable    "disable"    /* not supported yet */
%token <avKW> avEdge       "edge"       /* not supported yet */
%token <avKW> avIf         "if"


 // primitives
%token <avPri> avAnd       "and"
%token <avPri> avOr        "or"
%token <avPri> avNot       "not"
%token <avPri> avNand      "nand"
%token <avPri> avNor       "nor"
%token <avPri> avXor       "xor"
%token <avPri> avXnor      "xnor"
%token <avPri> avBuf       "buf"
%token <avPri> avBufif0    "bufif0"   /* not supported yet */
%token <avPri> avBufif1    "bufif1"   /* not supported yet */
%token <avPri> avNotif0    "notif0"   /* not supported yet */
%token <avPri> avnotif1    "notif1"   /* not supported yet */
%token <avPri> avPulldown  "pulldown" /* not supported yet */
%token <avPri> avPullup    "pullup"   /* not supported yet */
%token <avPri> avCmos      "cmos"     /* not supported yet */
%token <avPri> avNmos      "nmos"     /* not supported yet */
%token <avPri> avPmos      "pmos"     /* not supported yet */
%token <avPri> avRcmos     "rcmos"    /* not supported yet */
%token <avPri> avRnmos     "rnmos"    /* not supported yet */
%token <avPri> avRpmos     "rpmos"    /* not supported yet */
%token <avPri> avRtran     "rtran"    /* not supported yet */
%token <avPri> avRtranif0  "rtranif0" /* not supported yet */
%token <avPri> avRtranif1  "rtranif1" /* not supported yet */
%token <avPri> avTran      "tran"     /* not supported yet */
%token <avPri> avTranif0   "tranif0"  /* not supported yet */
%token <avPri> avTranif1   "tranif1"  /* not supported yet */



%%
