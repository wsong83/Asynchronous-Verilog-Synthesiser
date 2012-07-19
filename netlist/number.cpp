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
#include <boost/foreach.hpp>
#include "component.h"

using namespace netlist;
using std::ostream;
using std::string;
using shell::location;

// decimal or integer
netlist::Number::Number(const char *text, const int txt_leng) 
  : NetComp(tNumber), valid(false), valuable(false), sign_flag(false)
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

netlist::Number::Number(const location& lloc, const char *text, const int txt_leng) 
  : NetComp(tNumber, lloc), valid(false), valuable(false), sign_flag(false)
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
  : NetComp(tNumber), valid(true), valuable(true), sign_flag(d < 0)
{
  mpz_class m(d);
  txt_value = m.get_str(2);
  if(sign_flag) {               // negative number
    txt_value[0] = '0';
    mpz_class k(txt_value.c_str(), 2);
    k = (~k + 1);
    txt_value = k.get_str(2);
  }
  num_leng = txt_value.size();
}

netlist::Number::Number(const mpz_class& m) 
  : NetComp(tNumber), valid(true), valuable(true), sign_flag(m < 0)
{
  txt_value = m.get_str(2);
  if(sign_flag) {               // negative number
    txt_value[0] = '0';
    mpz_class k(txt_value.c_str(), 2);
    k = (~k + 1);
    txt_value = k.get_str(2);
  }
  num_leng = txt_value.size();
}

netlist::Number::Number(const string& txt)
  : NetComp(tNumber), num_leng(txt.size()), txt_value(txt), valid(true), sign_flag(false)
{
  if(txt.size() >0 && txt[0]=='-') {               // negative number
    txt_value[0] = '0';
    mpz_class k(txt_value.c_str(), 2);
    k = (~k + 1);
    txt_value = k.get_str(2);
    sign_flag = true;
  }
  check_valuable();
}

netlist::Number::Number(const location& lloc, const string& txt)
  : NetComp(tNumber, lloc), num_leng(txt.size()), txt_value(txt), valid(true), sign_flag(false)
{
  check_valuable();
}

mpz_class netlist::Number::get_value() const {
  if(sign_flag && txt_value.size() > 0 && txt_value[0] == '1') { // signed negative number
    mpz_class k(txt_value.c_str(), 2);
    k = (~k + 1);
    string result = "-" + k.get_str(2);
    return mpz_class(result.c_str(), 2);
  } else if(txt_value.size() > 0){
    return mpz_class(txt_value.c_str(), 2);
  } else {
    return mpz_class(0);
  }
}

void netlist::Number::set_signed() {
  sign_flag = true;
  if(txt_value.size() > 0 && txt_value[0] == '1') {
    txt_value.insert(0, 1, '0');
    if(txt_value.size() > num_leng) num_leng++;
  }
}

Number netlist::Number::addition(const Number& rhs) const {
  assert(valuable && rhs.valuable);
  return Number(get_value() + rhs.get_value());
}

Number netlist::Number::minus(const Number& rhs) const {
  assert(valuable && rhs.valuable);
  return Number(get_value() - rhs.get_value());
}

Number& netlist::Number::operator+= (const Number& rhs) {
  assert(valuable && rhs.valuable);
  mpz_class d = get_value();
  d += rhs.get_value();
  *this = Number(d);
  return *this;
}

ostream& netlist::Number::streamout (ostream& os, unsigned int indent) const{
  os << string(indent, ' ');
  if(valuable) {		// able to be represented as a decimal
    mpz_class d = get_value();
    if(d > MAX_INT_IN_STREAMOUT || d < -MAX_INT_IN_STREAMOUT)
      os << num_leng << "'h" << d.get_str(16);
    else
      os << num_leng << "'d" << d;
  } else
    os << num_leng << "'b" << txt_value;
  return os;
}

bool netlist::Number::check_inparse() {
  return valid;
}

Number* netlist::Number::deep_copy() const {
  return (new Number(*this));
}

bool netlist::Number::elaborate(NetComp::elab_result_t &result, const NetComp::ctype_t mctype, const std::vector<NetComp *>& fp) {
  // number is the simpliest form it could be, so directly return OK
  result = NetComp::ELAB_Normal;
  return true;
}

void netlist::Number::concatenate(const Number& rhs) {
  string ss = rhs.get_txt_value();
  string stuff;
  if(rhs.is_signed() && ss.size() > 0 && ss[0] == '1') {
    stuff = string(rhs.get_length() - ss.size(), '1');
  } else {
    stuff = string(rhs.get_length() - ss.size(), '0');
  }

  txt_value = txt_value + stuff + ss;
  num_leng += rhs.num_leng;
  valuable = valuable && rhs.valuable;
  sign_flag = false;            // once concatenated, all signed number turn to be unsigned
}

unsigned int netlist::Number::get_width() {
  return num_leng;
}

void netlist::Number::set_width(const unsigned int& w) {
  if(txt_value.size() <= w) {    // expand
    num_leng = w;
    if(sign_flag && txt_value.size() > 0 && txt_value[0] == '1') { // negtive value
      txt_value.insert(0, w - txt_value.size(), '1');
    } 
  } else {                      // reduce the size
    num_leng = w;
    txt_value = txt_value.substr(txt_value.size()-w);
    sign_flag = false;          // once reduced, it is no longer signed number
  }
}

bool netlist::Number::bin2num(const char *text, const int txt_leng, const int start) {
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

bool netlist::Number::dec2num(const char *text, const int txt_leng, const int start) {
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

bool netlist::Number::oct2num(const char *text, const int txt_leng, const int start) {
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

bool netlist::Number::hex2num(const char *text, const int txt_leng, const int start) {
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

std::string netlist::Number::trim_zeros(const std::string& str){
  if(str.empty()) return "0";
  std::size_t pos = str.find_first_not_of("0");
  if(pos == std::string::npos) {
    return "0";
  } else {
    return str.substr(pos);
  }
}

Number netlist::op_uor (const Number& lhs) {
  if(lhs.is_true()) return Number("1");
  if(lhs.is_false()) return Number("0");
  return Number("x");
}

Number netlist::op_uand (const Number& lhs) {
  string lstr = Number::trim_zeros(lhs.get_txt_value());
  if(lhs.get_length() > lstr.size()) return Number("0");
  if(lhs.is_x()) return Number("x");
  if(lstr.size() > 0 && string::npos == lstr.find('0')) return  Number("1");
  else return Number("0");
}

Number netlist::op_uxor (const Number& lhs) {
  string lstr = Number::trim_zeros(lhs.get_txt_value());
  if(lstr.empty()) return Number("0");
  if(lstr.size() == 1) return Number(lstr);
  else {
    char b = '0';
    BOOST_FOREACH(const char& m, lstr) {
      if(m == '1') {
        if(b == '0') b = '1';
        if(b == '1') b = '0';
      } else if(m != '0') {
        b = 'x';
        break;
      }
    }
    return Number(string(1, b));
  }
}

Number netlist::operator- (const Number& lhs) {
  assert(lhs.is_valuable());
  return Number(-lhs.get_value());
}

Number netlist::operator! (const Number& lhs) {
  if(lhs.is_true()) return Number("0");
  if(lhs.is_false()) return Number("1");
  return Number("x");
}

Number netlist::operator~ (const Number& lhs) {
  string lstr = lhs.get_txt_value();
  if(lstr.size() < lhs.get_length())
    lstr.insert(lstr.begin(), lhs.get_length() - lstr.size(), '0');
  BOOST_FOREACH(char& m, lstr) {
    if(m == '0') m = '1';
    else if(m == '1') m = '0';
    else m = 'x';
  }
  return Number(lstr);
}

Number netlist::operator* (const Number& lhs, const Number& rhs) {
  assert(lhs.is_valuable() && rhs.is_valuable());
  return Number(lhs.get_value() * rhs.get_value());
}

Number netlist::operator/ (const Number& lhs, const Number& rhs) {
  assert(lhs.is_valuable() && rhs.is_valuable());
  return Number(lhs.get_value() / rhs.get_value());
}

Number netlist::operator% (const Number& lhs, const Number& rhs) {
  assert(lhs.is_valuable() && rhs.is_valuable());
  return Number(lhs.get_value() % rhs.get_value());
}

Number netlist::operator+ (const Number& lhs, const Number& rhs) {
  return lhs.addition(rhs);
}

Number netlist::operator- (const Number& lhs, const Number& rhs) {
  return lhs.minus(rhs);
}

Number netlist::operator& (const Number& lhs, const Number& rhs) {
  string lstr = Number::trim_zeros(lhs.get_txt_value());
  string rstr = Number::trim_zeros(rhs.get_txt_value());
  if(lstr.size() > rstr.size())
    rstr.insert(rstr.begin(), lstr.size() - rstr.size(), '0');
  else 
    lstr.insert(lstr.begin(), rstr.size() - lstr.size(), '0');
  string rv(lstr.size(), '0');
  for(unsigned int i=0; i<lstr.size(); i++) {
    if(lstr[i] == '1' && rstr[i] == '1') rv[i] = '1';
    else if(lstr[i] == '0' || rstr[i] == '0') rv[i] = '0';
    else rv[i] = 'x';
  }
  return Number(rv);
}
  
Number netlist::operator| (const Number& lhs, const Number& rhs) {
  string lstr = Number::trim_zeros(lhs.get_txt_value());
  string rstr = Number::trim_zeros(rhs.get_txt_value());
  if(lstr.size() > rstr.size())
    rstr.insert(rstr.begin(), lstr.size() - rstr.size(), '0');
  else 
    lstr.insert(lstr.begin(), rstr.size() - lstr.size(), '0');
  string rv(lstr.size(), '0');
  for(unsigned int i=0; i<lstr.size(); i++) {
    if(lstr[i] == '0' && rstr[i] == '0') rv[i] = '0';
    else if(lstr[i] == '1' || rstr[i] == '1') rv[i] = '1';
    else rv[i] = 'x';
  }
  return Number(rv);
}
  

Number netlist::operator^ (const Number& lhs, const Number& rhs) {
  string lstr = Number::trim_zeros(lhs.get_txt_value());
  string rstr = Number::trim_zeros(rhs.get_txt_value());
  if(lstr.size() > rstr.size())
    rstr.insert(rstr.begin(), lstr.size() - rstr.size(), '0');
  else 
    lstr.insert(lstr.begin(), rstr.size() - lstr.size(), '0');
  string rv(lstr.size(), '0');
  for(unsigned int i=0; i<lstr.size(); i++) {
    if(lstr[i] == rstr[i]) {
      if(rstr[i] == '0' || rstr[i] == '1') rv[i] = '0';
      else rv[i] = 'x';
    } else if(lstr[i] != rstr[i]) {
      if((rstr[i] == '0' || rstr[i] == '1') && (lstr[i] == '0' || lstr[i] == '1'))
        rv[i] = '1';
      else rv[i] = 'x';
    }
  }
  return Number(rv);
}

Number netlist::operator&& (const Number& lhs, const Number& rhs) {
  if(lhs.is_false() || rhs.is_false() )
    return Number("0");
  else if(lhs.is_x() || rhs.is_x() )
    return Number("x");
  else
    return Number("1");
}

Number netlist::operator|| (const Number& lhs, const Number& rhs) {
  if(lhs.is_true() || rhs.is_true() )
    return Number("1");
  else if(lhs.is_x() || rhs.is_x() )
    return Number("x");
  else
    return Number("0");
}

bool netlist::operator== (const Number& lhs, const Number& rhs) {
  if(Number::trim_zeros(lhs.get_txt_value()) == Number::trim_zeros(rhs.get_txt_value()) && 
     string::npos == lhs.get_txt_value().find('x') &&
     string::npos == lhs.get_txt_value().find('z'))
    return true;
  else
    return false;
}

bool netlist::operator!= (const Number& lhs, const Number& rhs) {
  assert(lhs.is_valuable() && rhs.is_valuable()); // the result may be wrong when x or z exists
  if(Number::trim_zeros(lhs.get_txt_value()) != Number::trim_zeros(rhs.get_txt_value()) && 
     string::npos == lhs.get_txt_value().find('x') &&
     string::npos == lhs.get_txt_value().find('z'))
    return true;
  else
    return false;
}

bool netlist::operator< (const Number& lhs, const Number& rhs) {
  assert(lhs.is_valuable() && rhs.is_valuable());
  return lhs.get_value() < rhs.get_value();
}

bool netlist::operator<= (const Number& lhs, const Number& rhs) {
  assert(lhs.is_valuable() && rhs.is_valuable());
  return lhs.get_value() <= rhs.get_value();
}

bool netlist::operator> (const Number& lhs, const Number& rhs) {
  assert(lhs.is_valuable() && rhs.is_valuable());
  return lhs.get_value() > rhs.get_value();
}

bool netlist::operator>= (const Number& lhs, const Number& rhs) {
  assert(lhs.is_valuable() && rhs.is_valuable());
  return lhs.get_value() >= rhs.get_value();
}

bool netlist::op_case_equal(const Number& lhs, const Number& rhs) {
  return (Number::trim_zeros(lhs.get_txt_value()) == Number::trim_zeros(rhs.get_txt_value()));
}

Number netlist::operator>>(const Number& lhs, const Number& rhs) {
  long rhs_si = rhs.get_value().get_si();
  assert(rhs_si >= 0);

  rhs_si = rhs_si > static_cast<long>(lhs.get_length()) ? 
    static_cast<long>(lhs.get_length()) : rhs_si;
  string new_value = lhs.get_txt_value();
  new_value = new_value.substr(0, 
                               rhs_si > static_cast<long>(new_value.size()) 
                               ? 0 : static_cast<long>(new_value.size()) - rhs_si);
  Number rv(new_value);
  rv.set_length(lhs.get_length());   // size should not change
  return rv;
}

Number netlist::op_sign_rsh(const Number& lhs, const Number& rhs) {
  assert(lhs.is_valuable() && rhs.is_valuable());
  return lhs.get_value() >> rhs.get_value();
}

Number netlist::operator<< (const Number& lhs, const Number& rhs) {
  long rhs_si = rhs.get_value().get_si();
  assert(rhs_si >= 0);
  string m = lhs.get_txt_value();
  Number rv(m.append(rhs_si, '0'));
  rv.set_length(rhs_si + lhs.get_length());
  return rv;
}

