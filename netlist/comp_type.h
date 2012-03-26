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
 * Definition of the types of netlist components.
 * 09/02/2012   Wei Song
 *
 *
 */

enum ctype_t {
  tAssign,                  /* assignment */
  tBlockName,               /* block name */
  tCaseItem,                /* a case item */
  tCase,                    /* a case statement */
  tConcatenation,           /* concatenation */
  tExp,                     /* expression */
  tFuncName,                /* function name */
  tGenBlock,                /* heneration block */
  tInstance,                /* module instance */
  tInstName,                /* instance name */
  tLConcatenation,          /* left-side concatenation */
  tModule,                  /* Module declaration */
  tModuleName,              /* module name */
  tNumber,                  /* number */
  tParaName,                /* parameter name */
  tPort,                    /* port */
  tPortName,                /* port name */
  tRegister,                /* reg */
  tRange,                   /* range */
  tSeqBlock,                /* sequential block in always */
  tVariable,                /* variable, including wire, reg, and parameter */
  tVarName,                 /* variable name */
  tWire,                    /* wire */
  tUnkown                   /* type not initiated, indicate error!! */
};

