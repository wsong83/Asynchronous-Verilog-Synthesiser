// -*- Bison -*-
%skeleton "lalr1.cc"
%defines
%define namespace "averilog"
%define parser_class_name "av_parser"
%language "c++"
%output "averilog.cc"
%locations
%parse-param {std::string fname}
%parse-param {FILE * *sfile}
%lex-param {yyscan_t avscanner}
%debug
%{
/*
 * Copyright (c) 2012 Wei Song <songw@cs.man.ac.uk> 
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
 * 02/12/2011   Wei Song
 *
 *
 */

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <stack>
#include "averilog_util.h"
#include "averilog.lex.h"

#define yylex avlex

yyscan_t avscanner;

 void averilog::av_parser::error (const location_type& loc, const std::string& msg) {
   std::cout << loc << " " << msg << std::endl;
 }

%}

%initial-action
{
  @$.initialize(&fname);
  avlex_init(&avscanner);
  avset_in(*sfile, avscanner);
}


///////////////////////////////////////////////////
// token definitions

// operators
%token oPower       "**"
%token oGe          ">="
%token oLe          "<="   
%token oLAnd        "&&"
%token oLOr         "||"
%token oEq          "=="
%token oCEq         "==="
%token oNeq         "!="
%token oCNeq        "!=="
%token oLLsh        "<<"
%token oLRsh        ">>"
%token oARsh        ">>>"
%token oNColon      "-:"
%token oPColon      "+:"
%token oRNand       "~&"
%token oRNor        "~|"
%token oRXnor       "~^"

 // keywords
%token kAlways         "always"
%token kAssign         "assign"
%token kAutomatic      "automatic"    /* not supported yet */
%token kBegin          "begin"
%token kCase           "case"
%token kCasex          "casex"        /* not supported yet */
%token kCasez          "casez"        /* not supported yet */
%token kCell           "cell"         /* not supported yet */
%token kConfig         "config"       /* not supported yet */
%token kDeassign       "deassign"     /* not supported yet */
%token kDefault        "default"      /* not supported yet */
%token kDefparam       "defparam"     /* not supported yet */
%token kDesign         "design"	      /* not supported yet */
%token kDisable        "disable"      /* not supported yet */
%token kEdge           "edge"         /* not supported yet */
%token kElse           "else"
%token kEnd            "end"
%token kEndcase        "endcase"
%token kEndconfig      "endconfig"    /* not supported yet */
%token kEndfunction    "endfunction"
%token kEndgenerate    "endgenerate"
%token kEndmodule      "endmodule"
%token kEndprimitive   "endprimitive" /* not supported yet */
%token kEndspecify     "endspecify"   /* not supported yet */
%token kEndtable       "endtable"     /* not supported yet */
%token kEndtask        "endtask"      /* not supported yet */
%token kEvent          "event"        /* not supported yet */
%token kFor            "for"
%token kForce          "force"        /* not supported yet */
%token kForever        "forever"      /* not supported yet */
%token kFork           "fork"         /* not supported yet */
%token kFunction       "function"
%token kGenerate       "generate"
%token kGenvar         "genvar"
%token kHighz0         "highz0"       /* not supported yet */
%token kHighz1         "highz1"       /* not supported yet */
%token kIf             "if"
%token kIfnone         "ifnone"       /* not supported yet */
%token kIncdir         "incdir"       /* not supported yet */
%token kInclude        "include"      /* not supported yet */
%token kInitial        "initial"      /* not supported yet */
%token kInout          "inout"
%token kInput          "input"
%token kInstance       "instance"     /* not supported yet */
%token kInteger        "integer"
%token kJoin           "join"         /* not supported yet */
%token kLarge          "large"        /* not supported yet */
%token kLiblist        "liblist"      /* not supported yet */
%token kLibrary        "library"      /* not supported yet */
%token kLocalparam     "localparam"   /* not supported yet */
%token kMacromodule    "macromodule"  
%token kMedium         "medium"       /* not supported yet */
%token kModule         "module"
%token kNegedge        "negedge"
%token kNoshowcancelled "noshowcancelled"  /* not supported yet */
%token kOutput         "output"
%token kParameter      "parameter"
%token kPosedge        "posedge"
%token kPrimitive      "primitive"    /* not supported yet */
%token kPulsestyle_onevent "pulsestyle_onevent"  /* not supported yet */
%token kPulsestyle_ondetect "pulsestyle_ondetect"  /* not supported yet */
%token kReal           "real"         /* not supported yet */
%token kRealtime       "realtime"     /* not supported yet */
%token kReg            "reg"
%token kRelease        "release"      /* not supported yet */
%token kRepeat         "repeat"       /* not supported yet */
%token kScalared       "scalared"     /* not supported yet */
%token kShowcancelled  "showcancelled" /* not supported yet */
%token kSigned         "signed"       /* not supported yet */
%token kSmall          "small"        /* not supported yet */
%token kSpecify        "specify"      /* not supported yet */
%token kSpecparam      "specparam"    /* not supported yet */
%token kStrong0        "strong0"      /* not supported yet */
%token kStrong1        "strong1"      /* not supported yet */
%token kSupply0        "supply0"      /* not supported yet */
%token kSupply1        "supply1"      /* not supported yet */
%token kTable          "table"        /* not supported yet */
%token kTask           "task"         /* not supported yet */
%token kTime           "time"         /* not supported yet */
%token kTriand         "triand"       /* not supported yet */
%token kTrior          "trior"        /* not supported yet */
%token kTrireg         "trireg"       /* not supported yet */
%token kUnsigned       "unsigned"     /* not supported yet */
%token kUse            "use"          /* not supported yet */
%token kVectored       "vectored"     /* not supported yet */
%token kWait           "wait"         /* not supported yet */
%token kWand           "wand"         /* not supported yet */
%token kWeak0          "weak0"        /* not supported yet */
%token kWeak1          "weak1"        /* not supported yet */
%token kWhile          "while"        /* not supported yet */
%token kWire           "wire"
%token kWor            "wor"          /* not supported yet */

 // primitives
%token pAnd           "and"
%token pOr            "or"
%token pNot           "not"
%token pNand          "nand"
%token pNor           "nor"
%token pXor           "xor"
%token pXnor          "xnor"
%token pBuf           "buf"
%token pBufif0        "bufif0"   /* not supported yet */
%token pBufif1        "bufif1"   /* not supported yet */
%token pNotif0        "notif0"   /* not supported yet */
%token pnotif1        "notif1"   /* not supported yet */
%token pPulldown      "pulldown" /* not supported yet */
%token pPullup        "pullup"   /* not supported yet */
%token pCmos          "cmos"     /* not supported yet */
%token pNmos          "nmos"     /* not supported yet */
%token pPmos          "pmos"     /* not supported yet */
%token pRcmos         "rcmos"    /* not supported yet */
%token pRnmos         "rnmos"    /* not supported yet */
%token pRpmos         "rpmos"    /* not supported yet */
%token pRtran         "rtran"    /* not supported yet */
%token pRtranif0      "rtranif0" /* not supported yet */
%token pRtranif1      "rtranif1" /* not supported yet */
%token pTran          "tran"     /* not supported yet */
%token pTranif0       "tranif0"  /* not supported yet */
%token pTranif1       "tranif1"  /* not supported yet */

 // other
%token<tID> identifier
%token<tNumber> number

 // predence
%right '?' ':'
%left  "||"
%left  "&&"
%left  '|' "~|"
%left  '^' "~^"
%left  '&' "~&"
%left  "==" "!=" "===" "!=="
%left  '>' '<' "<=" ">="
%left  "<<" ">>" ">>>"
%left  '+' '-'
%left  '*' '/' '%'
%left  "**"
%left  oUNARY

 // type definitions
%type <tModuleName> module_identifier
 //%type <tPortName> port_identifier
%type <tVarName> variable_identifier


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
: "module" module_identifier ';'  { std::cout<< "module " << *$2<<std::endl; }
        module_items
        "endmodule"                      { std::cout<< *$2 << "endmodule"; }
    | "module" module_identifier '(' list_of_ports ')' ';'
        module_items
      "endmodule"
    | "module" module_identifier '#' '(' parameter_declaration ')' '(' list_of_ports ')' ';'
        module_items
      "endmodule"                   
    ;

// A.1.4 Module paramters and ports
list_of_ports
    : /* empty */
    | list_of_port_identifiers                  
    ;

// A.1.5 Module items
module_items
    : parameter_declaration ';'
    | input_declaration ';'
    | output_declaration ';'
    | variable_declaration ';'
    | function_declaration
    | continuous_assign
    | gate_instantiation
    | module_instantiation
    | always_construct
    | generated_instantiation
    ;

    
// A.2.1 Declaration types
// A.2.1.1 Module parameter declarations
parameter_declaration
    : "parameter" list_of_param_assignments
    ;

// A.2.1.2 Port declarations
input_declaration 
    : "input" list_of_port_identifiers
    | "input" '[' expression ':' expression ']' list_of_port_identifiers
    ;

output_declaration 
    : "output" list_of_port_identifiers
    | "output" '[' expression ':' expression ']' list_of_port_identifiers
    ;

// A.2.1.3 Type declarations
variable_declaration 
    : "wire" list_of_variable_identifiers
    | "wire" '[' expression ':' expression ']' list_of_variable_identifiers
    | "reg" list_of_variable_identifiers
    | "reg" '[' expression ':' expression ']' list_of_variable_identifiers
    | "genvar" list_of_variable_identifiers
    | "integer" list_of_variable_identifiers
    ;

list_of_variable_declarations
    : variable_declaration ';'
    | list_of_variable_declarations  variable_declaration ';'
    ;

// A.2.3 Declaration lists
list_of_param_assignments 
    : param_assignment
    | list_of_param_assignments ',' param_assignment
    ;

list_of_port_identifiers 
    : port_identifier 
    | list_of_port_identifiers ',' port_identifier
    ;

list_of_variable_identifiers 
    : variable_identifier
    | variable_identifier '=' expression
    | list_of_variable_identifiers ',' variable_identifier
    | list_of_variable_identifiers ',' variable_identifier '=' expression
    ;

// A.2.4 Declaration assignments
param_assignment 
    : parameter_identifier '=' expression
    ;

//A.2.6 Function declarations
function_declaration
    : "function" function_identifier ';'
        list_of_function_item_declaration
        statement
      "endfunction"
    | "function" "automatic" function_identifier ';'
        list_of_function_item_declaration
        statement
      "endfunction"
    | "function" function_identifier '(' list_of_ports ')' ';'
        list_of_function_item_declaration
        statement
      "endfunction"
    | "function" "automatic" function_identifier '(' list_of_ports ')' ';'
        list_of_function_item_declaration
        statement
      "endfunction"
    ;

list_of_function_item_declaration
    : function_item_declaration
    | list_of_function_item_declaration function_item_declaration
    ;

function_item_declaration 
    : input_declaration ';'
    | variable_declaration ';'
    ;

//A.3 Primitive instances
//A.3.1 Primitive instantiation and instances
gate_instantiation
    : n_input_gatetype n_input_gate_instances ';'
    | n_output_gatetype n_output_gate_instances ';'
    ;

n_input_gate_instances
    : n_input_gate_instance
    | n_input_gate_instances ',' n_input_gate_instance
    ;

n_input_gate_instance
    : '(' output_terminal ',' input_terminals ')'
    | instance_identifier '(' output_terminal ',' input_terminals ')'
    | instance_identifier '[' expression ':' expression ']' '(' output_terminal ',' input_terminals ')'
    ;

input_terminals
    : input_terminal
    | input_terminals ',' input_terminal
    ;

n_output_gate_instances
    : n_output_gate_instance
    | n_output_gate_instances ',' n_output_gate_instance
    ;

n_output_gate_instance
    : '(' output_terminals ',' input_terminal ')'
    | instance_identifier '(' output_terminals ',' input_terminal ')'
    | instance_identifier '[' expression ':' expression ']' '(' output_terminals ',' input_terminal ')'
    ;

output_terminals
    : output_terminal
    | output_terminals ',' output_terminal
    ;

//A.3.3 Primitive terminals
input_terminal
    : expression
    ;

output_terminal
    : variable_lvalue
    ;

//A.3.4 Primitive gate and switch types
n_input_gatetype
    : "and" 
    | "nand" 
    | "or" 
    | "nor" 
    | "xor" 
    | "xnor"
    ;

n_output_gatetype
    : "buf" 
    | "not"
    ;

//A.4.1 Module instantiation
module_instantiation 
    : module_identifier module_instances ';'
    | module_identifier parameter_value_assignment module_instances ';'
    ;

module_instances
    : module_instance
    | module_instance ',' module_instance
    ;

parameter_value_assignment 
    : '#' '(' list_of_parameter_assignments ')'
    ;

list_of_parameter_assignments 
    : ordered_parameter_assignments
    | named_parameter_assignments
    ;
  
ordered_parameter_assignments
    : ordered_parameter_assignment
    | ordered_parameter_assignments ',' ordered_parameter_assignment
    ;

named_parameter_assignments
    : named_parameter_assignment
    | named_parameter_assignments ',' named_parameter_assignment
    ;

ordered_parameter_assignment 
    : expression
    ;

named_parameter_assignment 
    : '.' parameter_identifier '('  ')'
    | '.' parameter_identifier '(' expression ')'
    ;

module_instance 
    : instance_identifier '(' ')'
    | instance_identifier '[' expression ':' expression ']' '(' ')'
    | instance_identifier '(' list_of_port_connections ')'
    | instance_identifier '[' expression ':' expression ']' '(' list_of_port_connections ')'
    ;

list_of_port_connections 
    : ordered_port_connections
    | named_port_connections
    ;

ordered_port_connections
    : ordered_port_connection 
    | ordered_port_connections ',' ordered_port_connection
    ;

named_port_connections
    : named_port_connection
    | named_port_connections ',' named_port_connection
    ;

ordered_port_connection 
    : expression
    ;

named_port_connection 
    : '.' port_identifier '(' ')'
    | '.' port_identifier '(' expression ')'
    ;

//A.4.2 Generated instantiation
generated_instantiation 
    : "generate" generate_items "endgenerate"
    ;

generate_items
    : /* empty */
    | generate_item
    | generate_items generate_item
    ;

generate_item_or_null
    : /* empty */
    | generate_item
    ;

generate_item 
    : variable_declaration ';'
    | function_declaration
    | continuous_assign
    | gate_instantiation
    | module_instantiation
    | always_construct
    | generate_conditional_statement
    | generate_case_statement
    | generate_loop_statement
    | generate_block
    ;

generate_conditional_statement 
    : "if" '(' expression ')' generate_item_or_null 
    | "if" '(' expression ')' generate_item_or_null "else" generate_item_or_null
    ;

generate_case_statement 
    : "case" '(' expression ')' "default" generate_item_or_null "endcase"
    | "case" '(' expression ')' genvar_case_items "endcase"
    | "case" '(' expression ')' genvar_case_items "default" generate_item_or_null "endcase"
    ;

genvar_case_items
    : genvar_case_item
    | genvar_case_items genvar_case_item
    ;

genvar_case_item 
    : expressions ':' generate_item_or_null 
    | "default" ':' generate_item_or_null
    ;

generate_loop_statement 
    : "for" '(' blocking_assignment ';' expression ';' blocking_assignment ')' "begin" ':' block_identifier generate_item_or_null "end"
    ;

generate_block 
    : "begin" generate_items "end"
    | "begin" ':' block_identifier  generate_items "end"
    ;

//A.6.1 Continuous assignment statements
continuous_assign 
    : "assign" list_of_net_assignments ';'              { std::cout << "continueous assign" << std::endl; }
    ;

list_of_net_assignments 
    : blocking_assignment                                { std::cout << "blocking assignment" << std::endl;}
    | list_of_net_assignments ',' blocking_assignment
    ;


//A.6.2 Procedural blocks and assignments
always_construct 
    : "always" statement
    ;

blocking_assignment 
: variable_lvalue '=' expression  
    ;

nonblocking_assignment 
    : variable_lvalue "<=" expression
    ;

//A.6.3 Parallel and sequential blocks    
seq_block
    : "begin" statement "end"
    | "begin" list_of_variable_declarations statement "end"
    | "begin" ':' block_identifier statement "end"
    | "begin" ':' block_identifier list_of_variable_declarations statement "end"
    ;

//A.6.4 Statements
statement
    : blocking_assignment ';'
    | case_statement
    | conditional_statement
    | loop_statement
    | nonblocking_assignment ';'
    | procedural_timing_control_statement
    | seq_block
    ;

statement_or_null 
    : /* empty */
    | statement
    ;
    
//A.6.5 Timing control statements
event_control
    : '@' '(' event_expressions ')'
    ;

event_expressions
    : event_expression
    | event_expressions "or" event_expression
    | event_expressions ',' event_expression
    ;

event_expression
    : expression
    | "posedge" expression
    | "negedge" expression
    ;

procedural_timing_control_statement
    : event_control statement_or_null
    ;

//A.6.6 Conditional statements
conditional_statement 
    : "if" '(' expression ')' statement_or_null
    | "if" '(' expression ')' statement_or_null "else" statement_or_null
    ;

//A.6.7 Case statements
case_statement 
    : "case" '(' expression ')' "default" statement_or_null "endcase"
    | "case" '(' expression ')' case_items "endcase"
    | "case" '(' expression ')' case_items "default" statement_or_null "endcase"
    ;

case_items
    : case_item
    | case_items case_item
    ;

case_item 
    : expressions ':' statement_or_null
    | "default" ':' statement_or_null
    ;

//A.6.8 Looping statements
loop_statement
    : "while" '(' expression ')' statement
    | "for" '(' blocking_assignment ';' expression ';' blocking_assignment ')' statement
    ;

// A.8 Expressions
// A.8.1 Concatenations
expressions
    : expression
    | expressions ',' expression
    ;

concatenation
    : '{' expressions '}'
    | '{' expression concatenation '}'
    ;

// A.8.2 Function calls
function_call
    : function_identifier '(' expressions ')'
    ;

//A.8.3 Expressions
expression
    : primary
    | '+' primary %prec oUNARY
    | '-' primary %prec oUNARY
    | '!' primary %prec oUNARY
    | '~' primary %prec oUNARY
    | '&' primary %prec oUNARY
    | "~&" primary %prec oUNARY
    | '|' primary %prec oUNARY
    | "~|" primary %prec oUNARY
    | '^' primary %prec oUNARY
    | "~^" primary %prec oUNARY
    | expression '+' expression
    | expression '-' expression
    | expression '*' expression
    | expression '/' expression
    | expression '%' expression
    | expression "==" expression
    | expression "!=" expression
    | expression "===" expression
    | expression "!==" expression
    | expression "&&" expression
    | expression "||" expression
    | expression "**" expression
    | expression '<' expression
    | expression "<=" expression
    | expression '>' expression
    | expression ">=" expression
    | expression '&' expression
    | expression '|' expression
    | expression '^' expression
    | expression "~^" expression
    | expression ">>" expression
    | expression "<<" expression
    | expression ">>>" expression
    | expression '?' expression ':' expression
    ;

range_expression
    : expression
    | expression ':' expression
    | expression "+:" expression
    | expression "-:" expression
    ;

//A.8.4 Primaries
primary
: number                             
    | variable_identifier
    | concatenation
    | function_call
    | '(' expression ')'
    ;

//A.8.5 Expression left-side values
variable_lvalue
: variable_identifier    
    | concatenation
    ;

//A.9 General
//A.9.3 Identifiers
block_identifier 
    : identifier
    ;

function_identifier 
    : identifier
    ;

module_identifier
:  identifier             { $$.reset(new netlist::MIdentifier(*$1)); }
    ;

instance_identifier 
    : identifier
    ;

parameter_identifier 
    : identifier
    ;

variable_identifier
: identifier           { $$.reset(new netlist::VIdentifier(*$1)); }
    | variable_identifier '[' range_expression ']'
    ;

port_identifier
    : identifier             
    ;

