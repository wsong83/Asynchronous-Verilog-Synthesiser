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
 * Averilog parser utilities.
 * 01/02/2012   Wei Song
 *
 *
 */

//#include <cstdio>
#include "averilog_util.h"

using namespace averilog;

void averilog::error_report(const string& err_msg, YYLTYPE * yyloc, ostream& os) {
  os << "\n" << *yyloc << " Scanner Error: " << err_msg << endl;
}

ostream& averilog::operator<< (ostream& os, const avID& hs) {
  os << hs.name;
  return os;
}

averilog::Parser::Parser(string fn, shell::Env& env)
  : sfile(NULL), fname(fn), bison_instance(fn, &sfile, env)
{ }

averilog::Parser::~Parser() {
  if(sfile != NULL)
    fclose(sfile);
}

bool averilog::Parser::initialize() {
  sfile = fopen(fname.c_str(), "r");
  return sfile != NULL;
}

bool averilog::Parser::parse() {
  return 0 == bison_instance.parse();
}

  
  

