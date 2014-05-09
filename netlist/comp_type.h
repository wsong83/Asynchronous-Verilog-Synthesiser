/*
 * Copyright (c) 2012-2014 Wei Song <songw@cs.man.ac.uk> 
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
 * Definition of the types of netlist components.
 * 09/02/2012   Wei Song
 *
 *
 */

enum ctype_t {
  tUnknown,                 /* type not initiated, indicate error!! */
  tAssign,                  /* assignment */
  tBlock,                   /* general block */
  tBlockName,               /* block name */
  tCaseItem,                /* a case item */
  tCase,                    /* a case statement */
  tConcatenation,           /* concatenation */
  tConElem,                 /* concatenation element */
  tExp,                     /* expression */
  tFor,                     /* for statement */
  tFuncName,                /* function name */
  tFunction,                /* function */
  tFuncCall,                /* function call */
  tGenBlock,                /* heneration block */
  tIf,                      /* if statement */
  tInstance,                /* module instance */
  tInstName,                /* instance name */
  tLConcatenation,          /* left-side concatenation */
  tModule,                  /* Module declaration */
  tModuleName,              /* module name */
  tNumber,                  /* number */
  tOperation,               /* operation */
  tParaConn,                /* parameter connection */
  tParaName,                /* parameter name */
  tPort,                    /* port */
  tPortConn,                /* port connection */
  tPortName,                /* port name */
  tRegister,                /* reg */
  tRange,                   /* range */
  tRangeArray,              /* array of range expressions */
  tSeqBlock,                /* sequential block in always */
  tVariable,                /* variable, including wire, reg, and parameter */
  tVarName,                 /* variable name */
  tWhile,                   /* while statement */
  tWire                     /* wire */
};

std::string get_type_name() const {
  std::string rv;
  switch(ctype) {
  case tAssign:          rv = "Assign Statement";        break;
  case tBlock:           rv = "Block";                   break;
  case tBlockName:       rv = "Block Name";              break;
  case tCaseItem:        rv = "Case Item";               break;
  case tCase:            rv = "Case Statement";          break;
  case tConcatenation:   rv = "Concatenation";           break;
  case tConElem:         rv = "Concatenation Element";   break;
  case tExp:             rv = "Expression";              break;
  case tFor:             rv = "For Statement";           break;
  case tFuncName:        rv = "Function Name";           break;
  case tFunction:        rv = "Function";                break;
  case tFuncCall:        rv = "Function Call";           break;
  case tGenBlock:        rv = "Generate Block";          break;
  case tIf:              rv = "If Statement";            break;
  case tInstance:        rv = "Instance";                break;
  case tInstName:        rv = "Instance Name";           break;
  case tLConcatenation:  rv = "Left-side Concatenation"; break;
  case tModule:          rv = "Module";                  break;
  case tModuleName:      rv = "Module Name";             break;
  case tNumber:          rv = "Number";                  break;
  case tOperation:       rv = "Operation";               break;
  case tParaConn:        rv = "Parameter Connection";    break;
  case tParaName:        rv = "Parameter Name";          break;
  case tPort:            rv = "Port Declaration";        break;
  case tPortConn:        rv = "Port Connection";         break;
  case tPortName:        rv = "Port Name";               break;
  case tRange:           rv = "Range Expression";        break;
  case tRangeArray:      rv = "Array of Range";          break;
  case tSeqBlock:        rv = "Sequential Block";        break;
  case tVariable:        rv = "Variable Declaration";    break;
  case tVarName:         rv = "Variable Name";           break;
  case tWhile:           rv = "While Statement";         break;
  default:               rv = "Unknown Type";
  }
  return rv;
}
