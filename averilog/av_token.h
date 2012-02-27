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
    shared_ptr<netlist::Assign>        tAssign;
    shared_ptr<netlist::BIdentifier>   tBlockName;
    shared_ptr<netlist::Concatenation> tConcatenation;
    shared_ptr<netlist::Expression>    tExp;		
    shared_ptr<netlist::FIdentifier>   tFuncName;
    shared_ptr<avID>                   tID;      
    shared_ptr<netlist::IIdentifier>   tInstName;
    shared_ptr<netlist::LConcatenation> tLConcatenation;
    shared_ptr<list<shared_ptr<netlist::Expression> > >   tListExp;
    shared_ptr<list<shared_ptr<netlist::PoIdentifier> > > tListPort;
    shared_ptr<list<shared_ptr<netlist::VIdentifier> > >  tListVar;
    shared_ptr<netlist::MIdentifier>   tModuleName;
    shared_ptr<netlist::Number>        tNumber;    
    shared_ptr<netlist::PaIdentifier>  tParaName;
    shared_ptr<netlist::PoIdentifier>  tPortName;
    shared_ptr<netlist::Range>         tRange;	
    shared_ptr<netlist::VIdentifier>   tVarName;	
  };
}


#endif