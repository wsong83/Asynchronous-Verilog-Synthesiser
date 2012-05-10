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
 * Data structure to store macros
 * 27/04/2012   Wei Song
 *
 *
 */

#ifndef AVS_MACRO_H_
#define AVS_MACRO_H_

#include<string>
#include<vector>
#include<map>

namespace VPPreProc {

  class VMacro {
  public:
    // constructors
    VMacro( const std::string& nm, /* macro name */
            const std::string& mv, /* macro value */
            const std::string& para = std::string(), /* macro argument */
            bool pre = false                         /* forced macro by command line */
            )
      : m_name(nm), m_para(para), m_value(mv), m_pre(pre) {}

    // helpers
    bool is_parameterized() const { return m_para.size() > 0; }
    
    //data
    std::string m_name;         /* the name of the macro */
    std::string m_para;         /* parameter list */
    std::string m_value;        /* body definition */
    bool m_pre;
  };

}

#endif
