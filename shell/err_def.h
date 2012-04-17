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
           "Unkown parser error. Please report to the developers <wsong83@gmail.com>.");

/// on the left side of an assign should be a non-const variable or concatenation
ERR_DEFINE("SYN-ASSIGN-0", 
           EError,   0, 
           "Wrong left-side value.");

ERR_DEFINE("SYN-ASSIGN-1", 
           EError,   0, 
           "Keyword \"assign\" can only be used in module/generate enviornment.");

ERR_DEFINE("SYN-FILE-0",   
           EError,   0, 
           "File terminated in multi-line comments.");

// duplicated instance name. The name of the duplicated instance will be numbered
ERR_DEFINE("SYN-INST-0",   
           EError,   1, 
           "Instance \"%1%\" is already defined.");

ERR_DEFINE("SYN-INST-1",   
           EWarning,   0, 
           "Primary gate is unnamed and will be named by a default name.");

// duplicated module definition. The duplicated module will be ignored.
ERR_DEFINE("SYN-MODULE-0", 
           EError,   1, 
           "Module \"%1%\" is already defined.");

ERR_DEFINE("SYN-NUM-0",    
           EError,   1, 
           "Unrecoginised format of number \"%1%\".");

ERR_DEFINE("SYN-PARA-0",   
           EError,   1, 
           "Parameter \"%1%\" is declared outside a module enviornment.");

// a port declaration is not found. It will be inserted.
ERR_DEFINE("SYN-PORT-0",   
           EError,   2, 
           "Port \"%1%\" is not found in the port declaration list in module \"%2%\".");

// port redefinition. The duplicated one will be ignored.
ERR_DEFINE("SYN-PORT-1",   
           EError, 2, 
           "Port \"%1%\" is already defined at %2% while the current one take effect.");

// define a reg outside a block or wire outside a module/generate. It will be ignored
ERR_DEFINE("SYN-VAR-0",    
           EError,   1, 
           "variable \"%1%\" is defined at an illegal place.");

// duplicate variable declaration. The duplicated one will be ignored.
ERR_DEFINE("SYN-VAR-1",    
           EError,   2, 
           "\"%1% %2%\" is already declared.");

// variable not found. It will be assumed as a 1-bit wire/reg.
ERR_DEFINE("SYN-VAR-3",    
           EError,   1, 
           "Variable (wire/reg/parameter) \"%1%\" is not found in current block.");

ERR_DEFINE("SYN-VAR-4",    
           EWarning, 1, 
           "In declaration initialisation for variable \"%1%\" will be ignored.");
