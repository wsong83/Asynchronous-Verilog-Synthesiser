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

#ifndef _H_COMPONENT_
#define _H_COMPONENT_

#include <string>

namespace netlist {

  class Number {
  public:
    Number(char *text, int txt_leng, int num_leng); /* convert decimal and integer to number */
    Number(char *text, int txt_leng);	    /* convert fixed numbers */
    Number(const string& txt_val, int num_leng, bool valuable); /* convert a binary string to number */    

    int get_value();
    int get_length();
    bool is_valuable();
    bool is_valid();
    Number& operator+ (Number& rhs);
    Number& operator<< (int rhs);
    string& to_string();

    //private:
    unsigned int value;         /* number value, valid when valuable */
    int num_leng;		/* the number of digits in the number */
    std::string txt_value;	/* literals of the number, useful when non-deterministic */
    bool valid;			/* true when number format is ok */
    bool valuable;		/* true when the number is const and deterministic */

    //helpers

    // convert the verilog fixed number field to number structure
    bool bin2num(char *text, int txt_leng, int start);
    bool dec2num(char *text, int txt_leng, int start);
    bool oct2num(char *text, int txt_leng, int start);
    bool hex2num(char *text, int txt_leng, int start);
    void update_value();
    void update_txt_value();

  };

  // overload operators


}


#endif
