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
 * The variable used in command line environment
 * 17/05/2012   Wei Song
 *
 *
 */

#ifndef AV_CMD_VARIABLE
#define AV_CMD_VARIABLE

#include "netlist/netcomp.h"

namespace shell {
  namespace CMD {

    class CMDVarHook;          /* the call back function when the value of a variable is changed */

    class CMDVar {
    public:
      enum var_t {
        vUnknown,                /* probably empty, unkown yet */
        vString,                /* string */
        vList,                  /* a string list */
      } var_type;

      CMDVar()
        : var_type(vUnknown) {}
      CMDVar(const std::string& rhs) 
        : var_type(vString), m_list(1, rhs) {}
      CMDVar(const std::vector<std::string>& rhs) 
        : var_type(vList), m_list(rhs) {
        if(rhs.size() == 1) {
          var_type = vString;
        }
      }

      // helpers
      //void set_string() { var_type = vString; }
      //void set_list() { var_type = vList; }
      bool is_string() const { return var_type == vString; }
      bool is_list() const { return var_type == vList; }
      const std::string& get_string() const { return m_list.front(); }
      const std::vector<std::string>& get_list() const { return m_list; }
      
      std::ostream& streamout( std::ostream& os) const;
      CMDVar& operator= (const std::string& );
      CMDVar& operator= (const std::list<std::string>& );
      CMDVar& operator= (const std::vector<std::string>& );
      operator std::string() const {return m_list.front();}
      
      boost::shared_ptr<CMDVarHook> hook; /* call back function */

    private:
      std::vector<std::string> m_list;

    };

    inline std::ostream& operator << (std::ostream& os, const CMDVar& rhs) {
      rhs.streamout(os);
      return os;
    }

    class CMDVarHook {          /* the call back function when the value of a variable is changed */
    public:
      virtual bool operator() (Env&, CMDVar&, const std::vector<std::string>&) { return true; }
    };

    // free function

    // resolve the variables in a string
    std::string cmd_variable_resolver(const std::map<std::string, CMDVar>&, const std::string&);

    // check the format of a variable name
    bool cmd_variable_name_checker(const std::string&);

  }
}

#endif
