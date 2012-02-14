/*
 * Copyright (c) 2012 Wei Song <songw@cs.man.ac.uk> 
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
 * Definition of netlist components.
 * 14/02/2012   Wei Song
 *
 *
 */

#ifndef _H_DATABASE_
#define _H_DATABASE_

#include <map>
#include <boost/shared_ptr.hpp>

namespace netlist {

  template <typename K, typename T>
    class database {
  public:
    typedef std::map<K, boost::shared_ptr<T> > DBT;
    typedef std::pair<K, boost::shared_ptr<T> > DTT;
    
    // store a component
    bool store(const K& key, boost::shared_ptr<T> comp) {
      std::pair<typename DBT::iterator, bool> res = db.insert(DTT(key, comp));
      return res.second;
    }

    boost::shared_ptr<T> find(const K& key) const {
      typename DBT::iterator it = db.find(key);
      if(it != db.end())
        return *it;
      else
        return boost::shared_ptr<T>();
    }

    unsigned int cout() const {
      return db.size();
    }

    void clear() {
      db.clear();
    }

    bool erase(const K& key) {
      return 1 == db.erase(key);
    }

    bool swap(const K& key, boost::shared_ptr<T> comp) {
      typename DBT::iterator it = db.find(key);
      if(it != db.end()) {
        db.erase(it);
        db.insert(it, comp);
        return true;
      } else {
        return false;
      }
    }

    boost::shared_ptr<T> fatch(const K& key) {
      typename DBT::iterator it = db.find(key);
      if(it != db.end()) {
        boost::shared_ptr<T> rv = *it;
        db.erase(it);
        return rv;
      } else
        return boost::shared_ptr<T>();
    }      

  private:
    DBT db;
  };

}

#endif
