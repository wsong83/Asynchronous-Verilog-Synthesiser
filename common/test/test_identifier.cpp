/*
 * Copyright (c) 2012 Wei Song <songw@cs.man.ac.uk> 
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
 * Test programe for identifiers
 * 06/02/2012   Wei Song
 *
 */

#include "component.h"
#include <iostream>

using namespace netlist;

int main(){
  BIdentifier empty;
  BIdentifier named("test_identifier");
  
  cout << empty << " " << empty.hashid << endl;
  cout << named << " " << named.hashid << endl;
  cout << (++empty) << " " << empty.hashid << endl;
  cout << (++empty) << " " << empty.hashid << endl;
  cout << ((++empty) < named) << endl;

  MIdentifier m1("module_1");
  MIdentifier m2("\\o83gr4 g01834module[]_2");

   cout << m1 << " " << m1.hashid << endl;
   cout << m2 << " " << m2.hashid << endl;
   cout << ++m1 << " " << m1.hashid << endl;
   cout << ++m2 << " " << m2.hashid << endl;
   cout << ++m1 << " " << m1.hashid << endl;
   cout << ++m2 << " " << m2.hashid << endl;
 

  
  return 0;
}
