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
%parse-param {shell::Env& av_env}
%lex-param {yyscan_t avscanner}
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
#include <boost/foreach.hpp>
  using namespace netlist;
  using namespace averilog;
  using std::string;
  using std::vector;
  using boost::shared_ptr;
  using std::list;
  using std::endl;
  using std::cout;
  using std::map;
  using std::pair;
  using std::vector;


#define yylex avlex
  
  yyscan_t avscanner;

  void averilog::av_parser::error (const location_type& loc, const string& msg) {
    av_env.error(loc, "PARSER-0");
    cout << msg << endl;
  }
  
#define Lib (*(av_env.curLib))

%}


%initial-action
{
  @$.initialize(fname);
  avlex_init_extra(&av_env, &avscanner);
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
%token kCasex          "casex"        
%token kCasez          "casez"        /* not supported yet */
%token kCell           "cell"         /* not supported yet */
%token kConfig         "config"       /* not supported yet */
%token kDeassign       "deassign"     /* not supported yet */
%token kDefault        "default"      
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
%token kSigned         "signed"
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
%token kWhile          "while" 
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
%type <tAssign>         blocking_assignment
%type <tAssign>         nonblocking_assignment
%type <tBlock>          statements
%type <tBlock>          statement
%type <tBlock>          statement_or_null 
%type <tBlock>          generate_item
%type <tBlock>          generate_items
%type <tBlock>          generate_item_or_null
%type <tBlock>          module_item
%type <tBlock>          module_items
%type <tBlockName>      block_identifier
%type <tCaseItem>       case_item
%type <tCaseItem>       generate_case_item
%type <tConcatenation>  concatenation
%type <tExp>            expression
%type <tExp>            primary
%type <tEvent>          event_expression
%type <tGenBlock>       generated_instantiation
%type <tInstance>       module_instance
%type <tInstance>       n_input_gate_instance
%type <tInstance>       n_output_gate_instance
%type <tInstName>       instance_identifier
%type <tLConcatenation> variable_lvalue
%type <tListAssign>     continuous_assign
%type <tListAssign>     list_of_net_assignments
%type <tListCaseItem>   case_items
%type <tListCaseItem>   generate_case_items
%type <tListExp>        expressions
%type <tListEvent>      event_expressions
%type <tListInst>       gate_instantiation
%type <tListInst>       module_instances
%type <tListInst>       module_instantiation
%type <tListInst>       n_input_gate_instances
%type <tListInst>       n_output_gate_instances
%type <tListParaAssign> list_of_parameter_assignments 
%type <tListParaAssign> named_parameter_assignments
%type <tListParaAssign> ordered_parameter_assignments
%type <tListPortConn>   input_terminals
%type <tListPortConn>   list_of_port_connections
%type <tListPortConn>   named_port_connections
%type <tListPortConn>   ordered_port_connections
%type <tListPortConn>   output_terminals
%type <tListPortDecl>   input_declaration
%type <tListPortDecl>   inout_declaration
%type <tListPortDecl>   list_of_port_identifiers
%type <tListPortDecl>   output_declaration
%type <tListVar>        list_of_variable_identifiers
%type <tListVarDecl>    list_of_variable_declarations
%type <tListVarDecl>    variable_declaration
%type <tListVarDecl>    list_of_param_assignments
%type <tListVarDecl>    parameter_declaration
%type <tModuleName>     module_identifier
%type <tModuleName>     n_input_gatetype
%type <tModuleName>     n_output_gatetype
%type <tParaAssign>     named_parameter_assignment
%type <tParaAssign>     ordered_parameter_assignment
%type <tPortName>       port_identifier
%type <tPortConn>       named_port_connection
%type <tPortConn>       ordered_port_connection
%type <tRange>          range_expression
%type <tSeqBlock>       always_construct
%type <tVarName>        parameter_identifier
%type <tVarName>        variable_identifier

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
    : "module" module_identifier ';' module_items "endmodule"                   
    {
      shared_ptr<Module> m(new Module(@$, $2, $4));
      if(!Lib.insert(m)) av_env.error(@$, "SYN-MODULE-0", $2.name); 
      av_env.error("SYN-MODULE-3", m->name.name, Lib.name);
      //cout<< *m;
    }
    | "module" module_identifier '(' ')' ';' module_items "endmodule"                   
    {
      shared_ptr<Module> m(new Module(@$, $2, $6));
      if(!Lib.insert(m)) av_env.error(@$, "SYN-MODULE-0", $2.name); 
      av_env.error("SYN-MODULE-3", m->name.name, Lib.name);
      //cout<< *m;
    }
    | "module" module_identifier '(' list_of_port_identifiers ')' ';' module_items "endmodule"
    { 
      shared_ptr<Module> m(new Module(@$, $2, $4, $7));
      if(!Lib.insert(m)) av_env.error(@$, "SYN-MODULE-0", $2.name); 
      av_env.error("SYN-MODULE-3", m->name.name, Lib.name);
      //cout<< *m;
    }
    | "module" module_identifier '#' '(' parameter_declaration ')' '(' list_of_port_identifiers ')' ';'
      module_items "endmodule"
    {
      shared_ptr<Module> m(new Module(@$, $2, $5, $8, $11));
      if(!Lib.insert(m)) av_env.error(@$, "SYN-MODULE-0", $2.name); 
      av_env.error("SYN-MODULE-3", m->name.name, Lib.name);
      //cout<< *m;
    }
    ;

// A.1.4 Module paramters and ports

// A.1.5 Module items
module_item
    : parameter_declaration ';'  { $$.reset(new Block()); $$->add_list<Variable>($1); }
    | input_declaration ';'      { $$.reset(new Block()); $$->add_list<Port>($1);     }
    | output_declaration ';'     { $$.reset(new Block()); $$->add_list<Port>($1);     }
    | inout_declaration ';'      { $$.reset(new Block()); $$->add_list<Port>($1);     }
    | variable_declaration ';'   { $$.reset(new Block()); $$->add_list<Variable>($1); }
    | function_declaration
    | continuous_assign          { $$.reset(new Block()); $$->add_list<Assign>($1);   }
    | gate_instantiation         { $$.reset(new Block()); $$->add_list<Instance>($1); }
    | module_instantiation       { $$.reset(new Block()); $$->add_list<Instance>($1); }
    | always_construct           { $$.reset(new Block()); $$->add($1);                }
    | generated_instantiation    { $$.reset(new Block()); $$->add($1);                }
    | statement                  { $$.reset(new Block()); av_env.error(@$, "SYN-MODULE-1"); }
    | error                      { $$.reset(new Block()); av_env.error(@$, "SYN-MODULE-1"); }
    ;

module_items
    : /* empty */                { $$.reset(new Block());                             }
    | module_item                { $$.reset(new Block()); $$->add_statements($1);     }
    | module_items module_item   { $$->add_statements($2);                            }
    
// A.2.1 Declaration types
// A.2.1.1 Module parameter declarations
parameter_declaration
    : "parameter" list_of_param_assignments { $$ = $2; }
    ;

// A.2.1.2 Port declarations
input_declaration 
    : "input" list_of_port_identifiers
    {
      bool undired = true;
      BOOST_FOREACH(shared_ptr<Port> it, $2) {
        if(undired && it->get_dir() == -2) {
          it->set_in();
        } else {
          undired = false;
        }
        $$.push_back(it);
      }
    }
    | "input" '[' expression ':' expression ']' list_of_port_identifiers
    {      
      bool undired = true;
      BOOST_FOREACH(shared_ptr<Port> it, $7) {
        if(undired && it->get_dir() == -2) {
          it->set_in();
          pair<shared_ptr<Expression>, shared_ptr<Expression> > m($3, $5);
          it->name.get_range().add_low_dimension(shared_ptr<Range>(new Range(@2+@6, m)));
        } else {
          undired = false;
        }
        $$.push_back(it);
      }
    }  
    | "input" "signed" '[' expression ':' expression ']' list_of_port_identifiers
    {      
      bool undired = true;
      BOOST_FOREACH(shared_ptr<Port> it, $8) {
        if(undired && it->get_dir() == -2) {
          it->set_in();
          pair<shared_ptr<Expression>, shared_ptr<Expression> > m($4, $6);
          it->name.get_range().add_low_dimension(shared_ptr<Range>(new Range(@3+@7, m)));
        } else {
          undired = false;
        }
        it->set_signed();
        $$.push_back(it);
      }
    }  
    ;

output_declaration 
    : "output" list_of_port_identifiers
    {
      bool undired = true;
      BOOST_FOREACH(shared_ptr<Port> it, $2) {
        if(undired && it->get_dir() == -2) {
          it->set_out();
        } else {
          undired = false;
        }
        $$.push_back(it);
      }
    }
    | "output" '[' expression ':' expression ']' list_of_port_identifiers
    {
      bool undired = true;
      BOOST_FOREACH(shared_ptr<Port> it, $7) {
        if(undired && it->get_dir() == -2) {
          it->set_out();
          pair<shared_ptr<Expression>, shared_ptr<Expression> > m($3, $5);
          it->name.get_range().add_low_dimension(shared_ptr<Range>(new Range(@2+@6,m)));
        } else {
          undired = false;
        }
        $$.push_back(it);
      }
    }  
    | "output" "signed" '[' expression ':' expression ']' list_of_port_identifiers
    {
      bool undired = true;
      BOOST_FOREACH(shared_ptr<Port> it, $8) {
        if(undired && it->get_dir() == -2) {
          it->set_out();
          pair<shared_ptr<Expression>, shared_ptr<Expression> > m($4, $6);
          it->name.get_range().add_low_dimension(shared_ptr<Range>(new Range(@3+@7,m)));
        } else {
          undired = false;
        }
        it->set_signed();
        $$.push_back(it);
      }
    }  
    | "output" "reg" list_of_port_identifiers
    {
      bool undired = true;
      BOOST_FOREACH(shared_ptr<Port> it, $3) {
        if(undired && it->get_dir() == -2) {
          it->set_out();
          it->ptype = 1;          /* reg */
        } else {
          undired = false;
        }
        $$.push_back(it);
      }
    }
    | "output" "reg" '[' expression ':' expression ']' list_of_port_identifiers
    {
      bool undired = true;
      BOOST_FOREACH(shared_ptr<Port> it, $8) {
        if(undired && it->get_dir() == -2) {
          it->set_out();
          it->ptype = 1;          /* reg */
          pair<shared_ptr<Expression>, shared_ptr<Expression> > m($4, $6);
          it->name.get_range().add_low_dimension(shared_ptr<Range>(new Range(@3+@7,m)));
        } else {
          undired = false;
        }
        $$.push_back(it);
      }
    }  
    | "output" "reg" "signed" '[' expression ':' expression ']' list_of_port_identifiers
    {
      bool undired = true;
      BOOST_FOREACH(shared_ptr<Port> it, $9) {
        if(undired && it->get_dir() == -2) {
          it->set_out();
          it->ptype = 1;          /* reg */
          pair<shared_ptr<Expression>, shared_ptr<Expression> > m($5, $7);
          it->name.get_range().add_low_dimension(shared_ptr<Range>(new Range(@4+@8,m)));
        } else {
          undired = false;
        }
        it->set_signed();
        $$.push_back(it);
      }
    }  
    | "output" "wire" list_of_port_identifiers
    {
      bool undired = true;
      BOOST_FOREACH(shared_ptr<Port> it, $3) {
        if(undired && it->get_dir() == -2) {
          it->set_out();
          it->ptype = 0;          /* reg */
        } else {
          undired = false;
        }
        $$.push_back(it);
      }
    }
    | "output" "wire" '[' expression ':' expression ']' list_of_port_identifiers
    {
      bool undired = true;
      BOOST_FOREACH(shared_ptr<Port> it, $8) {
        if(undired && it->get_dir() == -2) {
          it->set_out();
          it->ptype = 0;          /* reg */
          pair<shared_ptr<Expression>, shared_ptr<Expression> > m($4, $6);
          it->name.get_range().add_low_dimension(shared_ptr<Range>(new Range(@3+@7,m)));
        } else {
          undired = false;
        }
        $$.push_back(it);
      }
    }  
    | "output" "wire" "signed" '[' expression ':' expression ']' list_of_port_identifiers
    {
      bool undired = true;
      BOOST_FOREACH(shared_ptr<Port> it, $9) {
        if(undired && it->get_dir() == -2) {
          it->set_out();
          it->ptype = 0;          /* reg */
          pair<shared_ptr<Expression>, shared_ptr<Expression> > m($5, $7);
          it->name.get_range().add_low_dimension(shared_ptr<Range>(new Range(@4+@8,m)));
        } else {
          undired = false;
        }
        it->set_signed();
        $$.push_back(it);
      }
    }  
    ;

inout_declaration 
    : "inout" list_of_port_identifiers
    {
      bool undired = true;
      BOOST_FOREACH(shared_ptr<Port> it, $2) {
        if(undired && it->get_dir() == -2) {
          it->set_inout();
        } else {
          undired = false;
        }
        $$.push_back(it);
      }
    }
    | "inout" '[' expression ':' expression ']' list_of_port_identifiers
    {
      bool undired = true;
      BOOST_FOREACH(shared_ptr<Port> it, $7) {
        if(undired && it->get_dir() == -2) {
          it->set_inout();
          pair<shared_ptr<Expression>, shared_ptr<Expression> > m($3, $5);
          it->name.get_range().add_low_dimension(shared_ptr<Range>(new Range(@2+@6,m)));
        } else {
          undired = false;
        }
        $$.push_back(it);
      }
    }  

// A.2.1.3 Type declarations
variable_declaration 
    : "wire" list_of_variable_identifiers 
    {
      list<pair<VIdentifier, shared_ptr<Expression> > >::iterator it, end;
      for(it=$2.begin(), end=$2.end(); it!=end; it++){
        $$.push_back(shared_ptr<Variable>(new Variable(it->first.loc, 
                                                       it->first, it->second, Variable::TWire)));
        $$.back()->name.get_range() = $$.back()->name.get_select();
        $$.back()->name.get_select().clear();
        $$.back()->name.get_range().set_dim();
      }
    }
    | "wire" '[' expression ':' expression ']' list_of_variable_identifiers
    {
      list<pair<VIdentifier, shared_ptr<Expression> > >::iterator it, end;
      for(it=$7.begin(), end=$7.end(); it!=end; it++){
        $$.push_back(shared_ptr<Variable>(new Variable(it->first.loc, 
                                                       it->first, it->second, Variable::TWire)));
        $$.back()->name.get_range() = $$.back()->name.get_select();
        $$.back()->name.get_select().clear();
        $$.back()->name.get_range().set_dim();
        pair<shared_ptr<Expression>, shared_ptr<Expression> > m($3, $5);
        $$.back()->name.get_range().add_low_dimension(shared_ptr<Range>(new Range(@2+@6, m)));
      }
    }
    | "wire" "signed" '[' expression ':' expression ']' list_of_variable_identifiers
    {
      list<pair<VIdentifier, shared_ptr<Expression> > >::iterator it, end;
      for(it=$8.begin(), end=$8.end(); it!=end; it++){
        $$.push_back(shared_ptr<Variable>(new Variable(it->first.loc, 
                                                       it->first, it->second, Variable::TWire)));
        $$.back()->name.get_range() = $$.back()->name.get_select();
        $$.back()->name.get_select().clear();
        $$.back()->name.get_range().set_dim();
        pair<shared_ptr<Expression>, shared_ptr<Expression> > m($4, $6);
        $$.back()->name.get_range().add_low_dimension(shared_ptr<Range>(new Range(@3+@7, m)));
        $$.back()->set_signed();
      }
    }
    | "reg" list_of_variable_identifiers 
    {
      list<pair<VIdentifier, shared_ptr<Expression> > >::iterator it, end;
      for(it=$2.begin(), end=$2.end(); it!=end; it++){
        $$.push_back(shared_ptr<Variable>(new Variable(it->first.loc, 
                                                       it->first, it->second, Variable::TReg)));
        $$.back()->name.get_range() = $$.back()->name.get_select();
        $$.back()->name.get_select().clear();
        $$.back()->name.get_range().set_dim();
      }
    }
    | "reg" '[' expression ':' expression ']' list_of_variable_identifiers
    {
      list<pair<VIdentifier, shared_ptr<Expression> > >::iterator it, end;
      for(it=$7.begin(), end=$7.end(); it!=end; it++){
        $$.push_back(shared_ptr<Variable>(new Variable(it->first.loc, 
                                                       it->first, it->second, Variable::TReg)));
        $$.back()->name.get_range() = $$.back()->name.get_select();
        $$.back()->name.get_select().clear();
        vector<shared_ptr<Range> >::iterator rg_it, rg_end;
        $$.back()->name.get_range().set_dim();
        pair<shared_ptr<Expression>, shared_ptr<Expression> > m($3, $5);
        $$.back()->name.get_range().add_low_dimension(shared_ptr<Range>(new Range(@2+@6, m)));
      }
    }
    | "reg" "signed" '[' expression ':' expression ']' list_of_variable_identifiers
    {
      list<pair<VIdentifier, shared_ptr<Expression> > >::iterator it, end;
      for(it=$8.begin(), end=$8.end(); it!=end; it++){
        $$.push_back(shared_ptr<Variable>(new Variable(it->first.loc, 
                                                       it->first, it->second, Variable::TReg)));
        $$.back()->name.get_range() = $$.back()->name.get_select();
        $$.back()->name.get_select().clear();
        vector<shared_ptr<Range> >::iterator rg_it, rg_end;
        $$.back()->name.get_range().set_dim();
        pair<shared_ptr<Expression>, shared_ptr<Expression> > m($4, $6);
        $$.back()->name.get_range().add_low_dimension(shared_ptr<Range>(new Range(@3+@7, m)));
        $$.back()->set_signed();
      }
    }
    | "genvar" list_of_variable_identifiers
    {
      list<pair<VIdentifier, shared_ptr<Expression> > >::iterator it, end;
      for(it=$2.begin(), end=$2.end(); it!=end; it++){
        $$.push_back(shared_ptr<Variable>(new Variable(it->first.loc, 
                                                       it->first, it->second, Variable::TGenvar)));
        $$.back()->name.get_range() = $$.back()->name.get_select();
        $$.back()->name.get_select().clear();
        $$.back()->name.get_range().set_dim();
      }
    }
    | "integer" list_of_variable_identifiers
    {
      list<pair<VIdentifier, shared_ptr<Expression> > >::iterator it, end;
      for(it=$2.begin(), end=$2.end(); it!=end; it++){
        $$.push_back(shared_ptr<Variable>(new Variable(it->first.loc, 
                                                       it->first, it->second, Variable::TReg)));
        $$.back()->name.get_range() = $$.back()->name.get_select();
        $$.back()->name.get_select().clear();
        $$.back()->name.get_range().set_dim();
        $$.back()->name.get_range().add_low_dimension(shared_ptr<Range>(new Range(31, 0)));
      }
    }
    ;

list_of_variable_declarations
    : variable_declaration ';'                                { $$ = $1;                  }
    | list_of_variable_declarations  variable_declaration ';' { $$.splice($$.end(), $2); }
    ;

// A.2.3 Declaration lists
list_of_param_assignments 
    : parameter_identifier '=' expression
    { $$.push_back(shared_ptr<Variable>(new Variable(@$, $1,$3,Variable::TParam))); }
    | list_of_param_assignments ',' parameter_identifier '=' expression
    { $$.push_back(shared_ptr<Variable>(new Variable(@3+@5, $3,$5,Variable::TParam))); }
    ;

list_of_port_identifiers 
    : port_identifier
    { 
      $$.push_back(shared_ptr<Port>(new Port($1.loc, $1))); 
    }
    | input_declaration
    { $$ = $1; }
    | output_declaration
    { $$ = $1; }
    | list_of_port_identifiers ',' port_identifier 
    { 
      $$.push_back(shared_ptr<Port>(new Port($3.loc, $3))); 
    }
    | list_of_port_identifiers ',' input_declaration
    { 
      $$.insert($$.end(), $3.begin(), $3.end()); 
    }
    | list_of_port_identifiers ',' output_declaration
    { 
      $$.insert($$.end(), $3.begin(), $3.end()); 
    }
    | error
    ;


list_of_variable_identifiers 
    : variable_identifier
    { $$.push_back(pair<VIdentifier,shared_ptr<Expression> >($1, shared_ptr<Expression>())); }
    | variable_identifier '=' expression
    { $$.push_back(pair<VIdentifier,shared_ptr<Expression> >($1, $3)); } 
    | list_of_variable_identifiers ',' variable_identifier 
    { $$.push_back(pair<VIdentifier,shared_ptr<Expression> >($3, shared_ptr<Expression>())); }
    | list_of_variable_identifiers ',' variable_identifier '=' expression 
    { $$.push_back(pair<VIdentifier,shared_ptr<Expression> >($3, $5)); }
    ;

// A.2.4 Declaration assignments
//param_assignment 
//    : parameter_identifier '=' expression { $$.reset(new Variable($1,$3,Variable::TParam)); }
//    ;

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
    | "function" function_identifier '(' list_of_port_identifiers ')' ';'
        list_of_function_item_declaration
        statement
      "endfunction"
    | "function" "automatic" function_identifier '(' list_of_port_identifiers ')' ';'
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
    {
      BOOST_FOREACH(shared_ptr<Instance>& it, $2)
        it->set_mname($1);
      $$ = $2;
    }
    | n_output_gatetype n_output_gate_instances ';'
    {
      BOOST_FOREACH(shared_ptr<Instance>& it, $2)
        it->set_mname($1);
      $$ = $2;
    }
    ;

n_input_gate_instances
    : n_input_gate_instance                              { $$.push_back($1); }
    | n_input_gate_instances ',' n_input_gate_instance   { $$.push_back($3); }
    ;

n_input_gate_instance
    : '(' variable_lvalue ',' input_terminals ')'
    {
      // push the lvalue into port list
      $4.push_front(shared_ptr<PortConn>(new PortConn(@2, $2, 1)));
      $$.reset( new Instance(@$, $4,  Instance::prim_in_inst));
    }
    | instance_identifier '(' variable_lvalue ',' input_terminals ')'
    {
      $5.push_front(shared_ptr<PortConn>(new PortConn(@3, $3, 1)));
      $$.reset( new Instance(@$, $1, $5, Instance::prim_in_inst));
    }
    | instance_identifier '[' expression ':' expression ']' '(' variable_lvalue ',' input_terminals ')'
    ;

input_terminals
    : expression
    { $$.push_back(shared_ptr<PortConn>(new PortConn(@$, $1, -1))); }
    | input_terminals ',' expression
    { $$.push_back(shared_ptr<PortConn>(new PortConn(@3, $3, -1))); }
    ;

n_output_gate_instances
    : n_output_gate_instance                               { $$.push_back($1); }
    | n_output_gate_instances ',' n_output_gate_instance   { $$.push_back($3); }
    ;

n_output_gate_instance
    : '(' output_terminals ',' expression ')'
    {
      // push the expression into port list
      $2.push_back(shared_ptr<PortConn>(new PortConn(@$, $4, -1)));
      $$.reset( new Instance(@$, $2,  Instance::prim_out_inst));
    }
    | instance_identifier '(' output_terminals ',' expression ')'
    {
      $3.push_back(shared_ptr<PortConn>(new PortConn(@5, $5, -1)));
      $$.reset( new Instance(@$, $1, $3, Instance::prim_out_inst));
    }
    | instance_identifier '[' expression ':' expression ']' '(' output_terminals ',' expression ')'
    ;

output_terminals
    : variable_lvalue                      
    { $$.push_back(shared_ptr<PortConn>(new PortConn(@$, $1, 1))); }
    | output_terminals ',' variable_lvalue 
    { $$.push_back(shared_ptr<PortConn>(new PortConn(@3, $3, 1))); }
    ;

//A.3.4 Primitive gate and switch types
n_input_gatetype
    : "and"        { $$ = string("and"); }
    | "nand"       { $$ = string("nand"); }
    | "or"         { $$ = string("or"); }
    | "nor"        { $$ = string("nor"); }
    | "xor"        { $$ = string("xor"); }
    | "xnor"       { $$ = string("xnor"); }
    ;

n_output_gatetype
    : "buf"        { $$ = string("buf"); }
    | "not"        { $$ = string("not"); }
    ;

//A.4.1 Module instantiation
module_instantiation 
    : module_identifier module_instances ';'
    {
      BOOST_FOREACH(shared_ptr<Instance>& it, $2)
        it->set_mname($1);
      $$ = $2;
    }
    | module_identifier '#' '(' list_of_parameter_assignments ')' module_instances ';'
    {
      BOOST_FOREACH(shared_ptr<Instance>& it, $6) {
        it->set_mname($1);
        it->set_para($4);
      }
      $$ = $6;
    }
    ;

module_instances
    : module_instance                      { $$.push_back($1); }
    | module_instances ',' module_instance { $$.push_back($3); }
    ;

list_of_parameter_assignments 
    : ordered_parameter_assignments
    | named_parameter_assignments
    ;
  
ordered_parameter_assignments
    : ordered_parameter_assignment                                    { $$.push_back($1); }
    | ordered_parameter_assignments ',' ordered_parameter_assignment  { $$.push_back($3); }
    ;

named_parameter_assignments
    : named_parameter_assignment                                  { $$.push_back($1); }
    | named_parameter_assignments ',' named_parameter_assignment  { $$.push_back($3); }
    ;

ordered_parameter_assignment 
    : expression              { $$.reset( new ParaConn(@$, $1)); }
    ;

named_parameter_assignment 
    : '.' parameter_identifier '('  ')'           { $$.reset( new ParaConn(@$, $2)); }
    | '.' parameter_identifier '(' expression ')' { $$.reset( new ParaConn(@$, $2, $4)); }
    ;

module_instance 
    : instance_identifier '(' ')' { $$.reset(new Instance(@$, $1, list<shared_ptr<PortConn> >())); }
    | instance_identifier '[' expression ':' expression ']' '(' ')'
    | instance_identifier '(' list_of_port_connections ')'   { $$.reset(new Instance(@$, $1, $3)); }
    | instance_identifier '[' expression ':' expression ']' '(' list_of_port_connections ')'
    ;

list_of_port_connections 
    : ordered_port_connections
    | named_port_connections  
    ;

ordered_port_connections
    : ordered_port_connection                                { $$.push_back($1); }
    | ordered_port_connections ',' ordered_port_connection   { $$.push_back($3); }
    ;

named_port_connections
    : named_port_connection                             { $$.push_back($1); }
    | named_port_connections ',' named_port_connection  { $$.push_back($3); }
    ;

ordered_port_connection 
    : /* empty */             { $$.reset(); }
    | expression              { $$.reset( new PortConn(@$, $1)); }
    ;

named_port_connection 
    : '.' port_identifier '(' ')' { $$.reset( new PortConn(@$, $2)); }
    | '.' port_identifier '(' expression ')'  { $$.reset( new PortConn(@$, $2, $4));}
    ;

//A.4.2 Generated instantiation
generated_instantiation 
    : "generate" generate_items "endgenerate"   { $$.reset(new GenBlock(@$, *$2)); }
    ;

generate_items
    : generate_item                 { $$.reset(new Block()); $$->add_statements($1); }
    | generate_items generate_item  { $$->add_statements($2);                        }
    ;

generate_item_or_null
    : /* empty */        { $$.reset(new Block()); }
    | generate_item      { $$ = $1; }
    ;

generate_item 
    : variable_declaration ';'  { $$.reset(new Block());  $$->add_list<Variable>($1);  }
    | function_declaration      { $$.reset(new Block());                               }
    | continuous_assign         { $$.reset(new Block());  $$->add_list<Assign>($1);    }
    | gate_instantiation        { $$.reset(new Block());  $$->add_list<Instance>($1);  }
    | module_instantiation      { $$.reset(new Block());  $$->add_list<Instance>($1);  }
    | always_construct          { $$.reset(new Block());  $$->add($1);                 }
    | "if" '(' expression ')'  generate_item_or_null 
    { $$.reset(new Block()); $$->add_if(@$, $3, $5); }
    | "if" '(' expression ')' generate_item_or_null "else" generate_item_or_null
    { $$.reset(new Block()); $$->add_if(@$, $3, $5, $7); }
    | "case" '(' expression ')' "default"  generate_item_or_null "endcase"
    { shared_ptr<CaseItem> m(new CaseItem(@6, $6)); $$.reset(new Block()); $$->add_case(@$, $3, m); }
    | "case" '(' expression ')' generate_case_items "endcase"
    { $$.reset(new Block()); $$->add_case(@$, $3, $5); }
    | "case" '(' expression ')' generate_case_items "default" generate_item_or_null "endcase"
    { shared_ptr<CaseItem> m(new CaseItem(@7, $7)); $$.reset(new Block()); $$->add_case(@$, $3, $5, m); }
    | "casex" '(' expression ')' "default"  generate_item_or_null "endcase"
    { shared_ptr<CaseItem> m(new CaseItem(@6, $6)); $$.reset(new Block()); $$->add_case(@$, $3, m, CaseState::CASE_X); }
    | "casex" '(' expression ')' generate_case_items "endcase"
    { $$.reset(new Block()); $$->add_case(@$, $3, $5, CaseState::CASE_X); }
    | "casex" '(' expression ')' generate_case_items "default" generate_item_or_null "endcase"
    { shared_ptr<CaseItem> m(new CaseItem(@7, $7)); $$.reset(new Block()); $$->add_case(@$, $3, $5, m, CaseState::CASE_X); }
    | "casez" '(' expression ')' "default"  generate_item_or_null "endcase"
    { shared_ptr<CaseItem> m(new CaseItem(@6, $6)); $$.reset(new Block()); $$->add_case(@$, $3, m, CaseState::CASE_Z); }
    | "casez" '(' expression ')' generate_case_items "endcase"
    { $$.reset(new Block()); $$->add_case(@$, $3, $5, CaseState::CASE_Z); }
    | "casez" '(' expression ')' generate_case_items "default" generate_item_or_null "endcase"
    { shared_ptr<CaseItem> m(new CaseItem(@7, $7)); $$.reset(new Block()); $$->add_case(@$, $3, $5, m, CaseState::CASE_Z); }
    | "for" '(' blocking_assignment ';' expression ';' blocking_assignment ')' "begin" ':' block_identifier generate_item_or_null "end"
    { $$.reset(new Block()); $12->set_name($11); $$->add_for(@$, $3, $5, $7, $12); }
    | "begin" generate_items "end" { $$.reset(new Block());  shared_ptr<GenBlock> m(new GenBlock(@$, *$2)); $$->add(m);}
    | "begin" ':' block_identifier generate_items "end" { $$.reset(new Block()); shared_ptr<GenBlock> m(new GenBlock(@$, *$4)); m->set_name($3); $$->add(m);}
    ;

//generate_conditional_statement 
//    : "if" '(' expression ')' generate_item_or_null 
//    | "if" '(' expression ')' generate_item_or_null "else" generate_item_or_null
//    ;

//generate_case_statement 
//    : "case" '(' expression ')' "default" generate_item_or_null "endcase"
//    | "case" '(' expression ')' generate_case_items "endcase"
//    | "case" '(' expression ')' generate_case_items "default" generate_item_or_null "endcase"
//    ;

generate_case_items
    : generate_case_item                          { $$.push_back($1); }
    | generate_case_items generate_case_item      { $$.push_back($2); }
    ;

generate_case_item 
    : expressions ':' generate_item_or_null     { $$.reset(new CaseItem(@$, $1, $3)); }
    | "default" ':' generate_item_or_null       { $$.reset(new CaseItem(@$, $3)); }
    ;

//generate_loop_statement 
//    : "for" '(' blocking_assignment ';' expression ';' blocking_assignment ')' "begin" ':' block_identifier generate_item_or_null "end"
//    ;

//generate_block 
//    : "begin" generate_items "end"
//    | "begin" ':' block_identifier  generate_items "end"
//    ;

//A.6.1 Continuous assignment statements
continuous_assign 
    : "assign" list_of_net_assignments ';'
    {
      BOOST_FOREACH(shared_ptr<Assign>& it, $2)
        it->set_continuous();
      $$ = $2;
    }
    ;

list_of_net_assignments 
    : blocking_assignment                             { $$.push_back($1); }
    | list_of_net_assignments ',' blocking_assignment { $$.push_back($3); }
    ;


//A.6.2 Procedural blocks and assignments
always_construct 
    : "always" '@' '(' '*' ')' statement
    { 
      list<pair<int, shared_ptr<Expression> > > slist;
      VIdentifier wild("*");
      slist.push_back(pair<int, shared_ptr<Expression> >(0, shared_ptr<Expression>(new Expression(wild))));
      $$.reset(new SeqBlock(@$, slist, $6));
    }
    | "always" '@' '(' event_expressions ')' statement_or_null
    { 
      $$.reset(new SeqBlock(@$, $4, $6)); 
    }
    | "always" '@' '*' statement 
    { 
      list<pair<int, shared_ptr<Expression> > > slist;
      VIdentifier wild("*");
      slist.push_back(pair<int, shared_ptr<Expression> >(0, shared_ptr<Expression>(new Expression(wild))));
      $$.reset(new SeqBlock(@$, slist, $4));
    }
    | "always" statement
    { 
      $$.reset(new SeqBlock(@$, *$2));
    }
    ;

blocking_assignment 
    : variable_lvalue '=' expression  { $3->reduce(); $$.reset(new Assign(@$, $1, $3, true));}
    ;

nonblocking_assignment 
    : variable_lvalue "<=" expression  { $3->reduce(); $$.reset(new Assign(@$, $1, $3, false));}
    ;


//A.6.4 Statements
statements
    : statement              { $$.reset(new Block()); $$->add_statements($1); }
    | statements statement   { $$->add_statements($2);                        }
    ;

statement
    : blocking_assignment ';'    { $$.reset(new Block()); $$->add($1); }
    | nonblocking_assignment ';' { $$.reset(new Block()); $$->add($1); }
    | "case" '(' expression ')' "default" statement_or_null "endcase" 
    { shared_ptr<CaseItem> m(new CaseItem(@$, $6)); $$.reset(new Block()); $$->add_case(@$, $3, m); }
    | "case" '(' expression ')' case_items "endcase" { $$.reset(new Block()); $$->add_case(@$, $3, $5); }
    | "case" '(' expression ')' case_items "default" statement_or_null "endcase" 
    { shared_ptr<CaseItem> m(new CaseItem(@$, $7)); $$.reset(new Block()); $$->add_case(@$, $3, $5, m); }
    | "casex" '(' expression ')' "default" statement_or_null "endcase" 
    { shared_ptr<CaseItem> m(new CaseItem(@$, $6)); $$.reset(new Block()); $$->add_case(@$, $3, m, CaseState::CASE_X); }
    | "casex" '(' expression ')' case_items "endcase" { $$.reset(new Block()); $$->add_case(@$, $3, $5, CaseState::CASE_X); }
    | "casex" '(' expression ')' case_items "default" statement_or_null "endcase" 
    { shared_ptr<CaseItem> m(new CaseItem(@$, $7)); $$.reset(new Block()); $$->add_case(@$, $3, $5, m, CaseState::CASE_X); }
    | "casez" '(' expression ')' "default" statement_or_null "endcase" 
    { shared_ptr<CaseItem> m(new CaseItem(@$, $6)); $$.reset(new Block()); $$->add_case(@$, $3, m, CaseState::CASE_Z); }
    | "casez" '(' expression ')' case_items "endcase" { $$.reset(new Block()); $$->add_case(@$, $3, $5, CaseState::CASE_Z); }
    | "casez" '(' expression ')' case_items "default" statement_or_null "endcase" 
    { shared_ptr<CaseItem> m(new CaseItem(@$, $7)); $$.reset(new Block()); $$->add_case(@$, $3, $5, m, CaseState::CASE_Z); }
    | "if" '(' expression ')' statement_or_null 
    { $$.reset(new Block()); $$->add_if(@$, $3, $5); }
    | "if" '(' expression ')' statement_or_null "else" statement_or_null  
    { $$.reset(new Block()); $$->add_if(@$, $3, $5, $7); }
    | "while" '(' expression ')' statement 
    { $$.reset(new Block()); $$->add_while(@$, $3, $5); }
    | "for" '(' blocking_assignment ';' expression ';' blocking_assignment ')' statement  
    { $$.reset(new Block()); $$->add_for(@$, $3, $5, $7, $9); }
    | "begin" statements "end" 
    { $$ = $2; }
    | "begin" list_of_variable_declarations statements "end" 
    { 
      $$.reset(new Block(@$));
      $$->add_list<Variable>($2);
      $$->add_statements($3);
      $$->elab_inparse();
    }
    | "begin" ':' block_identifier statements "end" 
    { 
      $$.reset(new Block(@$, $3)); 
      $$->add_statements($4); 
      $$->elab_inparse();
    }
    | "begin" ':' block_identifier list_of_variable_declarations statements "end" 
    { 
      $$.reset(new Block(@$, $3));
      $$->add_list<Variable>($4);
      $$->add_statements($5);
      $$->elab_inparse();
    }
    ;

statement_or_null 
    : /* empty */     { $$.reset(new Block()); }
    | statement       { $$ = $1; }
    ;
    
event_expressions
    : event_expression                           { $$.push_back($1); }
    | event_expressions "or" event_expression    { $$.push_back($3); }
    | event_expressions ',' event_expression     { $$.push_back($3); }
    ;

event_expression
    : expression              { $$ = pair<int, shared_ptr<Expression> >(0, $1); }
    | "posedge" expression    { $$ = pair<int, shared_ptr<Expression> >(1, $2); }
    | "negedge" expression    { $$ = pair<int, shared_ptr<Expression> >(-1, $2); }
    ;

case_items
    : case_item             { $$.push_back($1); }
    | case_items case_item  { $$.push_back($2); }
    ;

case_item 
    : expressions ':' statement_or_null    { $$.reset(new CaseItem(@$, $1, $3)); }
    | "default" ':' statement_or_null      { $$.reset(new CaseItem(@$, $3)); }
    ;

// A.8 Expressions
// A.8.1 Concatenations
expressions
    : expression                  { $$.push_back($1); }
    | expressions ',' expression  { $$.push_back($3); }
    ;

concatenation
    : '{' expressions '}'
    {
      $$.reset( new Concatenation(@$));
      BOOST_FOREACH(const shared_ptr<Expression>& it, $2) {
        shared_ptr<ConElem> m(new ConElem(it->loc, it));
        *$$ + m;
      }
    }
    | '{' expression concatenation '}'
    {
      $$.reset( new Concatenation(@$)); 
      shared_ptr<ConElem> m(new ConElem(@$, $2, $3->data));
      *$$ + m;
    }
    ;

// A.8.2 Function calls
function_call
    : function_identifier '(' expressions ')'
    ;

//A.8.3 Expressions
expression
    : primary                       {                                                          }
    | '+' primary %prec oUNARY      { $$ = $2; $$->append(Operation::oUPos); $$->loc = @$;     }
    | '-' primary %prec oUNARY      { $$ = $2; $$->append(Operation::oUNeg); $$->loc = @$;     }
    | '!' primary %prec oUNARY      { $$ = $2; $$->append(Operation::oULRev); $$->loc = @$;    }
    | '~' primary %prec oUNARY      { $$ = $2; $$->append(Operation::oURev); $$->loc = @$;     }
    | '&' primary %prec oUNARY      { $$ = $2; $$->append(Operation::oUAnd); $$->loc = @$;     }
    | "~&" primary %prec oUNARY     { $$ = $2; $$->append(Operation::oUNand); $$->loc = @$;    }
    | '|' primary %prec oUNARY      { $$ = $2; $$->append(Operation::oUOr); $$->loc = @$;      }
    | "~|" primary %prec oUNARY     { $$ = $2; $$->append(Operation::oUNor); $$->loc = @$;     }
    | '^' primary %prec oUNARY      { $$ = $2; $$->append(Operation::oXor); $$->loc = @$;      }
    | "~^" primary %prec oUNARY     { $$ = $2; $$->append(Operation::oNxor); $$->loc = @$;     }
    | expression '+' expression     { $$->append(Operation::oAdd, *$3); $$->loc = @$;          }
    | expression '-' expression     { $$->append(Operation::oMinus, *$3); $$->loc = @$;        }
    | expression '*' expression     { $$->append(Operation::oTime, *$3); $$->loc = @$;         }
    | expression '/' expression     { $$->append(Operation::oDiv, *$3); $$->loc = @$;          }
    | expression '%' expression     { $$->append(Operation::oMode, *$3); $$->loc = @$;         }
    | expression "==" expression    { $$->append(Operation::oEq, *$3); $$->loc = @$;           }
    | expression "!=" expression    { $$->append(Operation::oNeq, *$3); $$->loc = @$;          }
    | expression "===" expression   { $$->append(Operation::oCEq, *$3); $$->loc = @$;          }
    | expression "!==" expression   { $$->append(Operation::oCNeq, *$3); $$->loc = @$;         }
    | expression "&&" expression    { $$->append(Operation::oLAnd, *$3); $$->loc = @$;         }
    | expression "||" expression    { $$->append(Operation::oLOr, *$3); $$->loc = @$;          }
    | expression "**" expression    { $$->append(Operation::oPower, *$3); $$->loc = @$;        }
    | expression '<' expression     { $$->append(Operation::oLess, *$3); $$->loc = @$;         }
    | expression "<=" expression    { $$->append(Operation::oLe, *$3); $$->loc = @$;           }
    | expression '>' expression     { $$->append(Operation::oGreat, *$3); $$->loc = @$;        }
    | expression ">=" expression    { $$->append(Operation::oGe, *$3); $$->loc = @$;           }
    | expression '&' expression     { $$->append(Operation::oAnd, *$3); $$->loc = @$;          }
    | expression '|' expression     { $$->append(Operation::oOr, *$3); $$->loc = @$;           }
    | expression '^' expression     { $$->append(Operation::oXor, *$3); $$->loc = @$;          }
    | expression "~^" expression    { $$->append(Operation::oNxor, *$3); $$->loc = @$;         }
    | expression ">>" expression    { $$->append(Operation::oRS, *$3); $$->loc = @$;           }
    | expression "<<" expression    { $$->append(Operation::oLS, *$3); $$->loc = @$;           }
    | expression ">>>" expression   { $$->append(Operation::oLRS, *$3); $$->loc = @$;          }
    | expression '?' expression ':' expression { $$->append(Operation::oQuestion, *$3, *$5); $$->loc = @$; }
    ;

range_expression
    : expression                    { $$.reset( new Range(@$, $1)); }       
    | expression ':' expression     
    { $$.reset( new Range(@$, pair<shared_ptr<Expression>,shared_ptr<Expression> >($1,$3))); }
    | expression "+:" expression    
    { $$.reset( new Range(@$, pair<shared_ptr<Expression>,shared_ptr<Expression> >($1,$3), 1)); }
    | expression "-:" expression    
    { $$.reset( new Range(@$, pair<shared_ptr<Expression>,shared_ptr<Expression> >($1,$3), -1)); }
    ;

//A.8.4 Primaries
primary
    : number              { $$.reset( new Expression(@$, $1)); }            
    | variable_identifier 
    {
      $$.reset( new Expression(@$, $1));
    }
    | concatenation      { $$.reset( new Expression(@$, $1)); }
    | function_call
    | '(' expression ')'  { $$ = $2; $$->loc = @$; }
    ;

//A.8.5 Expression left-side values
variable_lvalue
    : variable_identifier 
    {
      $$.reset( new LConcatenation(@$, $1));
    }
    | concatenation       
    { 
      $$.reset( new LConcatenation(@$, $1));
      if(!$$->is_valid()) 
        av_env.error(@$, "SYN-ASSIGN-0");
    }
    ;

//A.9 General
//A.9.3 Identifiers
block_identifier 
    : identifier            { $$ = BIdentifier(@$, $1); }    
    ;

function_identifier 
    : identifier
    ;

module_identifier
    :  identifier          { $$ = MIdentifier(@$, $1); }
    ;

instance_identifier 
    : identifier           { $$ = IIdentifier(@$, $1); }
    ;

parameter_identifier 
    : identifier           { $$ = VIdentifier(@$, $1); }
    ;

variable_identifier
    : identifier           { $$ = VIdentifier(@$, $1); }
    | variable_identifier '[' range_expression ']' { $$ = $1; $$.get_select().add_low_dimension($3); }
    ;

port_identifier
    : identifier          { $$ = VIdentifier(@$, $1); }             
    ;

