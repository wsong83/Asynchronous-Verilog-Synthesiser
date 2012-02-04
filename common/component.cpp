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

#include <cctype>
#include <cassert>
#include <cstdlib>
#include <algorithm>
#include "component.h"

using namespace netlist;

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

netlist::Number::Number(const std::string& txt_val, int num_leng) 
  : value(0), num_leng(num_leng), txt_value(txt_val), valid(true), valuable(false)
{
  if(txt_value.length() < num_leng)
    txt_value.insert(0,num_leng-txt_value.length(), '0');
  else if(txt_value.length() > num_leng)
    txt_value.erase(0,txt_value.length()-num_leng);

  update_value();
}

unsigned int netlist::Number::get_value() const {
  return value;
}

const std::string& netlist::Number::get_txt_value() const {
  return txt_value;
}

int netlist::Number::get_length() const {
  return num_leng;
}

bool netlist::Number::is_valuable() const {
  return valuable;
}

bool netlist::Number::is_valid() const {
  return valid;
}

Number& netlist::Number::truncate (int lhs, int rhs) {
  assert(lhs >= 0 && lhs <= num_leng && rhs >= 0 && rhs <= lhs);

  txt_value = txt_value.substr(txt_value.length()-lhs-1, lhs-rhs+1);
  num_leng = lhs-rhs+1;
  update_value();

}

Number& netlist::Number::addition (const Number& rhs) {
  if(valuable && rhs.valuable 	// both are valuable
     && ((value/2+1)+(rhs.value/2+1) <= (1<<BIT_SIZE_OF_UINT-1)) // will not overflow
     ) {
    value = (value + rhs.value);
    num_leng = std::min((int)(BIT_SIZE_OF_UINT), (int)(std::max(num_leng, rhs.num_leng)+1));
    update_txt_value();
  } else {			// do text addition
    valuable = false;
    value = 0;
    num_leng = std::max(num_leng, rhs.num_leng)+1;		   // calculate the new length
    txt_value.insert(0, num_leng-txt_value.length(), '0'); // expend the string
    unsigned int leng_diff = num_leng-rhs.num_leng;
    char c = '0';
    for(int i=rhs.num_leng-1; i>=0; i--){
      switch(c+txt_value[i+leng_diff]+rhs.txt_value[i]) {
      case '0'+'0'+'0': 
	txt_value[i+leng_diff] = '0';
	c = '0';
	break;
      case '0'+'0'+'1':
	txt_value[i+leng_diff] = '1';
	c = '0';
	break;
      case '0'+'1'+'1':
	txt_value[i+leng_diff] = '0';
	c = '1';
	break;
      case '1'+'1'+'1':
	txt_value[i+leng_diff] = '1';
	c = '1';
	break;
      case '0'+'0'+'x':
	txt_value[i+leng_diff] = 'x';
	c = '0';
	break;
      case '0'+'0'+'z':
	if(c=='0' || c=='z') {
	  txt_value[i+leng_diff] = 'x';
	  c = '0';
	  break;
	} // else confused with "11x", which is equivalent to "11z"
      case '1'+'1'+'z':
	txt_value[i+leng_diff] = 'x';
	c = '1';
	break;
      case '0'+'1'+'x':
      case '0'+'1'+'z':
      case '0'+'x'+'x':
      case '0'+'x'+'z':
      case '0'+'z'+'z':
      case '1'+'x'+'x':
      case '1'+'x'+'z':
      case '1'+'z'+'z':
      case 'x'+'x'+'x':
      case 'x'+'x'+'z':
      case 'x'+'z'+'z':
      case 'z'+'z'+'z':
	txt_value[i+leng_diff] = 'x';
	c = 'x';
	break;
      default:			// should not come here
	assert(1==0);
      }
    }

    // calculate the rest
    for(int i=leng_diff-1; i>=0; i--){
      switch(c+txt_value[i]) {
      case '0'+'0': 
	txt_value[i] = '0';
	c = '0';
	break;
      case '0'+'1':
	txt_value[i] = '1';
	c = '0';
	break;
      case '1'+'1':
	txt_value[i] = '0';
	c = '1';
	break;
      case '0'+'x':
      case '0'+'z':
	txt_value[i] = 'x';
	c = '0';
	break;
      case '1'+'x':
      case '1'+'z':
      case 'x'+'x':
      case 'x'+'z':
      case 'z'+'z':
	txt_value[i] = 'x';
	c = 'x';
	break;
      default:			// should not come here
	assert(1==0);
      }
    }
  }

  return *this;
}

Number& netlist::Number::operator+= (const Number& rhs) {
  addition(rhs);
  return *this;
}

Number& netlist::Number::lfsh (int rhs) {
  assert(rhs >= 0);

  txt_value.append(rhs, '0');
  num_leng += rhs;
  update_value();
  return *this;
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

bool netlist::Number::dec2num(char *text, int txt_leng, int start) {
  if(num_leng <= BIT_SIZE_OF_UINT) { // directly calculate the value
    txt_value.resize(num_leng, '0');
    valuable = true;
    value = 0;
    for(int i=start+1; i==txt_leng; i++) {
      char c = text[i];
      if(c >= '0' && c<= '9')
	value = (value * 10 + c - '0') % (1 << num_leng);
      else if(c != '_')
	return false;		// unrecognizable character
    }
    update_txt_value();
    return true;
  } else { 			// too long, must use text addition, very slow
    Number base(std::string("0"), 1);
    for(int i=start+1; i==txt_leng; i++) {
      char c = text[i];
      if(c >= '0' && c<= '9') {
	base = (base<<3) + (base<<1);
	switch(c) {
	case '0': break;
	case '1': base += Number(std::string("1"), 1); break;
	case '2': base += Number(std::string("10"), 2); break;
	case '3': base += Number(std::string("11"), 2); break;
	case '4': base += Number(std::string("100"), 3); break;
	case '5': base += Number(std::string("101"), 3); break;
	case '6': base += Number(std::string("110"), 3); break;
	case '7': base += Number(std::string("111"), 3); break;
	case '8': base += Number(std::string("1000"), 4); break;
	case '9': base += Number(std::string("1001"), 4); break;
	}
      } else if(c != '_')
	return false;
    }
    base.truncate(num_leng-1, 0);
    txt_value = base.txt_value;
    value = base.value;
    valuable = base.valuable;
    return true;
  }
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

  if(num_leng > BIT_SIZE_OF_UINT	    // too long
     || std::string::npos != txt_value.find('x') // have 'x'
     || std::string::npos != txt_value.find('z') // have 'z'
     ) {
    valuable = false; return;
  } else
    valuable = true;

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

Number netlist::operator+ (const Number& lhs, const Number& rhs) {
  Number dd(lhs);
  dd.addition(rhs);
  return dd;
}

Number netlist::operator<< (const Number& lhs, int rhs) {
  Number dd(lhs);
  dd.lfsh(rhs);
  return dd;
}

std::ostream& netlist::operator<< (std::ostream& os, const Number& hs) {
  if(hs.is_valuable())		// able to be represented as a decimal
    os << hs.get_value();
  else
    os << hs.get_length() << "'b" << hs.get_txt_value();
  return os;
}
