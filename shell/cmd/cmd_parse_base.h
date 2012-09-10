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
 * The base parser to provide some basic rules
 * 06/09/2012   Wei Song
 *
 *
 */

#ifndef _CMD_PARSE_BASE_H_
#define _CMD_PARSE_BASE_H_

// Boost.Spirit
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/support.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/variant/recursive_variant.hpp>

#include <string>
#include <vector>

namespace shell { 
  namespace CMD {
    template<typename Iterator>
      struct cmd_parse_base {
        boost::spirit::qi::rule<Iterator, std::string()> text;
        boost::spirit::qi::rule<Iterator, std::string()> identifier;
        boost::spirit::qi::rule<Iterator, std::string()> filename;
        boost::spirit::qi::rule<Iterator, void()> blanks;

        cmd_parse_base() {
          using boost::spirit::qi::lit;
          using boost::spirit::ascii::char_;
          using boost::spirit::ascii::print;
          using boost::spirit::ascii::alpha;
          using boost::spirit::ascii::alnum;
          using boost::spirit::ascii::digit;
          using boost::spirit::ascii::space;
          text %= (print - '-' - space) >> *(print - space);
          identifier %= (alpha|char_('$')|char_('\\')|char_('_')) 
            >> *(alnum|char_('$')|char_('\\')|char_('_')|char_('/')) 
            >> *(char_('[') >> +digit >> char_(']'));
          filename %= (print - '-' - space) >> *(print - space);
          blanks = +(space) || boost::spirit::qi::eoi;
        }
      };
  }
}

#endif /* _CMD_PARSE_BASE_H_ */
