/*
 * Copyright (c) 2011 Wei Song <songw@cs.man.ac.uk> 
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
 * Definition of netlist components.
 * 01/02/2011   Wei Song
 *
 *
 */

#include <ctype.h>
#include <stdlib.h>
#include "component.h"

// decimal or integer
netlist::Number::Number(char *text, int txt_leng, int num_leng) 
  : value(0), num_leng(num_leng), txt_value(num_leng,'0'), 
    valid(false), valuable(false) 
{
  for(int i = 0; i < txt_leng; i++) {
    if(isdigit(text[i]))
      value = value * 10 + text[i] - '0';
    else if(text[i] != '_')
      return;
  }

  unsigned int tmp = value;
  int i = num_leng-1;
  while(tmp != 0) {
    txt_value[i--] = tmp%2;
    tmp >>= 1;
  }

  valid = true;
  valuable = true;
}

// fixed numbers
netlist::Number::Number(char *text, int txt_leng)
  : value(0), num_leng(0), valid(false), valuable(false)
{
  int index;

  // get the num_leng
  if(!isdigit(text[index]))
    num_leng = 1;
  else {
    while(isdigit(text[index]) || (text[index]=='_')) {
      if(isdigit(text[index]))
	num_leng = num_leng * 10 + text[index] - '0';
      index++;
    }
  }

  if(num_leng == 0) return;	// wrong number leng, empty number

  switch(text[index++]) {
  case 'b':
  case 'd':
  case 'o':
  case 'h':
  default: return;		// wrong number format
  }
