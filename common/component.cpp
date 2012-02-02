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

  switch(text[index]) {
  case 'b':
  case 'B': valid = bin2num(text, txt_leng, index); return;
  case 'd':
  case 'D': valid = dec2num(text, txt_leng, index); return;
  case 'O':
  case 'o': valid = oct2num(text, txt_leng, index); return;
  case 'H':
  case 'h': valid = hex2num(text, txt_leng, index); return;
  default: return;		// wrong number format
  }
}

bool netlist::Number::bin2num(char *text, int txt_leng, int start) {
  int i = num_leng + 4;
  txt_value.resize(i, '0');
  valuable = true;
  for(int index=txt_leng-1; (i>4 || index==start); index--) {
    switch(text[index]) {
    case '0': txt_value[--i] = '0'; break;
    case '1': txt_value[--i] = '1'; break;
    case 'x':
    case 'X': txt_value[--i] = 'x'; valuable = false; break;
    case 'z':
    case 'Z': txt_value[--i] = 'z'; valuable = false; break;
    case '_': break;
    default: return false;
    }
  }
  txt_value.erase(0,4);
  update_value();
  return true;
}

bool netlist::Number::oct2num(char *text, int txt_leng, int start) {
  int i = num_leng + 4;
  txt_value.resize(i, '0');
  valuable = true;
  for(int index=txt_leng-1; (i>4 || index==start); index--) {
    switch(text[index]) {
    case '0': txt_value.replace(i-=3, 3, "000"); break;
    case '1': txt_value.replace(i-=3, 3, "001"); break;
    case '2': txt_value.replace(i-=3, 3, "010"); break;
    case '3': txt_value.replace(i-=3, 3, "011"); break;
    case '4': txt_value.replace(i-=3, 3, "100"); break;
    case '5': txt_value.replace(i-=3, 3, "101"); break;
    case '6': txt_value.replace(i-=3, 3, "110"); break;
    case '7': txt_value.replace(i-=3, 3, "111"); break;
    case 'x':
    case 'X': txt_value.replace(i-=3, 3, "xxx"); valuable = false; break;
    case 'z':
    case 'Z': txt_value.replace(i-=3, 3, "zzz"); valuable = false; break;
    case '_': break;
    default: return false;
    }
  }
  txt_value.erase(0,4);
  update_value();
  return true;
}

bool netlist::Number::hex2num(char *text, int txt_leng, int start) {
  int i = num_leng + 4;
  txt_value.resize(i, '0');
  valuable = true;
  for(int index=txt_leng-1; (i>4 || index==start); index--) {
    switch(text[index]) {
    case '0': txt_value.replace(i-=4, 4, "0000"); break;
    case '1': txt_value.replace(i-=4, 4, "0001"); break;
    case '2': txt_value.replace(i-=4, 4, "0010"); break;
    case '3': txt_value.replace(i-=4, 4, "0011"); break;
    case '4': txt_value.replace(i-=4, 4, "0100"); break;
    case '5': txt_value.replace(i-=4, 4, "0101"); break;
    case '6': txt_value.replace(i-=4, 4, "0110"); break;
    case '7': txt_value.replace(i-=4, 4, "0111"); break;
    case '8': txt_value.replace(i-=4, 4, "1000"); break;
    case '9': txt_value.replace(i-=4, 4, "1001"); break;
    case 'A':
    case 'a': txt_value.replace(i-=4, 4, "1010"); break;
    case 'B':
    case 'b': txt_value.replace(i-=4, 4, "1011"); break;
    case 'C':
    case 'c': txt_value.replace(i-=4, 4, "1100"); break;
    case 'D':
    case 'd': txt_value.replace(i-=4, 4, "1101"); break;
    case 'E':
    case 'e': txt_value.replace(i-=4, 4, "1110"); break;
    case 'F':
    case 'f': txt_value.replace(i-=4, 4, "1111"); break;
    case 'x':
    case 'X': txt_value.replace(i-=4, 4, "xxxx"); valuable = false; break;
    case 'z':
    case 'Z': txt_value.replace(i-=4, 4, "zzzz"); valuable = false; break;
    case '_': break;
    default: return false;
    }
  }
  txt_value.erase(0,4);
  update_value();
  return true;
}

void netlist::Number::update_value() {
  value = 0;

  if(!valuable || num_leng > 32) {
    valuable = false; return;
  }

  int cyc = num_leng/4;
  int rem = num_leng%4;
  int index = 0;

  for(; index<rem; index++) {
    value = value * 2 + txt_value[index] - '0';
  }

  for(int i=0; i<cyc; i++, index+=4) {
    value 
      = value * 16 
      + (txt_value[index  ]-'0') * 8
      + (txt_value[index+1]-'0') * 4
      + (txt_value[index+2]-'0') * 2
      + (txt_value[index+3]-'0') * 1
      ;
  }
}
  
void netlist::Number::update_txt_value() {
  unsigned int dd = value;
  int i = num_leng+4;

  if(!valuable) return;

  txt_value.resize(i, '0');

  for( ; i>4; i-=4) {
    txt_value[i-1] = (dd>>0)%2 + '0';
    txt_value[i-2] = (dd>>1)%2 + '0';
    txt_value[i-3] = (dd>>2)%2 + '0';
    txt_value[i-4] = (dd>>3)%2 + '0';
  }

  txt_value.erase(0,4);
}
