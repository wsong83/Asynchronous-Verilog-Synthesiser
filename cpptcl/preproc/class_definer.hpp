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
     class_definer & def(std::string const &name,
          R (C::*f)(BOOST_PP_ENUM_PARAMS(n,T)), policies const &p = policies())
     {
          ch_->register_method(name,
               boost::shared_ptr<details::object_cmd_base>(
                    new details::BOOST_PP_CAT(method,n)<C, R BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n,T)>(f)),
               p);
          return *this;
     }

     template <typename R BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename T)>
     class_definer & def(std::string const &name,
          R (C::*f)(BOOST_PP_ENUM_PARAMS(n,T)) const,
          policies const &p = policies())
     {
          ch_->register_method(name,
               boost::shared_ptr<details::object_cmd_base>(
                    new details::BOOST_PP_CAT(method,n)<C, R BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n,T)>(f)),
               p);
          return *this;
     }
#undef n
