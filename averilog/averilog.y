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
  
#define yylex avlex
  
  yyscan_t avscanner;
  
  void averilog::av_parser::error (const location_type& loc, const string& msg) {
    av_env.error(loc, "PARSER-0");
  }
  
  using namespace netlist;
    
#define Lib (*(av_env.curLib))

%}


%initial-action
{
  @$.initialize(&fname);
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
%type <tAssign>         blocking_assignment
%type <tAssign>         nonblocking_assignment
%type <tBlock>          statements
%type <tBlock>          statement
%type <tBlock>          statement_or_null 
%type <tBlockName>      block_identifier
%type <tCaseItem>       case_item
%type <tConcatenation>  concatenation
%type <tExp>            expression
%type <tExp>            primary
%type <tEvent>          event_expression
%type <tInstance>       module_instance
%type <tInstance>       n_input_gate_instance
%type <tInstance>       n_output_gate_instance
%type <tInstName>       instance_identifier
%type <tLConcatenation> variable_lvalue
%type <tListCaseItem>   case_items
%type <tListExp>        expressions
%type <tListEvent>      event_expressions
%type <tListInst>       module_instances
%type <tListInst>       n_input_gate_instances
%type <tListInst>       n_output_gate_instances
%type <tListPort>       list_of_port_identifiers
%type <tListParaAssign> list_of_parameter_assignments 
%type <tListParaAssign> named_parameter_assignments
%type <tListParaAssign> ordered_parameter_assignments
%type <tListPortConn>   input_terminals
%type <tListPortConn>   list_of_port_connections
%type <tListPortConn>   named_port_connections
%type <tListPortConn>   ordered_port_connections
%type <tListPortConn>   output_terminals
%type <tListVar>        list_of_variable_identifiers
%type <tModuleName>     module_identifier
%type <tModuleName>     n_input_gatetype
%type <tModuleName>     n_output_gatetype
%type <tParaAssign>     named_parameter_assignment
%type <tParaAssign>     ordered_parameter_assignment
%type <tPortName>       port_identifier
%type <tPortConn>       named_port_connection
%type <tPortConn>       ordered_port_connection
%type <tRange>          range_expression
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
    : "module" module_identifier ';'  { if(!Lib.insert($2)) av_env.error(yylloc, "SYN-MODULE-0", $2.name); }
        module_items
        "endmodule"                   { cout<< *(static_pointer_cast<Module>(Lib.get_current_comp())); Lib.pop(); }
    | "module" module_identifier '(' list_of_port_identifiers ')' ';'
    {
      if(!Lib.insert($2)) av_env.error(yylloc, "SYN-MODULE-0", $2.name);
      // get a pointer to the current module
      shared_ptr<Module> cm = static_pointer_cast<Module>(Lib.get_current_comp());
      // insert ports
      list<PoIdentifier>::iterator it, end;
      for(it = $4.begin(), end = $4.end(); it != end; it++) {
        PoIdentifier& pid = *it;
        if(cm->db_port.find(pid).use_count() == 0)
          cm->db_port.insert(pid, shared_ptr<Port>(new Port(pid)));
        else
          av_env.error(yylloc, "SYN-PORT-1", pid.name, cm->name.name);
      }
    }
        module_items
      "endmodule"                      { cout<< *(static_pointer_cast<Module>(Lib.get_current_comp())); Lib.pop(); }
    | "module" module_identifier '#' '(' parameter_declaration ')' '(' list_of_port_identifiers ')' ';'
        module_items
      "endmodule"                   
    ;

// A.1.4 Module paramters and ports

// A.1.5 Module items
module_item
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

module_items
    : module_item
    | module_items module_item
    
// A.2.1 Declaration types
// A.2.1.1 Module parameter declarations
parameter_declaration
    : "parameter" list_of_param_assignments
    ;

// A.2.1.2 Port declarations
input_declaration 
    : "input" list_of_port_identifiers
    {      
      // get a pointer to the current module
      shared_ptr<Module> cm = static_pointer_cast<Module>(Lib.get_current_comp());
      // insert ports
      list<PoIdentifier>::iterator it, end;
      for(it = $2.begin(), end = $2.end(); it != end; it++) {
        shared_ptr<Port> cp = cm->db_port.find(*it);
        if(0 != cp.use_count()) cp->set_in();
        else { av_env.error(yylloc, "SYN-PORT-0", it->name, cm->name.name); }
      }
    }
    | "input" '[' expression ':' expression ']' list_of_port_identifiers
    {      
      // get a pointer to the current module
      shared_ptr<Module> cm = static_pointer_cast<Module>(Lib.get_current_comp());
      // insert ports
      list<PoIdentifier>::iterator it, end;
      for(it = $7.begin(), end = $7.end(); it != end; it++) {
        shared_ptr<Port> cp = cm->db_port.find(*it);
        if(0 != cp.use_count()) { 
          cp->set_in();
          Range m(pair<shared_ptr<Expression>, shared_ptr<Expression> >($3, $5));
          cp->name.get_range_ref().push_back(m);
        } else {  av_env.error(yylloc, "SYN-PORT-0", it->name, cm->name.name); }
      }
    }  
    ;

output_declaration 
    : "output" list_of_port_identifiers
    {      
      // get a pointer to the current module
      shared_ptr<Module> cm = static_pointer_cast<Module>(Lib.get_current_comp());
      // insert ports
      list<PoIdentifier>::iterator it, end;
      for(it = $2.begin(), end = $2.end(); it != end; it++) {
        shared_ptr<Port> cp = cm->db_port.find(*it);
        if(0 != cp.use_count()) cp->set_out();
        else {  av_env.error(yylloc, "SYN-PORT-0", it->name, cm->name.name); }
      }
    }
    | "output" '[' expression ':' expression ']' list_of_port_identifiers
    {      
      // get a pointer to the current module
      shared_ptr<Module> cm = static_pointer_cast<Module>(Lib.get_current_comp());
      // insert ports
      list<PoIdentifier>::iterator it, end;
      for(it = $7.begin(), end = $7.end(); it != end; it++) {
        shared_ptr<Port> cp = cm->db_port.find(*it);
        if(0 != cp.use_count()) { 
          cp->set_out();
          vector<Range> rm;
          rm.push_back(Range(pair<shared_ptr<Expression>, shared_ptr<Expression> >($3, $5)));
          cp->name.set_range(rm);
        } else {  av_env.error(yylloc, "SYN-PORT-0", it->name, cm->name.name); }
      }
    }  
    ;

// A.2.1.3 Type declarations
variable_declaration 
    : "wire" list_of_variable_identifiers 
    {
      if(0 == $2.size()) {
        av_env.error(yylloc, "SYN-VAR-1", "wire");
      } else if(0 == Lib.get_current_comp().use_count()) {
        av_env.error(yylloc, "SYN-VAR-0", "Wire", $2.front().first.name);
      } else {
        shared_ptr<NetComp> father = Lib.get_current_comp();
        shared_ptr<Module> cm;
        switch(father->get_type()) {
        case NetComp::tModule: { 
          cm = static_pointer_cast<Module>(father);
          while(!$2.empty()) {
            shared_ptr<Variable> cw(new Variable($2.front().first));
            if($2.front().second.size() != 0) 
              av_env.error(yylloc, "SYN-VAR-4", cw->name.name);
            
            $2.pop_front();
            // change range selector to dimension delcaration
            cw->name.get_range_ref() = cw->name.get_select();
            cw->name.get_select_ref().clear();
            vector<shared_ptr<Range> >::iterator it, end;
            for(it = cw->name.get_range_ref().begin(), end = cw->name.get_range_ref().end(); it != end; it++ )
              (*it)->set_dim();

            // insert it in the database
            if(!cm->db_wire.insert(cw->name, cw)) {
              av_env.error(yylloc, "SYN-VAR-2", "wire", cw->name.name, cm->name.name);
            }
          }
        } break;
        default: ;/* doing nothing right now */
        }
      }
      ////////////////////////////////////////
    } 
    | "wire" '[' expression ':' expression ']' list_of_variable_identifiers
    {
      if(0 == $7.size()) {
        av_env.error(yylloc, "SYN-VAR-1", "wire");
      } else if(0 == Lib.get_current_comp().use_count()) {
        av_env.error(yylloc, "SYN-VAR-0", "Wire", $7.front().first.name);
      } else {
        shared_ptr<NetComp> father = Lib.get_current_comp();
        shared_ptr<Module> cm;
        switch(father->get_type()) {
        case NetComp::tModule: { 
          cm = static_pointer_cast<Module>(father);
          while(!$7.empty()) {
            VIdentifier& wn = $7.front().first;
            if($7.front().second.size() != 0) 
              av_env.error(yylloc, "SYN-VAR-4", wn.name);

            // change range selector to dimension delcaration
            wn.get_range_ref() = wn.get_select();
            wn.get_select_ref().clear();
            vector<shared_ptr<Range> >::iterator it, end;
            for(it = wn.get_range_ref().begin(), end = wn.get_range_ref().end(); it != end; it++ )
              (*it)->set_dim();

            Range rm(pair<shared_ptr<Expression>, shared_ptr<Expression> >($3, $5));
            wn.get_range_ref().push_back(rm);
            shared_ptr<Variable> cw(new Variable(wn));

            // insert it in the database
            $7.pop_front();
            if(!cm->db_wire.insert(cw->name, cw)) {
              av_env.error(yylloc, "SYN-VAR-2", "wire", cw->name.name, cm->name.name);
            }
          }
        } break;
	default: ;/* doing nothing right now */ 
	}
      }
      /////////////////////////////////////////////
    }
    | "reg" list_of_variable_identifiers 
    {
      if(0 == $2.size()) {
        av_env.error(yylloc, "SYN-VAR-1", "reg");
      } else if(0 == Lib.get_current_comp().use_count()) {
        av_env.error(yylloc, "SYN-VAR-0", "reg", $2.front().first.name);
      } else {
        shared_ptr<NetComp> father = Lib.get_current_comp();
        shared_ptr<Module> cm;
        switch(father->get_type()) {
        case NetComp::tModule: { 
          cm = static_pointer_cast<Module>(father);
          while(!$2.empty()) {
            shared_ptr<Variable> cr(new Variable($2.front().first));
            if($2.front().second.size() != 0) 
              av_env.error(yylloc, "SYN-VAR-4", cr->name.name);
            $2.pop_front();
            // change range selector to dimension delcaration
            cr->name.get_range_ref() = cr->name.get_select();
            cr->name.get_select_ref().clear();
            vector<shared_ptr<Range> >::iterator it, end;
            for(it = cr->name.get_range_ref().begin(), end = cr->name.get_range_ref().end(); it != end; it++ )
              (*it)->set_dim();
            
            // insert it in the database
            if(!cm->db_reg.insert(cr->name, cr)) {
              av_env.error(yylloc, "SYN-VAR-2", "reg", cr->name.name, cm->name.name);
            }
          }
        } break;
        default: ;/* doing nothing right now */
        }
      }
      ////////////////////////////////////////
    } 
    | "reg" '[' expression ':' expression ']' list_of_variable_identifiers
    {
      if(0 == $7.size()) {
        av_env.error(yylloc, "SYN-VAR-1", "reg");
      } else if(0 == Lib.get_current_comp().use_count()) {
        av_env.error(yylloc, "SYN-VAR-0", "reg", $7.front().first.name);
      } else {
        shared_ptr<NetComp> father = Lib.get_current_comp();
        shared_ptr<Module> cm;
        switch(father->get_type()) {
        case NetComp::tModule: { 
          cm = static_pointer_cast<Module>(father);
          while(!$7.empty()) {
            VIdentifier& rn = $7.front().first;
            if($7.front().second.size() != 0) 
              av_env.error(yylloc, "SYN-VAR-4", rn.name);

            // change range selector to dimension delcaration
            rn.get_range_ref() = rn.get_select();
            rn.get_select_ref().clear();
            vector<shared_ptr<Range> >::iterator it, end;
            for(it = rn.get_range_ref().begin(), end = rn.get_range_ref().end(); it != end; it++ )
              (*it)->set_dim();

            Range rm(pair<shared_ptr<Expression>, shared_ptr<Expression> >($3, $5));
            rn.get_range_ref().push_back(rm);
            shared_ptr<Variable> cr(new Variable(rn));

            // insert it in the database
            $7.pop_front();
            if(!cm->db_reg.insert(cr->name, cr)) {
              av_env.error(yylloc, "SYN-VAR-2", "reg", cr->name.name, cm->name.name);
            }
          }
        } break;
        default: ;/* doing nothing right now */ 
        }
      }
      /////////////////////////////////////////////
    }
    | "genvar" list_of_variable_identifiers
    {
      if(0 == $2.size()) {
        av_env.error(yylloc, "SYN-VAR-1", "genvar");
      } else if(0 == Lib.get_current_comp().use_count()) {
        av_env.error(yylloc, "SYN-VAR-0", "genvar", $2.front().first.name);
      } else {
        shared_ptr<NetComp> father = Lib.get_current_comp();
        shared_ptr<Module> cm;
        switch(father->get_type()) {
        case NetComp::tModule: { 
          cm = static_pointer_cast<Module>(father);
          while(!$2.empty()) {
            shared_ptr<Variable> cr(new Variable($2.front().first, $2.front().second));
            $2.pop_front();
            // change range selector to dimension delcaration
            cr->name.get_range_ref() = cr->name.get_select();
            cr->name.get_select_ref().clear();
            vector<shared_ptr<Range> >::iterator it, end;
            for(it = cr->name.get_range_ref().begin(), end = cr->name.get_range_ref().end(); it != end; it++ )
              (*it)->set_dim();
            
            // insert it in the database
            if(!cm->db_genvar.insert(cr->name, cr)) {
              av_env.error(yylloc, "SYN-VAR-2", "reg", cr->name.name, cm->name.name);
            }
          }
        } break;
        default: ;/* doing nothing right now */
        }
      }
      ////////////////////////////////////////
    } 
    | "integer" list_of_variable_identifiers
    {
      if(0 == $2.size()) {
        av_env.error(yylloc, "SYN-VAR-1", "integer");
      } else if(0 == Lib.get_current_comp().use_count()) {
        av_env.error(yylloc, "SYN-VAR-0", "integer", $2.front().first.name);
      } else {
        shared_ptr<NetComp> father = Lib.get_current_comp();
        shared_ptr<Module> cm;
        switch(father->get_type()) {
        case NetComp::tModule: { 
          cm = static_pointer_cast<Module>(father);
          while(!$2.empty()) {
            shared_ptr<Variable> cr(new Variable($2.front().first));
            if($2.front().second.size() != 0) 
              av_env.error(yylloc, "SYN-VAR-4", cr->name.name);
            $2.pop_front();
            // change range selector to dimension delcaration
            cr->name.get_range_ref() = cr->name.get_select();
            cr->name.get_select_ref().clear();
            vector<shared_ptr<Range> >::iterator it, end;
            for(it = cr->name.get_range_ref().begin(), end = cr->name.get_range_ref().end(); it != end; it++ )
              (*it)->set_dim();
            // an integer is a 32-bit reg
            cr->name.get_range_ref().push_back(Range(pair<Expression, Expression>(Expression(Number(31)), Expression(Number(0)))));
            
            // insert it in the database
            if(!cm->db_reg.insert(cr->name, cr)) {
              av_env.error(yylloc, "SYN-VAR-2", "integer", cr->name.name, cm->name.name);
            }
          }
        } break;
        default: ;/* doing nothing right now */
        }
      }
      ////////////////////////////////////////
    } 
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
    : port_identifier                              { $$.push_back($1); }
    | list_of_port_identifiers ',' port_identifier { $$.push_back($3); }
    ;

list_of_variable_identifiers 
    : variable_identifier                      { $$.clear(); $$.push_back(pair<VIdentifier,Expression>($1, Expression())); }
    | variable_identifier '=' expression       { $$.clear(); $$.push_back(pair<VIdentifier,Expression>($1, $3)); } 
    | list_of_variable_identifiers ',' variable_identifier { $$.push_back(pair<VIdentifier,Expression>($3, Expression())); }
    | list_of_variable_identifiers ',' variable_identifier '=' expression { $$.push_back(pair<VIdentifier,Expression>($3, $5)); }
    ;

// A.2.4 Declaration assignments
param_assignment 
    : parameter_identifier '=' expression 
    {
      shared_ptr<Variable> cp(new Variable($1,$3)); 
      shared_ptr<NetComp> father = Lib.get_current_comp();
      shared_ptr<Module> cm;
      switch(father->get_type()) {
      case NetComp::tModule: {
        cm = static_pointer_cast<Module>(father);
        if(!cm->db_param.insert(cp->name, cp)) {
          av_env.error(yylloc, "SYN-VAR-2", "Parameter", cp->name.name, cm->name.name);
        }
        break;
      }
      default:
        av_env.error(yylloc, "SYN-PARA-0", cp->name.name);
      }
    }
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
      list<shared_ptr<Instance> >::iterator it, end;
      shared_ptr<NetComp> father = Lib.get_current_comp();
      shared_ptr<Module> cm;
      
      for(it=$2.begin(), end=$2.end(); it!=end; it++) {
        (*it)->set_mname($1);
        shared_ptr<Instance>& ip = *it;
        switch(father->get_type()) {
        case NetComp::tModule: {
          cm = static_pointer_cast<Module>(father);
          if(!cm->db_instance.insert(ip->name, ip)) {
            av_env.error(yylloc, "SYN-INST-0", ip->name.name);
          }
          break;
        }
        default:;
        }
      }
    }
    | n_output_gatetype n_output_gate_instances ';'
    {
      list<shared_ptr<Instance> >::iterator it, end;
      shared_ptr<NetComp> father = Lib.get_current_comp();
      shared_ptr<Module> cm;
      
      for(it=$2.begin(), end=$2.end(); it!=end; it++) {
        (*it)->set_mname($1);
        shared_ptr<Instance>& ip = *it;
        switch(father->get_type()) {
        case NetComp::tModule: {
          cm = static_pointer_cast<Module>(father);
          if(!cm->db_instance.insert(ip->name, ip)) {
            av_env.error(yylloc, "SYN-INST-0", ip->name.name);
          }
          break;
        }
        default:;
        }
      }
    }
    ;

n_input_gate_instances
    : n_input_gate_instance                              { $$.clear(); $$.push_back($1); }
    | n_input_gate_instances ',' n_input_gate_instance   { $$.push_back($3); }
    ;

n_input_gate_instance
    : '(' variable_lvalue ',' input_terminals ')'
    {
      shared_ptr<NetComp> father = Lib.get_current_comp();
      shared_ptr<Module> cm;
      switch(father->get_type()) {
      case NetComp::tModule: { 
        cm = static_pointer_cast<Module>(father);
        // get a unique name
        while((cm->db_instance.find(cm->unnamed_instance)).use_count() != 0)
          ++(cm->unnamed_instance);
        // push the lvalue into port list
        $4.push_front(PortConn(Expression($2)));
        // assign a name for the instance
        $$.reset( new Instance(cm->unnamed_instance, $4,  Instance::prim_in_inst));
        ++(cm->unnamed_instance);
        break;
      }
      default: ;/* doing nothing right now */
      }
    }
    | instance_identifier '(' variable_lvalue ',' input_terminals ')'
    {
      $5.push_front(PortConn(Expression($3)));
      $$.reset( new Instance($1, $5, Instance::prim_in_inst));
    }
    | instance_identifier '[' expression ':' expression ']' '(' variable_lvalue ',' input_terminals ')'
    ;

input_terminals
    : expression                         { $$.push_back(PortConn($1)); }
    | input_terminals ',' expression     { $$.push_back(PortConn($3)); }
    ;

n_output_gate_instances
    : n_output_gate_instance                               { $$.push_back($1); }
    | n_output_gate_instances ',' n_output_gate_instance   { $$.push_back($3); }
    ;

n_output_gate_instance
    : '(' output_terminals ',' expression ')'
    {
      shared_ptr<NetComp> father = Lib.get_current_comp();
      shared_ptr<Module> cm;
      switch(father->get_type()) {
      case NetComp::tModule: { 
        cm = static_pointer_cast<Module>(father);
        // get a unique name
        while((cm->db_instance.find(cm->unnamed_instance)).use_count() != 0)
          ++(cm->unnamed_instance);
        // push the expression into port list
        $2.push_back(PortConn($4));
        // assign a name for the instance
        $$.reset( new Instance(cm->unnamed_instance, $2,  Instance::prim_out_inst));
        ++(cm->unnamed_instance);
        break;
      }
      default: ;/* doing nothing right now */
      }
    }
    | instance_identifier '(' output_terminals ',' expression ')'
    {
      $3.push_back(PortConn(Expression($5)));
      $$.reset( new Instance($1, $3, Instance::prim_out_inst));
    }
    | instance_identifier '[' expression ':' expression ']' '(' output_terminals ',' expression ')'
    ;

output_terminals
    : variable_lvalue                      { $$.push_back(PortConn(Expression($1))); }
    | output_terminals ',' variable_lvalue { $$.push_back(PortConn(Expression($3))); }
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
      shared_ptr<NetComp> father = Lib.get_current_comp();
      shared_ptr<Module> cm;
      switch(father->get_type()) {
      case NetComp::tModule: { 
        cm = static_pointer_cast<Module>(father);
        while(!$2.empty()) {
          shared_ptr<Instance> instp = $2.front();
          $2.pop_front();
          instp->set_mname($1);

          // insert it in the database
          if(!cm->db_instance.insert(instp->name, instp)) {
            av_env.error(yylloc, "SYN-INST-0", instp->name.name);
          }
        }
      } break;
      default: ;/* doing nothing right now */
      }
    }
    | module_identifier '#' '(' list_of_parameter_assignments ')' module_instances ';'
    {
      shared_ptr<NetComp> father = Lib.get_current_comp();
      shared_ptr<Module> cm;
      switch(father->get_type()) {
      case NetComp::tModule: { 
        cm = static_pointer_cast<Module>(father);
        while(!$6.empty()) {
          shared_ptr<Instance> instp = $6.front();
          $6.pop_front();
          instp->set_mname($1);
          instp->set_para($4);
          
          // insert it in the database
          if(!cm->db_instance.insert(instp->name, instp)) {
            av_env.error(yylloc, "SYN-INST-0", instp->name.name);
          }
        }
      } break;
      default: ;/* doing nothing right now */
      }
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
    : expression              { $$ = ParaConn($1); }
    ;

named_parameter_assignment 
    : '.' parameter_identifier '('  ')'           { $$ = ParaConn($2); }
    | '.' parameter_identifier '(' expression ')' { $$ = ParaConn($2, $4); }
    ;

module_instance 
    : instance_identifier '(' ')' { $$.reset(new Instance($1, list<PortConn>())); }
    | instance_identifier '[' expression ':' expression ']' '(' ')'
    | instance_identifier '(' list_of_port_connections ')'   { $$.reset(new Instance($1, $3)); }
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
    : /* empty */             { $$ = PortConn(); }
    | expression              { $$ = PortConn($1); }
    ;

named_port_connection 
    : '.' port_identifier '(' ')' { $$ = PortConn($2); }
    | '.' port_identifier '(' expression ')'  { $$ = PortConn($2, $4);}
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
    : "assign" list_of_net_assignments ';'  
    ;

list_of_net_assignments 
    : blocking_assignment                                
    {
      shared_ptr<NetComp> father = Lib.get_current_comp();
      shared_ptr<Module> cm;
      
      if(father->get_type() != NetComp::tModule) /* assign in non-module environment */
        av_env.error(yylloc, "SYN-ASSIGN-1");
      else {
        cm = static_pointer_cast<Module>(father);
        cm->db_assign.insert($1->name, $1);
      }
    }
    | list_of_net_assignments ',' blocking_assignment
    {
      shared_ptr<NetComp> father = Lib.get_current_comp();
      shared_ptr<Module> cm;
      
      if(father->get_type() != NetComp::tModule) /* assign in non-module environment */
        av_env.error(yylloc, "SYN-ASSIGN-1");
      else {
        cm = static_pointer_cast<Module>(father);
        cm->db_assign.insert($3->name, $3);
      }
    }
    ;


//A.6.2 Procedural blocks and assignments
always_construct 
    : "always" statement 
    {
      shared_ptr<NetComp> father = Lib.get_current_comp();
      shared_ptr<Module> cm;
      shared_ptr<GenBlock> cg;
      
      switch(father->get_type()) {
      case NetComp::tModule: {
        cm = static_pointer_cast<Module>(father);
        if($2.is_named()) {
          if(!cm->db_block.insert($2.name, shared_ptr<SeqBlock>(new SeqBlock($2))))
            assert(0 == "block named duplicated");
        } else {
          while(cm->db_block.find(cm->unnamed_block))
            ++(cm->unnamed_block);
          cm->db_block.insert(cm->unnamed_block, shared_ptr<SeqBlock>(new SeqBlock($2)));
        }
        break;
      }
      case NetComp::tGenBlock: {
        break;
      }
      default:
        assert(0 == "wrong block type");
      }
    }
    ;

blocking_assignment 
    : variable_lvalue '=' expression  { $3.reduce(); $$.reset(new Assign($1, $3, true)); $$->db_register();}
    ;

nonblocking_assignment 
    : variable_lvalue "<=" expression  { $3.reduce(); $$.reset(new Assign($1, $3, false)); $$->db_register();}
    ;

//A.6.3 Parallel and sequential blocks    
//seq_block
//    : "begin" statements "end"
//    | "begin" list_of_variable_declarations statements "end"
//    | "begin" ':' block_identifier statements "end"
//    | "begin" ':' block_identifier list_of_variable_declarations statements "end"
//    ;

//A.6.4 Statements
statements
    : statement
    | statements statement   { $$.add_statements($2); }
    ;

statement
    : blocking_assignment ';'    { $$.add_assignment(*$1); }
    | nonblocking_assignment ';' { $$.add_assignment(*$1); }
    | "case" '(' expression ')' "default" statement_or_null "endcase" 
    { CaseItem m($6); $$.add_case($3, m); }
    | "case" '(' expression ')' case_items "endcase" { $$.add_case($3, $5); }
    | "case" '(' expression ')' case_items "default" statement_or_null "endcase" 
    { CaseItem m($7); $$.add_case($3, $5, m); }
    | "if" '(' expression ')' statement_or_null 
      //{ $$.add_if($3, $5, SeqBlock()); }
    | "if" '(' expression ')' statement_or_null "else" statement_or_null  
      //{ $$.add_if($3, $5, $7); }
    | "while" '(' expression ')' statement 
      //{ $$.add_while($3, $5); }
    | "for" '(' blocking_assignment ';' expression ';' blocking_assignment ')' statement  
      //{ $$.add_for($3, $5, $7, $9); }
    | '@' '(' event_expressions ')' statement_or_null 
    { $$.add_seq_block($3, $5); /* this is not right */}
    | "begin" statements "end" 
    { $$.add_statements($2); }
    | "begin" 
    {
      Lib.push(shared_ptr<NetComp>(new SeqBlock()));
    } 
      list_of_variable_declarations statements "end" 
    { 
      shared_ptr<SeqBlock> bp = static_pointer_cast<SeqBlock>(Lib.get_current_comp());
      bp->add_statements($4);
      $$ = *bp;
      Lib.pop();
    }
    | "begin" ':' block_identifier statements "end" { $$.add_statements($4); $$.set_name($3); }
    | "begin" ':' block_identifier 
    {
      Lib.push(shared_ptr<NetComp>(new SeqBlock($3)));
    } 
      list_of_variable_declarations statements "end" 
    { 
      shared_ptr<SeqBlock> bp = static_pointer_cast<SeqBlock>(Lib.get_current_comp());
      bp->add_statements($6);
      $$ = *bp;
      Lib.pop();
    }
    ;

statement_or_null 
    : /* empty */
    | statement
    ;
    
//A.6.5 Timing control statements
//event_control
//    : '@' '(' event_expressions ')'
//    ;

event_expressions
    : event_expression                           { $$.push_back($1); }
    | event_expressions "or" event_expression    { $$.push_back($3); }
    | event_expressions ',' event_expression     { $$.push_back($3); }
    ;

event_expression
    : expression              { $$ = pair<int, Expression>(0, $1); }
    | "posedge" expression    { $$ = pair<int, Expression>(1, $2); }
    | "negedge" expression    { $$ = pair<int, Expression>(-1, $2); }
    ;

//procedural_timing_control_statement
//    : event_control statement_or_null
//    ;

//A.6.6 Conditional statements
//conditional_statement 
//    : "if" '(' expression ')' statement_or_null
//    | "if" '(' expression ')' statement_or_null "else" statement_or_null
//    ;

//A.6.7 Case statements
//case_statement 
//    : "case" '(' expression ')' "default" statement_or_null "endcase"
//    | "case" '(' expression ')' case_items "endcase"
//    | "case" '(' expression ')' case_items "default" statement_or_null "endcase"
//    ;

case_items
    : case_item             { $$.push_back($1); }
    | case_items case_item  { $$.push_back($2); }
    ;

case_item 
    : expressions ':' statement_or_null    { $$ = CaseItem($1, $3); }
    | "default" ':' statement_or_null      { $$ = CaseItem($3); }
    ;

//A.6.8 Looping statements
//loop_statement
//    : "while" '(' expression ')' statement
//    | "for" '(' blocking_assignment ';' expression ';' blocking_assignment ')' statement
//    ;

// A.8 Expressions
// A.8.1 Concatenations
expressions
    : expression                  { $$.push_back($1); }
    | expressions ',' expression  { $$.push_back($3); }
    ;

concatenation
    : '{' expressions '}'
    {
      list<Expression>::iterator it, end;
      $$ = Concatenation();
      for(it = $2.begin(), end = $2.end(); it != end; it++) {
        ConElem m(*it);
        $$ + m;
        //$$ + ConElem(*it);
      }
    }
    | '{' expression concatenation '}'
    {
      $$ = Concatenation(); 
      ConElem m($2, $3.data);
      $$ + m;
    }
    ;

// A.8.2 Function calls
function_call
    : function_identifier '(' expressions ')'
    ;

//A.8.3 Expressions
expression
    : primary                       {                                            }
    | '+' primary %prec oUNARY      { $$ = $2; $$.append(Operation::oUPos);      }
    | '-' primary %prec oUNARY      { $$ = $2; $$.append(Operation::oUNeg);      }
    | '!' primary %prec oUNARY      { $$ = $2; $$.append(Operation::oULRev);     }
    | '~' primary %prec oUNARY      { $$ = $2; $$.append(Operation::oURev);      }
    | '&' primary %prec oUNARY      { $$ = $2; $$.append(Operation::oUAnd);      }
    | "~&" primary %prec oUNARY     { $$ = $2; $$.append(Operation::oUNand);     }
    | '|' primary %prec oUNARY      { $$ = $2; $$.append(Operation::oUOr);       }
    | "~|" primary %prec oUNARY     { $$ = $2; $$.append(Operation::oUNor);      }
    | '^' primary %prec oUNARY      { $$ = $2; $$.append(Operation::oXor);       }
    | "~^" primary %prec oUNARY     { $$ = $2; $$.append(Operation::oNxor);      }
    | expression '+' expression     { $$.append(Operation::oAdd, $3);   }
    | expression '-' expression     { $$.append(Operation::oMinus, $3); }
    | expression '*' expression     { $$.append(Operation::oTime, $3);  }
    | expression '/' expression     { $$.append(Operation::oDiv, $3);   }
    | expression '%' expression     { $$.append(Operation::oMode, $3);  }
    | expression "==" expression    { $$.append(Operation::oEq, $3);    }
    | expression "!=" expression    { $$.append(Operation::oNeq, $3);   }
    | expression "===" expression   { $$.append(Operation::oCEq, $3);   }
    | expression "!==" expression   { $$.append(Operation::oCNeq, $3);  }
    | expression "&&" expression    { $$.append(Operation::oLAnd, $3);  }
    | expression "||" expression    { $$.append(Operation::oLOr, $3);   }
    | expression "**" expression    { $$.append(Operation::oPower, $3); }
    | expression '<' expression     { $$.append(Operation::oLess, $3);  }
    | expression "<=" expression    { $$.append(Operation::oLe, $3);    }
    | expression '>' expression     { $$.append(Operation::oGreat, $3); }
    | expression ">=" expression    { $$.append(Operation::oGe, $3);    }
    | expression '&' expression     { $$.append(Operation::oAnd, $3);   }
    | expression '|' expression     { $$.append(Operation::oOr, $3);    }
    | expression '^' expression     { $$.append(Operation::oXor, $3);   }
    | expression "~^" expression    { $$.append(Operation::oNxor, $3);  }
    | expression ">>" expression    { $$.append(Operation::oRS, $3);    }
    | expression "<<" expression    { $$.append(Operation::oLS, $3);    }
    | expression ">>>" expression   { $$.append(Operation::oLRS, $3);   }
    | expression '?' expression ':' expression { $$.append(Operation::oQuestion, $3, $5); }
    ;

range_expression
    : expression                    { $$ = Range($1); }       
    | expression ':' expression     { $$ = Range(pair<Expression,Expression>($1,$3)); }
    | expression "+:" expression    { $$ = Range(pair<Expression,Expression>($1,$3), 1); }
    | expression "-:" expression    { $$ = Range(pair<Expression,Expression>($1,$3), -1); }
    ;

//A.8.4 Primaries
primary
    : number              { $$ = $1; }            
    | variable_identifier 
    {
      // search this variable in current components until reach a module level
      list<shared_ptr<NetComp> >::iterator it = Lib.get_current_it();
      bool reach_a_module = false;
      bool found = false;
      while(Lib.it_valid(it)) {
        shared_ptr<NetComp> ccp(*it); /* point for the current component */
        switch(ccp->get_type()) {
        case NetComp::tModule: {
          reach_a_module = true;
          Module& cm = *(static_pointer_cast<Module>(ccp));
          shared_ptr<Variable> vp = cm.db_wire.find($1);
          if(0 != vp.use_count()) {
            found = true;
            $1.set_father(vp);
            break;
          }
          vp = cm.db_reg.find($1);
          if(0 != vp.use_count()) {
            found = true;
            $1.set_father(vp);
            break;
          }
          vp = cm.db_param.find($1);
          if(0 != vp.use_count()) {
            found = true;
            $1.set_father(vp);
            break;
          }
          break;
        }
        default:
          // should not run to here
          assert(0 == "component type");
        }
        if(reach_a_module || found) break;
        else it++;
      }
      if(found)
        $$ = $1;
      else
        av_env.error(yylloc, "SYN-VAR-3", $1.name);
    }
    | concatenation      { $$ = $1; }
    | function_call
    | '(' expression ')'  { $$ = $2; }
    ;

//A.8.5 Expression left-side values
variable_lvalue
    : variable_identifier 
    {
      // search this variable in current components until reach a module level
      list<shared_ptr<NetComp> >::iterator it = Lib.get_current_it();
      bool reach_a_module = false;
      bool found = false;
      while(Lib.it_valid(it)) {
        shared_ptr<NetComp> ccp(*it); /* point for the current component */
        switch(ccp->get_type()) {
        case NetComp::tModule: {
          reach_a_module = true;
          Module& cm = *(static_pointer_cast<Module>(ccp));
          shared_ptr<Variable> vp = cm.db_wire.find($1);
          if(0 != vp.use_count()) {
            found = true;
            $1.set_father(vp);
            break;
          }
          vp = cm.db_reg.find($1);
          if(0 != vp.use_count()) {
            found = true;
            $1.set_father(vp);
            break;
          }
          vp = cm.db_param.find($1);
          if(0 != vp.use_count()) {
            found = true;
            $1.set_father(vp);
            break;
          }
          break;
        }
        default:
          // should not run to here
          assert(0 == "component type");
        }
        if(reach_a_module || found) break;
        else it++;
      }
      if(found)
        $$ = $1;
      else
        av_env.error(yylloc, "SYN-VAR-3", $1.name);
    }
    | concatenation       
    { 
      $$ = $1;
      if(!$$.is_valid()) 
        av_env.error(yylloc, "SYN-ASSIGN-0");
    }
    ;

//A.9 General
//A.9.3 Identifiers
block_identifier 
    : identifier            { $$ = $1; }    
    ;

function_identifier 
    : identifier
    ;

module_identifier
    :  identifier          { $$ = $1; }
    ;

instance_identifier 
    : identifier           { $$ = $1; }
    ;

parameter_identifier 
    : identifier           { $$ = $1; }
    ;

variable_identifier
    : identifier           { $$ = $1; }
    | variable_identifier '[' range_expression ']' { $$.get_select_ref().push_back($3); }
    ;

port_identifier
    : identifier       { $$ = $1; }             
    ;

