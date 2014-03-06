/*
 * Copyright (c) 2014-2014 Wei Song <songw@cs.man.ac.uk> 
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
 * Helper classes for the saif parser
 * 06/02/2014   Wei Song
 *
 *
 */

#ifndef SAIF_UTIL_H_
#define SAIF_UTIL_H_

#include <string>
#include <iostream>
#include <gmpxx.h>
#include <list>
#include "saif_db.hpp"

#define YYSTYPE saif::saif_token_type

namespace saif {

  struct saif_token_type {
    std::string                          tStr;              // string
    std::string                          tVar;              // variable
    mpz_class                            tNum;              // number
    std::pair<unsigned int, mpz_class>   tAct;              // activity
    boost::shared_ptr<saif::SaifRecord>  tRecord;           // a saif record
    std::pair<std::string, boost::shared_ptr<saif::SaifSignal> >
                                         tSig;              // signal
    std::map<std::string, boost::shared_ptr<saif::SaifSignal> >
                                         tSigList;          // signal list
    boost::shared_ptr<saif::SaifInstance> 
                                         tInst;             // saif instance pair
    std::pair<std::string, boost::shared_ptr<saif::SaifInstance> >
                                         tInstPair;         // saif instance pair
    std::map<std::string, boost::shared_ptr<saif::SaifInstance> >
                                         tInstList;         // saif instance list
  };

  class SaifLexer {
  public:
    SaifLexer(std::istream *);
    int lexer(saif_token_type *);
    

  private:
    enum state_t {
      S_BEGIN = 0,
      S_DEF,
      S_DIV,
      S_TIMESCALE,
      S_DUR,
      S_INST,
      S_PORT,
      S_SIG,
      S_STR
    };

    std::istream * istm;        // input stream
    std::string buf;
    std::list<std::pair<unsigned int, unsigned int> > state;
    std::string m_string;

    std::string next_token();   // get the next token
    bool validate_token(const std::string&, saif_token_type *, int&); // analyse the token

    // helper
    bool token_helper(int, bool, unsigned int,
                      unsigned int, bool, int&, bool);
  };

  std::string signal_name_normalizer(const std::string&);
  std::string signal_name_parser(const std::string&, std::list<int>&);
}


#endif
