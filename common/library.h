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
 * Definition of a library
 * 15/02/2012   Wei Song
 *
 *
 */

#ifndef _H_VA_LIBRARY_
#define _H_VA_LIBRARY_

#include <boost/shared_ptr.hpp>

namespace netlist {

  class Library {
  public:
    Library(
	    const std::string& nm = std::string("work"),    /* library name */
	    const std::string& fn = std::string("work.db"), /* library file name */
	    const std::string& ph = std::string("./")	    /* library path name */
	    )
      : name(nm), path(ph), file_name(fn) {}


    // helpers
    bool pull();		/* read in the library from disk */
    // read in with explicit path + file name
    bool pull(const std::string& fn, const std::string& ph = std::string("./"));
    bool push();		/* write the library to disk */
    // write with explicit path + file name
    bool push(const std::string& fn, const std::string& ph = std::string("./"));

    // insert a new module by its name
    bool insert(const MIdentifier& mn) { 
      boost::shared_ptr<Module> mm(new Module(mn));
      bool rv = db.insert(mn, mm); 
      if(rv) list.push_front(mm);
      return rv;
    }
    
    // insert a new module by its object
    bool insert(boost::shared_ptr<Module> mm) { 
      bool rv = db.insert(mm->name, mm); 
      if(rv) list.push_front(mm);
      return rv;
    }
    
    // find a module in the library
    boost::shared_ptr<Module> find(const MIdentifier& mn) { return db.find(mn); } 
    
    // return a pointer to the current netlist component
    boost::shared_ptr<NetComp> get_current_comp() { return list.front(); }
    
    // return a iterator of the current item list
    std::list<boost::shared_ptr<NetComp> >::iterator get_current_it() { return list.begin(); }
    
    // check whether the iterator is valid in the list
    bool it_valid(const std::list<boost::shared_ptr<NetComp> >::iterator& it) const { return it != list.end(); }
    
    // push one item to the process list
    void push(boost::shared_ptr<NetComp> item) { list.push_front(item); }
    
    // pop the top item from the process list
    void pop() { list.pop_front(); }

    // data items
    std::string name;		/* the name of library */
    std::string path;		/* the path for the database on disk */
    std::string file_name;	/* the file name of the disk copy */

    DataBase<MIdentifier, Module> db;	/* the practical module database */

    // helper items
    /* instead of using a stack, a list is provide to track the
       parse or elaboration process.
       A list provide better access to father components instead
       only the direct father.
     */
    std::list<boost::shared_ptr<NetComp> >   list; 
  };
}

#endif
