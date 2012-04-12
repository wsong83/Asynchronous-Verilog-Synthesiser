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
 * Definition of netlist components.
 * 01/02/2012   Wei Song
 *
 *
 */

#include <cctype>
#include <algorithm>
#include "component.h"

using namespace netlist;

// decimal or integer
netlist::Number::Number(char *text, int txt_leng) 
  : NetComp(tNumber), valid(false), valuable(false)
{
  string m;
  int i;
  for(i = 0; i < txt_leng; i++) {
    if(isdigit(text[i]))
      m.push_back(text[i]);
    else if(text[i] != '_')
      break;			// error
  }
  
  if(m.empty()) m.push_back('1');

  mpz_class dd(m, 10);
  
  if(i == txt_leng) { 		// simple number
    valid = true;
    valuable = true;
    txt_value = dd.get_str(2);
    num_leng = txt_value.size();
  } else {			// fixed number
    num_leng = dd.get_si();
    if(text[i++] != '\'') return; // wrong format
    switch(text[i]) {
    case 'b':
    case 'B': valid = bin2num(text, txt_leng, i); return;
    case 'd':
    case 'D': valid = dec2num(text, txt_leng, i); return;
    case 'O':
    case 'o': valid = oct2num(text, txt_leng, i); return;
    case 'H':
    case 'h': valid = hex2num(text, txt_leng, i); return;
    default: return;		// wrong number format
    }
  }
}

netlist::Number::Number(int d) 
  : NetComp(tNumber), valid(true), valuable(true)
{
  mpz_class m(d);
  txt_value = m.get_str(2);
  num_leng = txt_value.size();
}

netlist::Number::Number(const mpz_class& m) 
  : NetComp(tNumber), valid(true), valuable(true)
{
  txt_value = m.get_str(2);
  num_leng = txt_value.size();
}

netlist::Number::Number(const string& txt)
  : NetComp(tNumber), num_leng(txt.size()), txt_value(txt), valid(true)
{
  check_valuable();
}

Number& netlist::Number::truncate (int lhs, int rhs) { // no sign support
  assert(lhs >= 0 && (unsigned int)(lhs) <= num_leng && rhs >= 0 && rhs <= lhs);
  txt_value = txt_value.substr(txt_value.length()-lhs-1, lhs-rhs+1);
  num_leng = lhs-rhs+1;
  return *this;
}

Number netlist::Number::addition(const Number& rhs) const {
  assert(valuable && rhs.valuable);
  Number m(0);
  mpz_class d1(txt_value, 2);
  mpz_class d2(rhs.txt_value, 2);
  d1 = d1 + d2;
  m.txt_value = d1.get_str(2);
  m.num_leng = m.txt_value.size();
  return m;
}

Number netlist::Number::minus(const Number& rhs) const {
  assert(valuable && rhs.valuable);
  Number m(0);
  mpz_class d1(txt_value, 2);
  mpz_class d2(rhs.txt_value, 2);
  d1 = d1 - d2;
  m.txt_value = d1.get_str(2);
  m.num_leng = m.txt_value.size();
  return m;
}

Number& netlist::Number::operator+= (const Number& rhs) {
  assert(valuable && rhs.valuable);
  mpz_class d(txt_value, 2);
  d = d + mpz_class(rhs.txt_value, 2);
  txt_value = d.get_str(2);
  num_leng = txt_value.size();
  return *this;
}

Number& netlist::Number::lfsh (int rhs) {
  assert(rhs >= 0);

  txt_value.append(rhs, '0');
  num_leng += rhs;
  return *this;
}

ostream& netlist::Number::streamout (ostream& os, unsigned int indent) const{
  os << string(indent, ' ');
  if(valuable) {		// able to be represented as a decimal
    mpz_class d(txt_value,2);
    if(d > MAX_INT_IN_STREAMOUT || d < -MAX_INT_IN_STREAMOUT)
      os << num_leng << "'h" << d.get_str(16);
    else
      os << num_leng << "'d" << d;
  } else
    os << num_leng << "'b" << txt_value;
  return os;
}

Number* netlist::Number::deep_copy() const {
  return (new Number(*this));
}

void netlist::Number::concatenate(const Number& rhs) {
  txt_value = txt_value + rhs.txt_value;
  num_leng += rhs.num_leng;
  valuable = valuable && rhs.valuable;
}


bool netlist::Number::bin2num(char *text, int txt_leng, int start) {
  string m;
  bool v = true;

  for(int i=start+1; i<txt_leng; i++) {
    if(text[i] >= '0' && text[i] <= '1')
      m.push_back(text[i]);
    else if(text[i] == 'x' || text[i] == 'X') {
      m.push_back('x');
      v = false;
    } else if(text[i] == 'z' || text[i] == 'z') {
      m.push_back('z');
      v = false;
    } else if(text[i] != '_')
      return false;		// wrong format
  }

  valuable = v;
  txt_value = m;
  
  return true;
}

bool netlist::Number::dec2num(char *text, int txt_leng, int start) {
  string m;

  for(int i=start+1; i<txt_leng; i++) {
    if(text[i] >= '0' && text[i] <= '9')
      m.push_back(text[i]);
    else if(text[i] != '_')
      return false;		// wrong format
  }

  mpz_class dd(m, 10);
  txt_value  = dd.get_str(2);
  if(txt_value.size() > num_leng) txt_value.erase(0, txt_value.size() - num_leng);
  else if(txt_value.size() < num_leng) txt_value.insert(0, num_leng - txt_value.size(), '0');
  valuable = true;
  
  return true;
}

bool netlist::Number::oct2num(char *text, int txt_leng, int start) {
  string m;
  bool v = true;

  for(int i=start+1; i<txt_leng; i++) {
    if(text[i] >= '0' && text[i] <= '7')
      m.push_back(text[i]);
    else if(text[i] == 'x' || text[i] == 'X') {
      m.push_back('x');
      v = false;
    } else if(text[i] == 'z' || text[i] == 'z') {
      m.push_back('z');
      v = false;
    } else if(text[i] != '_')
      return false;		// wrong format
  }

  if(v) {			// valuable
    valuable = true;
    mpz_class dd(m, 8);
    txt_value = dd.get_str(2);
  } else {
    valuable = false;
    for(unsigned int i=0; i<m.size(); i++)
      switch(m[i]) {
      case '0': txt_value += "000"; break;
      case '1': txt_value += "001"; break;
      case '2': txt_value += "010"; break;
      case '3': txt_value += "011"; break;
      case '4': txt_value += "100"; break;
      case '5': txt_value += "101"; break;
      case '6': txt_value += "110"; break;
      case '7': txt_value += "111"; break;
      }
  }
  
  if(txt_value.size() > num_leng) txt_value.erase(0, txt_value.size() - num_leng);
  else if(txt_value.size() < num_leng) txt_value.insert(0, num_leng - txt_value.size(), '0');

  return true;
}

bool netlist::Number::hex2num(char *text, int txt_leng, int start) {
  string m;
  bool v = true;

  for(int i=start+1; i<txt_leng; i++) {
    if(text[i] >= '0' && text[i] <= '9')
      m.push_back(text[i]);
    else if(text[i] == 'a' || text[i] == 'A') {
      m.push_back('a');
    }
    else if(text[i] == 'b' || text[i] == 'B') {
      m.push_back('b');
    }
    else if(text[i] == 'c' || text[i] == 'C') {
      m.push_back('c');
    }
    else if(text[i] == 'd' || text[i] == 'D') {
      m.push_back('d');
    }
    else if(text[i] == 'e' || text[i] == 'E') {
      m.push_back('e');
    }
    else if(text[i] == 'f' || text[i] == 'F') {
      m.push_back('f');
    }
    else if(text[i] == 'x' || text[i] == 'X') {
      m.push_back('x');
      v = false;
    } else if(text[i] == 'z' || text[i] == 'z') {
      m.push_back('z');
      v = false;
    } else if(text[i] != '_')
      return false;		// wrong format
  }

  if(v) {			// valuable
    valuable = true;
    mpz_class dd(m, 16);
    txt_value = dd.get_str(2);
  } else {
    valuable = false;
    for(unsigned int i=0; i<m.size(); i++)
      switch(m[i]) {
      case '0': txt_value += "0000"; break;
      case '1': txt_value += "0001"; break;
      case '2': txt_value += "0010"; break;
      case '3': txt_value += "0011"; break;
      case '4': txt_value += "0100"; break;
      case '5': txt_value += "0101"; break;
      case '6': txt_value += "0110"; break;
      case '7': txt_value += "0111"; break;
      case '8': txt_value += "1000"; break;
      case '9': txt_value += "1001"; break;
      case 'A':
      case 'a': txt_value += "1010"; break;
      case 'B':
      case 'b': txt_value += "1011"; break;
      case 'C':
      case 'c': txt_value += "1100"; break;
      case 'D':
      case 'd': txt_value += "1101"; break;
      case 'E':
      case 'e': txt_value += "1110"; break;
      case 'F':
      case 'f': txt_value += "1111"; break;
      case 'x':
      case 'X': txt_value += "xxxx"; break;
      case 'z':
      case 'Z': txt_value += "zzzz"; break;
      }
  }
  
  if(txt_value.size() > num_leng) txt_value.erase(0, txt_value.size() - num_leng);
  else if(txt_value.size() < num_leng) txt_value.insert(0, num_leng - txt_value.size(), '0');
  return true;
}

bool netlist::Number::check_valuable() {
  for(unsigned int i=0; i<txt_value.size(); i++)
    if(txt_value[i] == 'x' || txt_value[i] == 'z') {
      valuable = false;
      return false;
    }

  valuable = true;
  return true;
}

Number netlist::operator+ (const Number& lhs, const Number& rhs) {
  return lhs.addition(rhs);
}

Number netlist::operator- (const Number& lhs, const Number& rhs) {
  return lhs.minus(rhs);
}

bool netlist::operator== (const Number& lhs, const Number& rhs) {
  if(lhs.get_txt_value() == rhs.get_txt_value() && 
     string::npos == lhs.get_txt_value().find('x') &&
     string::npos == lhs.get_txt_value().find('z'))
    return true;
  else
    return false;
}

Number netlist::operator<< (const Number& lhs, int rhs) {
  Number dd(lhs);
  dd.lfsh(rhs);
  return dd;
}

