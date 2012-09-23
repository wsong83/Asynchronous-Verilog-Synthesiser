/*
 * Copyright (c) 2012-2012 Wei Song <songw@cs.man.ac.uk> 
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
 * a command line tool to convert sdfg to pdf
 * 22/09/2012   Wei Song
 *
 *
 */

#include "sdfg2pdf.hpp"
#include <fstream>

using std::string;
using boost::shared_ptr;

int main(int argc, char *argv[]) {

  if(argc != 2) return 1;
  string filename(argv[1]);

  std::ifstream infile(filename.c_str());
  shared_ptr<SDFG::dfgGraph> G = SDFG::read(infile);
  infile.close();

  filename += ".copy";
  std::ofstream outfile(filename.c_str());
  G->write(outfile);
  outfile.close();

  return 0;
}
