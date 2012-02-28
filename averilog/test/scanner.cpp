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
 * 
 * 06/02/2012   Wei Song
 *
 *
 */

#include "netlist/component.h"
#include "averilog/averilog_util.h"
#include "averilog/averilog.lex.h"

int main(int argc, char*argv[])
{
  int tmp;
  YYSTYPE lval;
  yyscan_t scanner;
  YYLTYPE yyloc;
  FILE * sfile;

  sfile = fopen(argv[1], "r");
  string fn(argv[1]);
  yyloc.initialize(&fn);
  avlex_init (&scanner);
  avset_in(sfile, scanner);
  while((tmp = avlex(&lval, &yyloc, scanner)) != 0) {
    cout << tmp << " ";
    if(tmp == token::number)
      cout << "Number:" << lval.tNumber << " ";
  }
    
  avlex_destroy(scanner);
  fclose(sfile);
  return 0;
}
