// -*- Bison -*-
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
#include <vector>
#include <stack>

%}

%union {
  int                         avTOp;			/* operator, dummy value, not suppose to be used */
  int                         avTKW;			/* keyword, dummy value, not suppose to be used */
  int                         avTPri;			/* primitive gate, dummy value, not suppose to be used */
  vector<std::string>         avTIDList;		/* a list of IDs */
  std::string                 avTID;     		/* identifier */
  vector<AVNetParameter>      avTParaList;		/* parameter list */
  AVNetParameter              avTPara;			/* a single parameter assign */
}


///////////////////////////////////////////////////
// token definitions

// operators
%token <avTOp>                  '{'
%token <avTOp>                  '}'
%token <avTOp>                  '+'
%token <avTOp>                  '-'
%token <avTOp>                  '*'
%token <avTOp>                  '/'
%token <avTOp>   avOpPower      "**"
%token <avTOp>                  '%'
%token <avTOp>                  '>'
%token <avTOp>                  '<'
%token <avTOp>   avOpGe         ">="
%token <avTOp>   avOpLe         "<="
%token <avTOp>                  '!'
%token <avTOp>   avOpLAnd       "&&"
%token <avTOp>   avOpLOr        "||"
%token <avTOp>   avOpEq         "=="
%token <avTOp>   avOpNeq        "!="
%token <avTOp>                  '~'
%token <avTOp>                  '&'
%token <avTOp>                  '|'
%token <avTOp>                  '^'
%token <avTOp>  avOpLLsh        "<<"
%token <avTOp>  avOpLRsh        ">>"
%token <avTOp>  avOpALsh        "<<<"
%token <avTOp>  avOpARsh        ">>>"
%token <avTOp>  avOpQuestion    '?'
%token <avTOp>  avOpColon       ':'
%token <avTOp>                  '['
%token <avTOp>                  ']'
%token <avTOp>                  '('
%token <avTOp>                  ')'
%token <avTOp>                  '@'
%token <avTOp>  avOpColon      "-:"
%token <avTOp>  avOpPColon      "+:"
%token <avTOp>  avOpRNand       "~&"
%token <avTOp>  avOpRNor        "~|"
%token <avTOp>  avOpRXnor       "~^"


 // predence
%right '?' ':'
%left  avTOpLor
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
%token <avTKW> avAlways         "always"
%token <avTKW> avAssign         "assign"
%token <avTKW> avAutomatic      "automatic"    /* not supported yet */
%token <avTKW> avBegin          "begin"
%token <avTKW> avCase           "case"
%token <avTKW> avCasex          "casex"        /* not supported yet */
%token <avTKW> avCasez          "casez"        /* not supported yet */
%token <avTKW> avCell           "cell"         /* not supported yet */
%token <avTKW> avConfig         "config"       /* not supported yet */
%token <avTKW> avDeassign       "deassign"     /* not supported yet */
%token <avTKW> avDefault        "default"      /* not supported yet */
%token <avTKW> avDefparam       "defparam"     /* not supported yet */
%token <avTKW> avDesign         "design"	      /* not supported yet */
%token <avTKW> avDisable        "disable"      /* not supported yet */
%token <avTKW> avEdge           "edge"         /* not supported yet */
%token <avTKW> avElse           "else"
%token <avTKW> avEnd            "end"
%token <avTKW> avEndcase        "endcase"
%token <avTKW> avEndconfig      "endconfig"    /* not supported yet */
%token <avTKW> avEndfunction    "endfunction"
%token <avTKW> avEndgenerate    "endgenerate"
%token <avTKW> avEndmodule      "endmodule"
%token <avTKW> avEndprimitive   "endprimitive" /* not supported yet */
%token <avTKW> avEndspecify     "endspecify"   /* not supported yet */
%token <avTKW> avEndtable       "endtable"     /* not supported yet */
%token <avTKW> avEndtask        "endtask"      /* not supported yet */
%token <avTKW> avEvent          "event"        /* not supported yet */
%token <avTKW> avFor            "for"
%token <avTKW> avForce          "force"        /* not supported yet */
%token <avTKW> avForever        "forever"      /* not supported yet */
%token <avTKW> avFork           "fork"         /* not supported yet */
%token <avTKW> avFunction       "function"
%token <avTKW> avGenerate       "generate"
%token <avTKW> avGenvar         "genvar"
%token <avTKW> avHighz0         "highz0"       /* not supported yet */
%token <avTKW> avHighz1         "highz1"       /* not supported yet */
%token <avTKW> avIf             "if"
%token <avTKW> avIfnone         "ifnone"       /* not supported yet */
%token <avTKW> avIncdir         "incdir"       /* not supported yet */
%token <avTKW> avInclude        "include"      /* not supported yet */
%token <avTKW> avInitial        "initial"      /* not supported yet */
%token <avTKW> avInout          "inout"
%token <avTKW> avInput          "input"
%token <avTKW> avInstance       "instance"     /* not supported yet */
%token <avTKW> avInteger        "integer"
%token <avTKW> avJoin           "join"         /* not supported yet */
%token <avTKW> avLarge          "large"        /* not supported yet */
%token <avTKW> avLiblist        "liblist"      /* not supported yet */
%token <avTKW> avLibrary        "library"      /* not supported yet */
%token <avTKW> avLocalparam     "localparam"   /* not supported yet */
%token <avTKW> avMacromodule    "macromodule"  
%token <avTKW> avMedium         "medium"       /* not supported yet */
%token <avTKW> avModule         "module"
%token <avTKW> avNegedge        "negedge"
%token <avTKW> avNoshowcancelled "noshowcancelled"  /* not supported yet */
%token <avTKW> avOutput         "output"
%token <avTKW> avParameter      "parameter"
%token <avTKW> avPosedge        "posedge"
%token <avTKW> avPrimitive      "primitive"    /* not supported yet */
%token <avTKW> avPulsestyle_onevent "pulsestyle_onevent"  /* not supported yet */
%token <avTKW> avPulsestyle_ondetect "pulsestyle_ondetect"  /* not supported yet */
%token <avTKW> avReal           "real"         /* not supported yet */
%token <avTKW> avRealtime       "realtime"     /* not supported yet */
%token <avTKW> avReg            "reg"
%token <avTKW> avRelease        "release"      /* not supported yet */
%token <avTKW> avRepeat         "repeat"       /* not supported yet */
%token <avTKW> avScalared       "scalared"     /* not supported yet */
%token <avTKW> avShowcancelled  "showcancelled" /* not supported yet */
%token <avTKW> avSigned         "signed"       /* not supported yet */
%token <avTKW> avSmall          "small"        /* not supported yet */
%token <avTKW> avSpecify        "specify"      /* not supported yet */
%token <avTKW> avSpecparam      "specparam"    /* not supported yet */
%token <avTKW> avStrong0        "strong0"      /* not supported yet */
%token <avTKW> avStrong1        "strong1"      /* not supported yet */
%token <avTKW> avSupply0        "supply0"      /* not supported yet */
%token <avTKW> avSupply1        "supply1"      /* not supported yet */
%token <avTKW> avTable          "table"        /* not supported yet */
%token <avTKW> avTask           "task"         /* not supported yet */
%token <avTKW> avTime           "time"         /* not supported yet */
%token <avTKW> avTriand         "triand"       /* not supported yet */
%token <avTKW> avTrior          "trior"        /* not supported yet */
%token <avTKW> avTrireg         "trireg"       /* not supported yet */
%token <avTKW> avUnsigned       "unsigned"     /* not supported yet */
%token <avTKW> avUse            "use"          /* not supported yet */
%token <avTKW> avVectored       "vectored"     /* not supported yet */
%token <avTKW> avWait           "wait"         /* not supported yet */
%token <avTKW> avWand           "wand"         /* not supported yet */
%token <avTKW> avWeak0          "weak0"        /* not supported yet */
%token <avTKW> avWeak1          "weak1"        /* not supported yet */
%token <avTKW> avWhile          "while"        /* not supported yet */
%token <avTKW> avWire           "wire"
%token <avTKW> avWor            "wor"          /* not supported yet */

 // primitives
%token <avTPri> avAnd           "and"
%token <avTPri> avOr            "or"
%token <avTPri> avNot           "not"
%token <avTPri> avNand          "nand"
%token <avTPri> avNor           "nor"
%token <avTPri> avXor           "xor"
%token <avTPri> avXnor          "xnor"
%token <avTPri> avBuf           "buf"
%token <avTPri> avBufif0        "bufif0"   /* not supported yet */
%token <avTPri> avBufif1        "bufif1"   /* not supported yet */
%token <avTPri> avNotif0        "notif0"   /* not supported yet */
%token <avTPri> avnotif1        "notif1"   /* not supported yet */
%token <avTPri> avPulldown      "pulldown" /* not supported yet */
%token <avTPri> avPullup        "pullup"   /* not supported yet */
%token <avTPri> avCmos          "cmos"     /* not supported yet */
%token <avTPri> avNmos          "nmos"     /* not supported yet */
%token <avTPri> avPmos          "pmos"     /* not supported yet */
%token <avTPri> avRcmos         "rcmos"    /* not supported yet */
%token <avTPri> avRnmos         "rnmos"    /* not supported yet */
%token <avTPri> avRpmos         "rpmos"    /* not supported yet */
%token <avTPri> avRtran         "rtran"    /* not supported yet */
%token <avTPri> avRtranif0      "rtranif0" /* not supported yet */
%token <avTPri> avRtranif1      "rtranif1" /* not supported yet */
%token <avTPri> avTran          "tran"     /* not supported yet */
%token <avTPri> avTranif0       "tranif0"  /* not supported yet */
%token <avTPri> avTranif1       "tranif1"  /* not supported yet */


%start source_text

%%

// The formal BNF synteax from Annex A, IEEE Std 1364-2001 Version C Verilog Hardware Description Language

 // A.1.3 Module and primitive source text
source_text 
    : /* empty */ 
    | descriptionList
    ;

descriptionList 
    : description      
    | descriptionList description 
    ;

description 
    : module_declaration      
      // only module is supported right now, macros (define) should be handled by Lexer already
    | error        
    ;

module_declaration
    : module_keyword avID module_parameter_port_list { db->push(AVNetModule($2, $3)); /* initialise a module */}
        '(' list_of_ports ')' ';'
        module_items
      avEndmodule                      { db->insert(db->current()); db->pop(); /* pop out the module */}
    | module_keyword avID module_parameter_port_list { db->push(AVNetModule($2, $3)); /* initialise a module */}
        '(' list_of_port_declarations ')' ';'
        non_port_module_items
      avEndmodule                      { db->insert(db->current()); db->pop(); /* pop out the module */}
    | module_keyword avID module_parameter_port_list { db->push(AVNetModule($2, $3));/* initialise a module */ }
        non_port_module_items
      avEndmodule                      { db->insert(db->current()); db->pop(); /* pop out the module */}
    ;

module_keyword
    : avModule
    | avMacromodule
    ;

// A.1.4 Module paramters and ports
module_parameter_port_list<avTParaList>
    : /* empty */   
    | '#' '(' ')'   
    | '#' '(' parameter_declarations ')' { $$ = $3; }
    ;

parameter_declarations<avTParaList>
    : parameter_declaration              { $$.push_back($1); }
    | parameter_declarations ',' parameter_declaration { $$.push_back($3); }
    ;

// port list, not fully supported yet
list_of_ports
    : /* empty */
    | identifier_list                  { db->current()->insert_ports($1); /* inser a list of ports */}
    ;

list_of_port_declarations
    : port_declaration
    | list_of_port_declarations ',' port_declaration
    ;

port_declaration
    : input_declaration
    | output_declaration
//    | inout_declaration
    ;

// A.1.5 Module items
module_items
    : module_or_generate_item
    | port_declaration
    | generated_instantiation
//    | local_parameter_declaration
    | parameter_declaration
//    | specify_block
//    | specparam_declaration
    ;

non_port_module_item
    : module_or_generate_item
    | generated_instantiation
//    | local_parameter_declaration
    | parameter_declaration
//    | specify_block
//    | specparam_declaration
    ;

module_or_generate_item
    : module_or_generate_item_declaration
//    | parameter_override
    | continuous_assign
    | gate_instantiation
//    | udp_instantiation
    | module_instantiation
//    | initial_construct
    | always_construct
    ;
    
module_or_generate_item_declaration
    : net_declaration
    | reg_declaration
    | integer_declaration
//    | real_declaration
//    | time_declaration
//    | realtime_declaration
//    | event_declaration
    | genvar_declaration
//    | task_declaration
    | function_declaration
    ;

// A.2.1 Declaration types
// A.2.1.1 Module parameter declarations




// parameter declaration inside module
module_item_parameter
    : avParameter parameter_assign ';' { if(!(db->current())) {db->current()->add_parameter($2); }
                                         else {avError("Error: Unexpected parameter declaration outside a module!")}; }
    ;

// port declaration
module_item_port
    : avInput identifier_list ';'          { if(!(db->current())) {db->current()->add_ports(0, avRange(1), $2); }
                                             else {avError("Error: Unexpected port declaration outside a module!")}; }
    | avInput avRange identifier_list ';'  { if(!(db->current())) {db->current()->add_ports(0, $2, $3); }
                                             else {avError("Error: Unexpected port declaration outside a module!")}; }
    | avOutput identifier_list ';'         { if(!(db->current())) {db->current()->add_ports(1, avRange(1), $2); }
                                             else {avError("Error: Unexpected port declaration outside a module!")}; }
    | avOutput avRange identifier_list ';' { if(!(db->current())) {db->current()->add_ports(1, $2, $3); }
                                             else {avError("Error: Unexpected port declaration outside a module!")}; }

// wire declaration
module_item_wire:
    : avWire 



identifier_list<avTIDList>
    : avID                             { $$.push_back($1); }
    | identifier_list ',' avID         { $$.push_back($3); }
