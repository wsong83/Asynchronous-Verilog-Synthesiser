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

#ifndef _H_NUMBER_
#define _H_NUMBER_
#include <gmpxx.h>

namespace netlist {

  class Number : public NetComp{
  public:
    // constructors
    Number() : NetComp(tNumber), valid(false), valuable(false) {}
    Number(char *text, int txt_leng); /* constructor for scanner */
    Number(int d);
    Number(const string&);
    Number(const mpz_class&);

    // helpers
    const string& get_txt_value() const { return txt_value; }
    mpz_class get_value() const { return mpz_class(txt_value, 2); }
    int get_length() const { return num_leng; }
    bool is_valuable() const { return valuable; }
    bool is_valid() const { return valid; }
    Number& truncate (int lhs, int rhs);
    Number addition (const Number& rhs) const;
    Number minus (const Number& rhs) const;
    Number& operator+= (const Number& rhs);
    Number& lfsh (int rhs);
    void concatenate(const Number&); /* concatenate two fixed-point number */

    // inherit from NetComp
    NETLIST_STREAMOUT_FUN_DECL;
    virtual Number* deep_copy() const;

    // operation
    void negate() {}

  private:
    unsigned int num_leng;	/* the number of digits in the number */
    string txt_value;	/* literals of the number, useful when non-deterministic */
    bool valid;			/* true when number format is ok */
    bool valuable;		/* true when the number is const and deterministic */

    //helpers

    // convert the verilog fixed number field to number structure
    bool bin2num(char *text, int txt_leng, int start);
    bool dec2num(char *text, int txt_leng, int start);
    bool oct2num(char *text, int txt_leng, int start);
    bool hex2num(char *text, int txt_leng, int start);
    bool check_valuable();
  };

  // overload operators
  Number operator+ (const Number& lhs, const Number& rhs);
  Number operator- (const Number& lhs, const Number& rhs);
  bool operator== (const Number& lhs, const Number& rhs);
  Number operator<< (const Number& lhs, int rhs);
  NETLIST_STREAMOUT(Number)

}


#endif
