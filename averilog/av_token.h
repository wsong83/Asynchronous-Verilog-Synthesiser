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
 * Token definitions
 * 17/02/2012   Wei Song
 *
 *
 */

#ifndef _H_AV_TOKEN_
#define _H_AV_TOKEN_

// copy from averilog.lex.h
#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

namespace averilog {

#define YYSTYPE av_token_type
  
  struct av_token_type {
    shared_ptr<netlist::Assign> tAssign;
    netlist::BIdentifier        tBlockName;
    netlist::Concatenation      tConcatenation;
    netlist::Expression         tExp;		
    netlist::FIdentifier        tFuncName;
    avID                        tID;      
    shared_ptr<netlist::Instance> tInstance;
    netlist::IIdentifier        tInstName;
    netlist::LConcatenation     tLConcatenation;
    list<netlist::Expression>   tListExp;
    list<shared_ptr<netlist::Instance> > tListInst;
    list<netlist::PoIdentifier> tListPort;
    list<netlist::ParaConn>     tListParaAssign;
    list<netlist::PortConn>     tListPortConn;
    list<pair<netlist::VIdentifier, netlist::Expression> > tListVar;
    netlist::MIdentifier        tModuleName;
    netlist::Number             tNumber;    
    netlist::PoIdentifier       tPortName;
    netlist::ParaConn           tParaAssign;
    netlist::PortConn           tPortConn;
    netlist::Range              tRange;	
    pair<netlist::VIdentifier, netlist::Expression> tVarAssign;
    netlist::VIdentifier        tVarName;	
  };
}


#endif
