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
 
%}

%option never-interactive
%option c++

ws         [ \t\b\f\r]+
dig_d      [0-9]
dig_b      [xzXZ01_
dig_o      [xzXZ0-7]
dig_h      [a-fxzA-FXZ0-9]
dec        ({dig_d}+_?)+
fnum_b     {dec}?\'b({dig_b}+_?)+
fnum_d     {dec}?\'d({dig_d}+_?)+
fnum_o     {dec}?\'o({dig_o}+_?)+
fnum_h     {dec}?\'h({dig_h}+_?)+
number     dec|fnum_b|fnum_d|fnum_o|fnum_h

%%

"**"       { return oPower; }
">="       { return oGe;    }
"<="       { return oLe;    }
"&&"       { return oLAnd;  }
"||"       { return oLOr;   }
"=="       { return oEq;    }
"==="      { return oCEq;   }
"!="       { return oNeq;   }
"!=="      { return oCNeq;  }
"<<"       { return oLLsh;  }
">>"       { return oLRsh;  }
"<<<"      { return oLLsh; /* equivalent to << */ }
">>>"      { return oARsh;  }
"-:"       { return oNColon; }
"+:"       { return oPColon; }
"~&"       { return oRNand; }
"~|"       { return oRNor;  }
"~^"       { return oRXnor; }
"^~"       { return oRXnor; /* equivalent to ~^  */ }

"always"   { return kAlways; }
"assign"   { return KAssign; }
"automatic"  { return kAutomatic; }    
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
