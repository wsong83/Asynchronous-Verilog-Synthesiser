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
 * Const Numbers
 * 01/02/2012   Wei Song
 *
 *
 */

#ifndef AV_H_NUMBER_
#define AV_H_NUMBER_
#include <gmpxx.h>

namespace netlist {

  class Number : public NetComp{
  public:
    // constructors
    Number() : NetComp(tNumber), num_leng(0), valid(false), valuable(false), sign_flag(false) {};
    Number(const char *text, const int txt_leng); /* constructor for scanner */
    Number(const shell::location& lloc, const char *text, const int txt_leng); /* constructor for scanner */
    Number(int d);
    Number(const std::string&);
    Number(const shell::location& lloc, const std::string&);
    Number(const mpz_class&);

    // helpers
    const std::string get_txt_value() const { 
      if(txt_value.empty()) return "0"; 
      else return txt_value; 
    }
    mpz_class get_value() const;
    unsigned int get_length() const { return num_leng; }
    void set_length(const unsigned int& ns) { num_leng = ns; }
    void set_signed();            // change a default unsigned number to signed number
    bool is_signed() const {return sign_flag; }
    bool is_valuable() const { return valuable; }
    bool is_valid() const { return valid; }
    bool is_true() const { return std::string::npos != txt_value.find('1'); }
    bool is_false() const { return trim_zeros(txt_value) == "0"; }
    bool is_x() const { 
      return 
        std::string::npos != txt_value.find('x') ||
        std::string::npos != txt_value.find('X') ||
        std::string::npos != txt_value.find('z') ||
        std::string::npos != txt_value.find('Z');
    }
    Number addition (const Number& rhs) const;
    Number minus (const Number& rhs) const;
    Number& operator+= (const Number& rhs);
    
    void concatenate(const Number&); /* concatenate two fixed-point number */
    static std::string trim_zeros(const std::string&);

    // inherit from NetComp
    NETLIST_STREAMOUT_DECL;
    NETLIST_CHECK_INPARSE_DECL;
    virtual Number* deep_copy() const;
    virtual void db_register(int iod) {}
    virtual void db_expunge() {}
    NETLIST_ELABORATE_DECL;

  private:
    unsigned int num_leng;	// the number of digits in the number
    std::string txt_value;	// literals of the number, useful when non-deterministic
    bool valid;			    // true when number format is ok
    bool valuable;		    // true when the number is const and deterministic
    bool sign_flag;         // true when it is a signed number

    //helpers

    // convert the verilog fixed number field to number structure
    bool bin2num(const char *text, int txt_leng, int start);
    bool dec2num(const char *text, int txt_leng, int start);
    bool oct2num(const char *text, int txt_leng, int start);
    bool hex2num(const char *text, int txt_leng, int start);
    bool check_valuable();
  };

  // overload operators
  Number op_uor    (const Number& lhs);
  Number op_uand   (const Number& lhs);
  Number op_uxor   (const Number& lhs);
  Number operator- (const Number& lhs);
  Number operator! (const Number& lhs);
  Number operator~ (const Number& lhs);
  Number operator* (const Number& lhs, const Number& rhs);
  Number operator/ (const Number& lhs, const Number& rhs);
  Number operator% (const Number& lhs, const Number& rhs);
  Number operator+ (const Number& lhs, const Number& rhs);
  Number operator- (const Number& lhs, const Number& rhs);
  Number operator& (const Number& lhs, const Number& rhs);
  Number operator| (const Number& lhs, const Number& rhs);
  Number operator^ (const Number& lhs, const Number& rhs);
  Number operator&& (const Number& lhs, const Number& rhs);
  Number operator|| (const Number& lhs, const Number& rhs);
  bool operator== (const Number& lhs, const Number& rhs);
  bool operator!= (const Number& lhs, const Number& rhs);
  bool operator< (const Number& lhs, const Number& rhs);
  bool operator<= (const Number& lhs, const Number& rhs);
  bool operator> (const Number& lhs, const Number& rhs);
  bool operator>= (const Number& lhs, const Number& rhs);
  bool op_case_equal (const Number& lhs, const Number& rhs);
  Number operator>> (const Number& lhs, const Number& rhs);
  Number op_sign_rsh (const Number& lhs, const Number& rhs);
  Number operator<< (const Number& lhs, const Number& rhs);
  NETLIST_STREAMOUT(Number)

}


#endif

// Local Variables:
// mode: c++
// End:
