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
 * Using boost preprocessor to reduce the burden of writing templates
 * 06/07/2012   Wei Song
 *
 */

#define n BOOST_PP_ITERATION()
     template <typename R BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename T)>
     void def(std::string const &name, R (*f)(BOOST_PP_ENUM_PARAMS(n,T)),
          policies const &p = policies())
     {
          add_function(name,
               boost::shared_ptr<details::callback_base>(
               new details::BOOST_PP_CAT(callback,n)<R BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n,T)>(f)), p);
     }
    
    // using the last arg as a client data, but must be pointer
    template <typename R, BOOST_PP_ENUM_PARAMS(BOOST_PP_ADD(n,1), typename T)>
    void def(std::string const &name, R (*f)(BOOST_PP_ENUM_PARAMS(BOOST_PP_ADD(n,1),T) *), BOOST_PP_CAT(T,n) *cData,
             policies const &p = policies()) {
      add_function(name,
                   boost::shared_ptr<details::callback_base>
                   (new details::BOOST_PP_CAT(BOOST_PP_CAT(callback,BOOST_PP_ADD(n,1)),_cd)<R, BOOST_PP_ENUM_PARAMS(BOOST_PP_ADD(n,1),T)>(f)), p, (ClientData)(cData));
    }
#undef n
