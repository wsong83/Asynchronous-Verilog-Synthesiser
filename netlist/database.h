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

/*
#include <list>
using std::list;
#include <map>
using std::map;
#include <utility>
using std::pair;
#include <boost/shared_ptr.hpp>
using boost::shared_ptr;
#include <ostream>
using std::ostream;
*/

namespace netlist {

  // the database
  template <typename K, typename T, bool ORDER = false>
    class DataBase {
  public:
    typedef map<K, shared_ptr<T> > DBTM;
    typedef list<pair<K, shared_ptr<T> > > DBTL;
    typedef pair<K, shared_ptr<T> > DTT;
    
    // store a component
    bool insert(const K& key, const shared_ptr<T>& comp) {
      if(ORDER) {
        if(find(key).use_count() != 0){
          return false;
        } else {
          db_list.push_back(DTT(key, comp));
          return true;
        }
      } else {
        pair<typename DBTM::iterator, bool> res = db_map.insert(DTT(key, comp));
        return res.second;
      }
    }
    
    shared_ptr<T> find(const K& key) {
      if(ORDER) {
        typename DBTL::iterator it, end;
        
        for(it=db_list.begin(), end=db_list.end(); it!=end; it++)
          if(it->first == key) break;
        
        if(it != end)
          return it->second;
        else
          return shared_ptr<T>();

      } else {
        typename DBTM::iterator it = db_map.find(key);

        if(it != db_map.end())
          return it->second;
        else
          return shared_ptr<T>();
      }
    }
    
    unsigned int cout() const {
      if(ORDER)
        return db_list.size();
      else
        return db_map.size();
    }
    
    void clear() {
      db_list.clear();
      db_map.clear();
    }
    
    bool empty() const {
      if(ORDER)
        return db_list.empty();
      else
        return db_map.empty();
    }
    
    bool erase(const K& key) {
      if(ORDER) {
        typename DBTL::iterator it, end;
        for(it=db_list.begin(), end=db_list.end(); it!=end; it++)
          if(it->first == key) {
            db_list.erase(it);
            return true;
          }
        return false;
      } else {
        return 1 == db_map.erase(key);
      }
    }
    
    shared_ptr<T> fetch(const K& key) {
      shared_ptr<T> m = find(key);
      if(m.use_count() != 0) erase(key);
      return m;
    }
    
    typename DBTL::iterator begin_order()             { return db_list.begin(); }
    typename DBTM::iterator begin()                   { return db_map.begin();  }
    typename DBTL::const_iterator begin_order() const { return db_list.begin(); }
    typename DBTM::const_iterator begin() const       { return db_map.begin();  }
    typename DBTL::iterator end_order()               { return db_list.end();   }
    typename DBTM::iterator end()                     { return db_map.end();    }
    typename DBTL::const_iterator end_order() const   { return db_list.end();   }
    typename DBTM::const_iterator end() const         { return db_map.end();    }

    ostream& streamout(ostream& os, unsigned int indent) const {
      if(ORDER) {
        typename DBTL::const_iterator it, end;
        for(it=db_list.begin(), end=db_list.end(); it!=end; it++)
          it->second->streamout(os, indent);
      } else {
        typename DBTM::const_iterator it, end;
        for(it=db_map.begin(), end=db_map.end(); it!=end; it++)
          it->second->streamout(os, indent);
      }
      return os;
    }
    
  private:
    DBTM db_map;                /* unordered database */
    DBTL db_list;               /* oredered database */
    
  };
  
  template<typename K, typename T, bool ORDER>
    ostream& operator<< ( ostream& os, const DataBase<K,T,ORDER>& rhs) {
    return rhs.streamout(os, 0);
  }
}

#endif
