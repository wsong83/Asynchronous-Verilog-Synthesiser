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
 * Expressions
 * 08/02/2011   Wei Song
 *
 *
 */

#include "expression.h"

using namespace netlist;

netlist::Expression::Expression(unsigned int exp) {}

netlist::Expression::Expression(const Number& exp) {}

bool netlist::Expression::is_valuable() const {return false;}

int netlist::Expression::get_value() const { return 0; }

bool netlist::Expression::operator== (const Expression& rhs) const {
  return false;
}

Expression netlist::operator+ (const Expression& lhs, const Expression& rhs) {
  return Expression(0);
}

Expression netlist::operator- (const Expression& lhs, const Expression& rhs) {
  return Expression(0);
}

std::ostream& netlist::Expression::streamout(std::ostream& os) const {
  os << "expression";
  return os;
}
