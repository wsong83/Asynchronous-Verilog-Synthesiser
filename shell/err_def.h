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

// non-constant concationation parameter
ERR_DEFINE("ELAB-EXPRESSION-0",
           EError,   1,
           "\"%1\" cannot be reduced to a constant. In concatenation {Var{exp0, exp1, ..}}, Var must be a constant: a const exp, a parameter or a generate variable.");

ERR_DEFINE("ELAB-INST-0", 
           EError,   1, 
           "fail to find the module named \"%1%\".");

ERR_DEFINE("ELAB-INST-1", 
           EError,   2, 
           "fail to find port \"%1%\" in module \"%2%\".");

ERR_DEFINE("ELAB-INST-2", 
           EError,   1, 
           "fail to resolved the parameter assignment of module \"%1%\".");

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

// wire/reg has no driver
ERR_DEFINE("ELAB-VAR-0",
           EError,   1,
           "variable \"%1%\" has no driver.");

// wire/reg has multiple driver
ERR_DEFINE("ELAB-VAR-1",
           EError,   3,
           "variable \"%1%\" has multiple driver, where two of the drivers are located at %2% and %3%.");

// wire/reg has multiple driver
ERR_DEFINE("ELAB-VAR-2",
           EWarning,   1,
           "variable \"%1%\" has no load.");

// wire/reg has no driver
ERR_DEFINE("ELAB-VAR-3",
           EWarning,   1,
           "variable \"%1%\" is defined but not used.");

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

// duplicated instance name. The name of the duplicated instance will be numbered
ERR_DEFINE("SYN-INST-0",   
           EError,   2, 
           "instance \"%1%\" is renamed as another instance is named the same at %2%.");

ERR_DEFINE("SYN-INST-1",   
           EWarning,   0, 
           "primary gate is unnamed and will be named by a default name.");

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
