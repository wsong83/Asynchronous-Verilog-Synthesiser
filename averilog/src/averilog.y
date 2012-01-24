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
  vector<std::string>         tIDList;		/* a list of IDs */
  std::string                 tID;     		/* identifier */
  vector<AVNetParameter>      tParaList;	/* parameter list */
  AVNetParameter              tPara;		/* a single parameter assign */
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
%token oNeq         "!="
%token oLLsh        "<<"
%token oLRsh        ">>"
%token oALsh        "<<<"
%token oARsh        ">>>"
%token oNColon      "-:"
%token oPColon      "+:"
%token oRNand       "~&"
%token oRNor        "~|"
%token oRXnor       "~^"
%token oRXnor1      "^~"
%token oPos
%token oNeg
%token oLNot
%token oNot

 // predence
%right '?' ':'
%left  "||"
%left  "&&"
%left  '|' "~|"
%left  '^' "~^" "^~"
%left  '&' "~&"
%left  "==" "!="
%left  '>' '<' "<=" ">="
%left  "<<" ">>" "<<<" ">>>"
%left  '+' '-'
%left  '*' '/' '%'
%left  "**"
%left  oPos oNeg oLNot oNot

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
%token id

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
    : kModule id module_parameter_port_list
        '(' list_of_ports ')' ';'
        module_items
      kEndmodule
    | kModule id module_parameter_port_list
        module_items
      kEndmodule                   
    ;

// A.1.4 Module paramters and ports
module_parameter_port_list
    : /* empty */   
    | '#' '(' ')'   
    | '#' '(' parameter_declarations ')' 
    ;

parameter_declarations
    : parameter_declaration              
    | parameter_declarations ',' parameter_declaration 
    ;


////////////////////////////////////////////////////////////////////////

// port list, not fully supported yet
list_of_ports
    : /* empty */
    | identifier_list                  
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
    | parameter_declaration ';'
//    | specify_block
//    | specparam_declaration
    ;

non_port_module_item
    : module_or_generate_item
    | generated_instantiation
//    | local_parameter_declaration
    | parameter_declaration ';'
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
    : net_declaration ';'
    | reg_declaration
    | integer_declaration ';'
//    | real_declaration
//    | time_declaration
//    | realtime_declaration
//    | event_declaration
    | genvar_declaration ';'
//    | task_declaration
    | function_declaration
    ;

// A.2.1 Declaration types
// A.2.1.1 Module parameter declarations
parameter_declaration
    : avParameter list_of_param_assignments
    ;

// A.2.1.2 Port declarations
input_declaration 
    : avInput list_of_port_identifiers
    | avInput range_declaration list_of_port_identifiers
    ;

output_declaration 
    : avOutput list_of_port_identifiers
    | avOutput range_declaration list_of_port_identifiers
    ;

// A.2.1.3 Type declarations
genvar_declaration 
    : avGenvar list_of_genvar_identifiers
    ;

integer_declaration
    : avInteger list_of_variable_identifiers
    ;

net_declaration 
    : net_type list_of_net_identifiers
    | net_type range list_of_net_identifiers
    ;

net_type
    : avWire
    ;

reg_declaration 
    : avReg list_of_variable_identifiers
    | avReg range list_of_variable_identifiers
    ;

// A.2.3 Declaration lists
list_of_genvar_identifiers 
    : genvar_identifier 
    | list_of_genvar_identifiers ',' genvar_identifier
    ;

//list_of_net_decl_assignments ::= net_decl_assignment { , net_decl_assignment }

list_of_net_identifiers
    : net_identifier 
    | net_identifier dimensions
    | list_of_net_identifiers ',' net_identifier
    | list_of_net_identifiers ',' net_identifier dimentions
    ;

// prepare for multi-dimention
dimentions
    : dimention
    | dimentions dimention
    ;

list_of_param_assignments 
    : param_assignment
    | list_of_param_assignments ',' param_assignment
    ;

list_of_port_identifiers 
    : port_identifier 
    | list_of_port_identifier ',' port_identifier
    ;

//list_of_real_identifiers ::= real_type { , real_type }
//list_of_specparam_assignments ::= specparam_assignment { , specparam_assignment }
//list_of_variable_identifiers ::= variable_type { , variable_type }
//list_of_variable_port_identifiers ::= port_identifier [ = constant_expression ] { , port_identifier [ = constant_expression ] }

// A.2.4 Declaration assignments
//net_decl_assignment ::= net_identifier = expression

param_assignment 
    : parameter_identifier '=' constant_expression
    ;

//specparam_assignment ::= specparam_identifier = constant_mintypmax_expression | pulse_control_specparam
//pulse_control_specparam ::= PATHPULSE$ = ( reject_limit_value [ , error_limit_value ] ) ; 
//| PATHPULSE$specify_input_terminal_descriptor$specify_output_terminal_descriptor
//= ( reject_limit_value [ , error_limit_value ] ) ;
//error_limit_value ::= limit_value
//reject_limit_value ::= limit_value
//limit_value ::= constant_mintypmax_expression

//A.2.5 Declaration ranges
dimension 
    : '[' dimension_constant_expression ':' dimension_constant_expression ']'
    ;

range
    : '[' msb_constant_expression ':' lsb_constant_expression ']'
    ;

//A.2.6 Function declarations
function_declaration
    : avFunction function_identifier ';'
        list_of_function_item_declaration
        function_statement
      avEndfunction
    | avFunction avAutomatic function_identifier ';'
        list_of_function_item_declaration
        function_statement
      avEndfunction
    | avFunction function_identifier '(' function_port_list ')' ';'
        list_of_function_item_declaration
        function_statement
      avEndfunction
    | avFunction avAutomatic function_identifier '(' function_port_list ')' ';'
        list_of_function_item_declaration
        function_statement
      avEndfunction
    ;

list_of_function_item_declaration
    : function_item_declaration
    | list_of_function_item_declaration function_item_declaration
    ;

function_item_declaration 
    : block_item_declaration
    | tf_input_declaration
    ;

function_port_list 
    : tf_input_declaration
    | function_port_list ',' tf_input_declaration
    ;

//A.2.8 Block item declarations
block_item_declaration 
    : block_reg_declaration
//| { attribute_instance } event_declaration
    | integer_declaration
//| { attribute_instance } local_parameter_declaration
    | parameter_declaration
//| { attribute_instance } real_declaration
//| { attribute_instance } realtime_declaration
//| { attribute_instance } time_declaration
    ;

block_reg_declaration 
    : avReg list_of_block_variable_identifiers
    | avReg range list_of_block_variable_identifiers
    ;

list_of_block_variable_identifiers 
    : block_variable_type 
    | list_of_block_variable_identifiers ',' block_variable_type
    ;

block_variable_type 
    : variable_identifier
    | variable_identifier dimensions
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
    : name_of_instance '(' ')'
    | name_of_instance '(' list_of_port_connections ')'
    ;

name_of_instance 
    : module_instance_identifier 
    | module_instance_identifier range
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
    : avGenerate generate_item avEndgenerate
    ;

generate_item_or_null
    : /* empty */
    | generate_item
    ;

generate_item 
    : generate_conditional_statement
    | generate_case_statement
    | generate_loop_statement
    | generate_block
    | module_or_generate_item
    ;

generate_conditional_statement 
    : avIf '(' constant_expression ')' generate_item_or_null 
    | avIf '(' constant_expression ')' generate_item_or_null avElse generate_item_or_null
    ;

generate_case_statement 
    : avCase '(' constant_expression ')' genvar_case_items avEndcase
    ;
genvar_case_items
    : genvar_case_item
    | genvar_case_items genvar_case_item
    ;

genvar_case_item 
    : constant_expressions ':' generate_item_or_null 
    | avDefault : generate_item_or_null
    | avDefault generate_item_or_null
    ;

constant_expressions
    : constant_expression
    | constant_expressions ',' constant_expression
    ;

generate_loop_statement 
    : avFor '(' genvar_assignment ';' constant_expression ';' genvar_assignment ')' avBegin ':' generate_block_identifier generate_item_or_null avEnd
    ;

genvar_assignment 
    : genvar_identifier '=' constant_expression
    ;

generate_block 
    | avBegin generate_item_or_null avEnd
    : avBegin ':' generate_block_identifier  generate_item_or_null avEnd
    ;

//A.6.1 Continuous assignment statements
continuous_assign 
    : avAssign list_of_net_assignments ';'
    | avAssign delay3 list_of_net_assignments ';'
    ;

list_of_net_assignments 
    : net_assignment 
    | list_of_net_assignments ',' net_assignment
    ;

net_assignment 
    : net_lvalue '=' expression
    ;

//A.6.2 Procedural blocks and assignments
//initial_construct ::= initial statement
always_construct 
    : avAlways statement
    ;

blocking_assignment 
    : variable_lvalue '=' expression
    ;

nonblocking_assignment 
    : variable_lvalue avOpNbassign expression
    | variable_lvalue avOpNbassign delay_or_event_control expression
    ;

//procedural_continuous_assignments ::=
//assign variable_assignment
//| deassign variable_lvalue
//| force variable_assignment
//| force net_assignment
//| release variable_lvalue
//| release net_lvalue
//function_blocking_assignment ::= variable_lvalue = expression
//function_statement_or_null ::=
//function_statement
//| { attribute_instance } ;


//A.6.4 Statements
statement
    : blocking_assignment ';'
    | case_statement
    | conditional_statement
//| { attribute_instance } disable_statement
//| { attribute_instance } event_trigger
    | loop_statement
    | nonblocking_assignment ';'
//| { attribute_instance } par_block
//| { attribute_instance } procedural_continuous_assignments ;
//| { attribute_instance } procedural_timing_control_statement
//| { attribute_instance } seq_block
//| { attribute_instance } system_task_enable
//| { attribute_instance } task_enable
//| { attribute_instance } wait_statement
    ;

statement_or_null 
    : /* empty */
    | statement
    ;

function_statement 
    : function_blocking_assignment ';'
    | function_case_statement
    | function_conditional_statement
    | function_loop_statement
//| { attribute_instance } function_seq_block
//| { attribute_instance } disable_statement
//| { attribute_instance } system_task_enable
    ;


//A.6.5 Timing control statements
delay_control 
    : '#' delay_value
    | '#' '(' mintypmax_expression ')'
    ;

delay_or_event_control 
    : delay_control
//| event_control
//| repeat ( expression ) event_control
    ;

//disable_statement ::=
//disable hierarchical_task_identifier ;
//| disable hierarchical_block_identifier ;
//event_control ::=
//@ event_identifier
//| @ ( event_expression )
//| @*
//| @ (*)
//event_trigger ::=
//-> hierarchical_event_identifier ;
//event_expression ::=
//expression
//| hierarchical_identifier
//| posedge expression
//| negedge expression
//| event_expression or event_expression
//| event_expression , event_expression
//procedural_timing_control_statement ::=
//delay_or_event_control statement_or_null
//wait_statement ::=
//wait ( expression ) statement_or_null

//A.6.6 Conditional statements
conditional_statement 
    : avIf '(' expression ')' statement_or_null 
    | avIf '(' expression ')' statement_or_null avElse statement_or_null
    | avIf '(' expression ')' statement_or_null if_else_if_statements avElse statement_or_null
    ;

if_else_if_statements 
    : avElse avIf '(' expression ')' statement_or_null
    | if_else_if_statements avElse avIf '(' expression ')' statement_or_null
    ;

function_conditional_statement 
    : avIf '(' expression ')' statement_or_null 
    | avIf '(' expression ')' statement_or_null avElse statement_or_null
    | avIf '(' expression ')' statement_or_null function_if_else_if_statements avElse statement_or_null
    ;

function_if_else_if_statements 
    : avElse avIf '(' expression ')' statement_or_null
    | function_if_else_if_statements avElse avIf '(' expression ')' statement_or_null
    ;

//A.6.7 Case statements
case_statement 
    : avCase '(' expression ')' case_items avEndcase
    ;

case_items
    : case_item
    | case_items case_item
    ;

case_item 
    : expressions ':' statement_or_null
    | avDefault ':' statement_or_null
    | avDefault statement_or_null
    ;

expressions
    : expression
    | expressions ',' expression
    ;

function_case_statement ::=
case ( expression )
function_case_item { function_case_item } endcase
| casez ( expression )
function_case_item { function_case_item } endcase
| casex ( expression )
function_case_item { function_case_item } endcase
function_case_item ::=
expression { , expression } : function_statement_or_null
| default [ : ] function_statement_or_null


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
