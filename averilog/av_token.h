/*
 * Copyright (c) 2012-2013 Wei Song <songw@cs.man.ac.uk> 
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

#ifndef AV_H_AV_TOKEN_
#define AV_H_AV_TOKEN_

#include <boost/shared_ptr.hpp>
#include <utility>
#include <list>
#include "netlist/component.h"

// copy from averilog.lex.h
#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

namespace averilog {

  struct av_token_type {
    boost::shared_ptr<netlist::Assign>                                  tAssign;
    boost::shared_ptr<netlist::Block>                                   tBlock;
    netlist::BIdentifier                                                tBlockName;
    boost::shared_ptr<netlist::CaseItem>                                tCaseItem;
    boost::shared_ptr<netlist::Concatenation>                           tConcatenation;
    boost::shared_ptr<netlist::Expression>                              tExp;
    std::pair<int, boost::shared_ptr<netlist::Expression> >             tEvent;
    boost::shared_ptr<netlist::FuncCall>                                tFuncCall;
    netlist::FIdentifier                                                tFuncName;
    boost::shared_ptr<netlist::Function>                                tFunction;
    boost::shared_ptr<netlist::GenBlock>                                tGenBlock;
    averilog::avID                                                      tID;      
    boost::shared_ptr<netlist::Instance>                                tInstance;
    netlist::IIdentifier                                                tInstName;
    boost::shared_ptr<netlist::LConcatenation>                          tLConcatenation;
    std::list<boost::shared_ptr<netlist::Assign> >                      tListAssign;
    std::list<boost::shared_ptr<netlist::CaseItem> >                    tListCaseItem;
    std::list<boost::shared_ptr<netlist::Expression> >                  tListExp;
    std::list<std::pair<int, boost::shared_ptr<netlist::Expression> > > tListEvent;
    std::list<boost::shared_ptr<netlist::Instance> >                    tListInst;
    //std::list<netlist::VIdentifier>                                     tListPort;
    std::list<boost::shared_ptr<netlist::ParaConn> >                    tListParaAssign;
    std::list<boost::shared_ptr<netlist::Port> >                        tListPortDecl;
    std::list<boost::shared_ptr<netlist::PortConn> >                    tListPortConn;
    std::list<std::pair<netlist::VIdentifier, boost::shared_ptr<netlist::Expression> > > tListVar;
    std::list<boost::shared_ptr<netlist::Variable> >                    tListVarDecl;
    netlist::MIdentifier                                                tModuleName;
    boost::shared_ptr<netlist::Module>                                  tModule;
    netlist::Number                                                     tNumber;    
    netlist::VIdentifier                                                tPortName;
    boost::shared_ptr<netlist::ParaConn>                                tParaAssign;
    boost::shared_ptr<netlist::PortConn>                                tPortConn;
    boost::shared_ptr<netlist::Range>                                   tRange;
    boost::shared_ptr<netlist::SeqBlock>                                tSeqBlock;
    std::pair<netlist::VIdentifier, boost::shared_ptr<netlist::Expression> > tVarAssign;
    netlist::VIdentifier                                                tVarName;	
  };
}


#endif
// Local Variables:
// mode: c++
// End:
