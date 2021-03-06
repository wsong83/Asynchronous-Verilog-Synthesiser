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
 * Error definitions
 * 16/04/2012   Wei Song
 *
 *
 */

/** ERR DEFINITION:
    ERR_DEFINE(                      // MACRO defined in err_report.cpp
      const string,                  // Error ID for storage and searching
      severe_t,                      // EFatal, EError, EWarning or EInfo, defined in err_report.h
      unsigned int,                  // Number of parameters needed in the error message
      const string                   // The error message with parameter placements embedded
    )
*/

/// patterns that cannot recognised by the averilog parser, need to improve the parser grammer file in this case
ERR_DEFINE("PARSER-0",     
           EFatal,   0, 
           "unkown parser error. Please report to the developers.");

ERR_DEFINE("PARSER-1",     
           EInfo,   1, 
           "reading in \"%1%\".");

ERR_DEFINE("ANA-SSA-0",
           EInfo,   3,
           "register \"%1%\" has two sequential assignements, which may confuse SSA analysis. \n(1) %2% \n(2) %3%");

ERR_DEFINE("ANA-SSA-1",
           EInfo,   2,
           "latch \"%1%\" is used in the calculation for the combined expression assign \"%2%\", which may be a design error.");

ERR_DEFINE("ANA-SSA-2",
           EInfo,   2,
           "loop break at \"%1%\" in the calculation of combined expression.");

ERR_DEFINE("ELAB-0", 
           EInfo,   2, 
           "elaborating module \"%1%\"%2%.");

ERR_DEFINE("ELAB-ASSIGN-0", 
           EError,   0, 
           "illegal place for a continuous assignment.");

ERR_DEFINE("ELAB-ASSIGN-1", 
           EError,   0, 
           "illegal place for a non-continuous assignment.");

ERR_DEFINE("ELAB-BLOCK-0", 
           EError,   0, 
           "illegal place for a sequential block.");

ERR_DEFINE("ELAB-BLOCK-1", 
           EError,   0, 
           "illegal place for a block.");

ERR_DEFINE("ELAB-CASE-0", 
           EError,   0, 
           "illegal place for a case statement.");

// case items after a default case
ERR_DEFINE("ELAB-CASE-1", 
           EWarning,   0, 
           "the cases after a default case will be removed.");

// duplicated case statements
ERR_DEFINE("ELAB-CASE-2", 
           EWarning,   0, 
           "duplicated case item will be removed.");

// the case expression is not constant
ERR_DEFINE("ELAB-CASE-3", 
           EError,   1, 
           "the case expression %1% is not constant.");

// non-constant concationation parameter
ERR_DEFINE("ELAB-EXPRESSION-0",
           EError,   1,
           "\"%1%\" cannot be reduced to a constant. In concatenation {Var{exp0, exp1, ..}}, Var must be a constant: a const exp, a parameter or a generate variable.");

ERR_DEFINE("ELAB-EXPRESSION-2",
           EError,   1,
           "\"%1%\" is not supported yet. Variable power operation is not synthesisable.");

ERR_DEFINE("ELAB-EXPRESSION-3",
           EError,   1,
           "\"%1%\" is not supported yet. Variable division/modulate operation is not synthesisable.");

ERR_DEFINE("ELAB-FOR-0",
           EError,   0,
           "for statement must have a initial assignment.");

ERR_DEFINE("ELAB-FOR-1",
           EError,   1,
           "cannot evaluate the initial assignment \"%1%\" of the for statement");

ERR_DEFINE("ELAB-FOR-2",
           EError,   1,
           "cannot handle multiple variable initial assignment \"%1%\" of the for statement");

ERR_DEFINE("ELAB-FOR-3",
           EError,   1,
           "cannot evaluate the condition \"%1%\" of the for statement");

ERR_DEFINE("ELAB-FOR-4",
           EError,   1,
           "cannot evaluate the increment assignment \"%1%\" of the for statement");

ERR_DEFINE("ELAB-FOR-5",
           EError,   0,
           "define functions in a for statement is not supported yet.");

ERR_DEFINE("ELAB-FOR-6",
           EError,   1,
           "the variable/instance/function name \"%1%\" conflicts in unfolding a for statement.");

ERR_DEFINE("ELAB-IF-0", 
           EError,   0, 
           "illegal place for an if statement.");

ERR_DEFINE("ELAB-INST-0", 
           EError,   1, 
           "fail to find the module named \"%1%\".");

ERR_DEFINE("ELAB-INST-1", 
           EError,   2, 
           "fail to find port \"%1%\" in module \"%2%\".");

ERR_DEFINE("ELAB-INST-2", 
           EError,   1, 
           "fail to resolved the parameter assignment of module \"%1%\".");

ERR_DEFINE("ELAB-INST-3", 
           EError,   0, 
           "the number of unnamed parameters does not match.");

ERR_DEFINE("ELAB-NUM-0",    
           EError,   1, 
           "cannot get a decimal value from number \"%1%\".");

// a parameter of a module cannot be resolved to a number when trying to elaborate it
ERR_DEFINE("ELAB-PARA-0", 
           EError,   2, 
           "fail to calculate the value of parameter \"%1%\" in module \"%2%\".");

ERR_DEFINE("ELAB-PARA-1", 
           EError,   2, 
           "fail to find parameter \"%1%\" in module \"%2%\".");

// non-constant port range
ERR_DEFINE("ELAB-RANGE-0",
           EError,   1,
           "variable \"%1%\" has one or more non-constant range expressions.");

ERR_DEFINE("ELAB-RANGE-1",
           EError,   1,
           "range \"%1%\" is not a valid range selection.");

// wire/reg has no driver
ERR_DEFINE("ELAB-VAR-0",
           EError,   1,
           "variable \"%1%\" has no driver.");

// wire/reg has multiple driver
ERR_DEFINE("ELAB-VAR-1",
           EError,   3,
           "variable \"%1%\" has multiple drivers, where two of the drivers are located at %2% and %3%.");

// wire/reg has multiple driver
ERR_DEFINE("ELAB-VAR-2",
           EWarning,   1,
           "variable \"%1%\" has no load.");

// wire/reg has no driver
ERR_DEFINE("ELAB-VAR-3",
           EWarning,   1,
           "variable \"%1%\" is defined but not used.");

// wire/reg has no driver
ERR_DEFINE("ELAB-VAR-4",
           EError,   2,
           "variable \"%1%\" is out of the range definition of \"%2%\".");

// wire/reg has no driver
ERR_DEFINE("ELAB-VAR-5",
           EError,   1,
           "variable \"%1%\" has an illegal range definition.");

ERR_DEFINE("SDFG-ANALYSE-0", 
           EInfo,   1, 
           "a loop is detected: %1%");

ERR_DEFINE("SDFG-DATAPATH-0", 
           EInfo,   1, 
           "removing control nodes for module \"%1%\".");

ERR_DEFINE("SDFG-DATAPATH-1", 
           EInfo,   1, 
           "removing disconnected nodes for module \"%1%\".");

ERR_DEFINE("SDFG-DEDUCTION-0", 
           EInfo,   1, 
           "node \"%1%\" is considered to be a driver of clock signal.");

ERR_DEFINE("SDFG-DEDUCTION-1", 
           EInfo,   1, 
           "node \"%1%\" is considered to be a driver of reset signal.");

ERR_DEFINE("SDFG-DEDUCTION-2", 
           EInfo,   1, 
           "node \"%1%\" is considered to be a driver of control signal.");

ERR_DEFINE("SDFG-EXTRACT-0", 
           EInfo,   0, 
           "netlist is automatically uniquified.");

ERR_DEFINE("SDFG-EXTRACT-1", 
           EInfo,   1, 
           "extracting SDFG for module \"%1%\".");

ERR_DEFINE("SDFG-FSM-0", 
           EInfo,   1, 
           "extracting controllers for module \"%1%\".");

ERR_DEFINE("SDFG-SIMPLIFY-0", 
           EInfo,   1, 
           "node \"%1%\" is removed as it has no output edges.");

ERR_DEFINE("SDFG-SIMPLIFY-1", 
           EInfo,   2, 
           "node \"%1%\" is removed and its single input node \"%2%\" is connected to all output nodes.");

ERR_DEFINE("SDFG-SIMPLIFY-2", 
           EInfo,   2, 
           "node \"%1%\" is removed and its single output node \"%2%\" is connected to all input nodes.");

ERR_DEFINE("SDFG-SIMPLIFY-3", 
           EInfo,   3, 
           "move the through wire from \"%1%\" to \"%2%\" to the upper module \"%3%\".");

ERR_DEFINE("SDFG-SIMPLIFY-4", 
           EInfo,   2, 
           "move the constant output port \"%1%\" to the upper module \"%2%\".");

ERR_DEFINE("SDFG-SIMPLIFY-5", 
           EInfo,   1, 
           "module \"%1%\" is flattened as it has no registers and node count is small.");

/// on the left side of an assign should be a non-const variable or concatenation
ERR_DEFINE("SYN-ASSIGN-0", 
           EError,   0, 
           "wrong left-side value.");

ERR_DEFINE("SYN-ASSIGN-1", 
           EError,   0, 
           "keyword \"assign\" can only be used in module/generate enviornment.");

ERR_DEFINE("SYN-BLOCK-0", 
           EError,   2, 
           "block \"%1%\" is renamed as another block is named the same at %2%.");

ERR_DEFINE("SYN-BLOCK-1", 
           EError,   1, 
           "sequential block \"%1%\" has a mixed sensitive list with both edge and level triggering signals.");

ERR_DEFINE("SYN-BLOCK-2", 
           EWarning,   1, 
           "sequential block \"%1%\" has no/auto-inferred sensitive list.");

ERR_DEFINE("SYN-FILE-0",   
           EError,   0, 
           "file terminated in multi-line comments.");

ERR_DEFINE("SYN-FUNC-0",   
           EError,   2, 
           "function \"%1%\" is already defined at %2%.");

// duplicated instance name. The name of the duplicated instance will be numbered
ERR_DEFINE("SYN-INST-0",   
           EError,   2, 
           "instance \"%1%\" is renamed as another instance is named the same at %2%.");

ERR_DEFINE("SYN-INST-1",   
           EWarning,   0, 
           "primary gate is unnamed and will be named by a default name.");

ERR_DEFINE("SYN-INST-2",   
           EWarning,   1, 
           "instance \"%1\" defined in an unnamed embedded block has a same name in its parent block is not supported yet.");

// duplicated module definition. The duplicated module will be ignored.
ERR_DEFINE("SYN-MODULE-0", 
           EError,   1, 
           "module \"%1%\" is already defined.");

ERR_DEFINE("SYN-MODULE-1", 
           EError,   0, 
           "wrong type of statements in the module enviornment.");

ERR_DEFINE("SYN-MODULE-2", 
           EError,   1, 
           "module \"%1%\" is a black-box.");

ERR_DEFINE("SYN-MODULE-3", 
           EInfo,   2, 
           "module \"%1%\" is added to library \"%2%\".");

ERR_DEFINE("SYN-NUM-0",    
           EError,   1, 
           "unrecoginised format of number \"%1%\".");

ERR_DEFINE("SYN-PARA-0",   
           EError,   1, 
           "parameter \"%1%\" is declared outside a module enviornment.");

ERR_DEFINE("SYN-PARA-1",   
           EError, 2, 
           "parameter \"%1%\" is already defined at %2%.");

// a port declaration is not found. It will be inserted.
ERR_DEFINE("SYN-PORT-0",   
           EError,   2, 
           "port \"%1%\" is not listed in the port list of module \"%2%\".");

// port redefinition. The duplicated one will be ignored.
ERR_DEFINE("SYN-PORT-1",   
           EError, 2, 
           "port \"%1%\" is already defined at %2%.");

// input port without a wire declaration. A declaration is added.
ERR_DEFINE("SYN-PORT-2",   
           EWarning, 1, 
           "port \"%1%\" is declared without a wire/reg declaration.");

// define a reg outside a block or wire outside a module/generate. It will be ignored
ERR_DEFINE("SYN-VAR-0",    
           EError,   1, 
           "variable \"%1%\" is defined at an illegal place.");

// duplicate variable declaration. The duplicated one will be ignored.
ERR_DEFINE("SYN-VAR-1",    
           EError,   2, 
           "variable \"%1%\" is ignored as it was already declared at %2%.");

// variable not found. It will be assumed as a 1-bit wire/reg.
ERR_DEFINE("SYN-VAR-3",    
           EError,   1, 
           "variable (wire/reg/parameter/genvar) \"%1%\" is not found in current block.");

ERR_DEFINE("SYN-VAR-4",    
           EWarning, 1, 
           "in declaration initialisation for variable \"%1%\" will be ignored.");


// Local Variables:
// mode: c++
// End:
