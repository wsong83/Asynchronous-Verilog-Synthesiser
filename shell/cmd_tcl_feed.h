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
 * Input feeder for the Tcl command line environment
 * 02/07/2012   Wei Song
 *
 *
 */

#ifndef _CMD_TCL_FEED_H_
#define _CMD_TCL_FEED_H_

#include<fstream>
#include<iostream>
#include "env.h"

namespace shell{
  namespace CMD {

    class CMDTclFeed {
    public:
      CMDTclFeed();
      virtual ~CMDTclFeed();

      //helper
      bool initialise(Env *, const std::string& fn = "");
      std::string getline();        // read a line of Tcl command

    private:
      char* lex_buf;
      unsigned int rp, fp;         // read pointer and full position
      unsigned int bufsize;        // record the current buffer size
      std::ifstream * file_stream; // the initial script file 
      Env * gEnv;
      std::istream& cstream(bool&);
      void getline_priv(bool m_prop = true); // the internal get line function to fill the buffer
      void check_buf_size();    // increase the buffer if necessary
    };

  }
}

#endif /* _CMD_TCL_FEED_H_ */

// Local Variables:
// mode: c++
// End:
