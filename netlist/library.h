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
	    const string& nm = string("work"),    /* library name */
	    const string& fn = string("work.db"), /* library file name */
	    const string& ph = string("./")	    /* library path name */
	    )
      : name(nm), path(ph), file_name(fn) {}


    // helpers
    bool pull();		/* read in the library from disk */
    // read in with explicit path + file name
    bool pull(const string& fn, const string& ph = string("./"));
    bool push();		/* write the library to disk */
    // write with explicit path + file name
    bool push(const string& fn, const string& ph = string("./"));

    // insert a new module by its name
    bool insert(const MIdentifier& mn) { 
      shared_ptr<Module> mm(new Module(mn));
      bool rv = db.insert(mn, mm); 
      if(rv) comp_list.push_front(mm);
      return rv;
    }
    
    // insert a new module by its object
    bool insert(shared_ptr<Module> mm) { 
      bool rv = db.insert(mm->name, mm); 
      if(rv) comp_list.push_front(mm);
      return rv;
    }
    
    // find a module in the library
    shared_ptr<Module> find(const MIdentifier& mn) { return db.find(mn); } 
    
    // return a pointer to the current netlist component
    shared_ptr<NetComp> get_current_comp() { return comp_list.front(); }
    
    // return a iterator of the current item list
    list<shared_ptr<NetComp> >::iterator get_current_it() { return comp_list.begin(); }
    
    // check whether the iterator is valid in the list
    bool it_valid(const list<shared_ptr<NetComp> >::iterator& it) const { return it != comp_list.end(); }
    
    // push one item to the process list
    void push(shared_ptr<NetComp> item) { comp_list.push_front(item); }
    
    // pop the top item from the process list
    void pop() { comp_list.pop_front(); }

    // data items
    string name;		/* the name of library */
    string path;		/* the path for the database on disk */
    string file_name;	/* the file name of the disk copy */

    DataBase<MIdentifier, Module> db;	/* the practical module database */

    // helper items
    /* instead of using a stack, a list is provide to track the
       parse or elaboration process.
       A list provide better access to father components instead
       only the direct father.
     */
    list<shared_ptr<NetComp> >  comp_list; 
  };
}

#endif
