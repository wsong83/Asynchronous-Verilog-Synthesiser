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
 * A general database for different network components
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
    class DataBase {
  public:
    typedef std::map<K, boost::shared_ptr<T> > DBT;
    typedef std::pair<K, boost::shared_ptr<T> > DTT;
    
    // store a component
    bool insert(const K& key, boost::shared_ptr<T> comp) {
      std::pair<typename DBT::iterator, bool> res = db.insert(DTT(key, comp));
      return res.second;
    }

    boost::shared_ptr<T> find(const K& key) {
      typename DBT::iterator it = db.find(key);
      if(it != db.end())
        return (*it).second;
      else
        return boost::shared_ptr<T>();
    }

    unsigned int cout() const {
      return db.size();
    }

    void clear() {
      db.clear();
    }
    
    bool empty() const {
      return db.empty();
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
        boost::shared_ptr<T> rv = (*it).second;
        db.erase(it);
        return rv;
      } else
        return boost::shared_ptr<T>();
    }

    typename std::map<K, boost::shared_ptr<T> >::iterator begin() {
      return db.begin();
    }

    typename std::map<K, boost::shared_ptr<T> >::const_iterator begin() const {
      return db.begin();
    }
    
    typename std::map<K, boost::shared_ptr<T> >::iterator end() {
      return db.begin();
    }

    typename std::map<K, boost::shared_ptr<T> >::const_iterator end() const {
      return db.begin();
    }
    
    std::ostream& streamout(std::ostream& os) const {
      typename DBT::const_iterator it;
      typename DBT::const_iterator end = db.end();
      for (it = db.begin(); it != end; it++)
        os << *((*it).second);
      return os;
    }

  private:
    DBT db;
  };

  template<typename K, typename T>
    std::ostream& operator<< ( std::ostream& os, const DataBase<K,T>& rhs) {
    return rhs.streamout(os);
  }
}

#endif
