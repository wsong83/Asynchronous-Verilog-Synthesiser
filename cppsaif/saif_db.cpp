/*
* Copyright (c) 2014-2014 Wei Song <songw@cs.man.ac.uk>
* Advanced Processor Technologies Group, School of Computer Science
* University of Manchester, Manchester M13 9PL UK
*
* This source code is free software; you can redistribute it
* and/or modify it in source code form under the terms of the GNU
* General Public License as published by the Free Software
* Foundation; either version 2 of the License, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
*/

/*
* A saif database
* 06/02/2014 Wei Song
*
*
*/

#include "saif_db.hpp"
#include <boost/foreach.hpp>

using std::string;

saif::SaifRecord::SaifRecord() {
  T0 = 0;
  T1 = 0;
  TX = 0;
  TZ = 0;
  TC = 0;
  IG = 0;
  TB = 0;
}

std::ostream& saif::SaifRecord::streamout( std::ostream& os) const {
  os << "(T0 " << T0.get_str() << ") ";
  os << "(T1 " << T1.get_str() << ") ";
  os << "(TX " << TX.get_str() << ") ";
  os << "(TZ " << TZ.get_str() << ") ";
  os << "(TC " << TC.get_str() << ") ";
  os << "(IG " << IG.get_str() << ") ";
  os << "(TB " << TB.get_str() << ") ";
  return os;
}

std::ostream& saif::SaifSignal::streamout( std::ostream& os) const {
  return streamout(os, "", "");
}

std::ostream& saif::SaifSignal::streamout( std::ostream& os, const std::string& sig, const std::string& dim, unsigned int indent) const {
  if(bits.empty()) {
    os << std::string(indent, ' ') << "(" << sig << dim << std::endl;
    os << std::string(indent, ' ') << "    " << *data << std::endl;
    os << std::string(indent, ' ') << ")" << std::endl;
  } else {
    typedef std::pair<const int, boost::shared_ptr<SaifSignal> > signal_type;
    BOOST_FOREACH(signal_type it, bits) {
      mpz_class m = it.first;
      std::string m_dim = dim + "[" + m.get_str() + "]";
      it.second->streamout(os, sig, m_dim, indent);
    }
  }
  return os;
}

std::ostream& saif::SaifInstance::streamout( std::ostream& os) const {
  return streamout(os, "");
}

std::ostream& saif::SaifInstance::streamout( std::ostream& os, const std::string& name, unsigned int indent) const {
  os << std::string(indent, ' ') << "(INSTANCE ";

  if(module_name.empty())
    os << name << std::endl;
  else
    os << "\"" << module_name << "\" " << name << std::endl;

  if(!signals.empty()) {
    os << std::string(indent+2, ' ') << "(NET" << std::endl;
    typedef std::pair<const string&, boost::shared_ptr<SaifSignal> > signal_type;
    BOOST_FOREACH(signal_type it, signals) {
      it.second->streamout(os, it.first, "", indent+4);
    }
    os << std::string(indent+2, ' ') << ")" << std::endl;
  }

  if(!instances.empty()) {
    typedef std::pair<const string&, boost::shared_ptr<SaifInstance> > instance_type;
    BOOST_FOREACH(instance_type it, instances) {
      it.second->streamout(os, it.first, indent+2);
    }
  }

  os << std::string(indent, ' ') << ")" << std::endl;

  return os;
}

mpz_class saif::SaifDB::get_duration(const std::string& unit) const {
  
  mpz_class full_duration;

  if(timescale.second == "fs")
    full_duration = duration * timescale.first;
  else if(timescale.second == "ps")
    full_duration = duration * timescale.first * mpz_class("1000");
  else if(timescale.second == "ns")
    full_duration = duration * timescale.first * mpz_class("1000000");
  else if(timescale.second == "us")
    full_duration = duration * timescale.first * mpz_class("1000000000");
  else if(timescale.second == "ms")
    full_duration = duration * timescale.first * mpz_class("1000000000000");
  else if(timescale.second == "s")
    full_duration = duration * timescale.first * mpz_class("1000000000000000");
  else
    assert(0 == "wrong timescale unit in the saif file");

  if(unit == "fs")
    return full_duration;
  else if(unit == "ps")
    return full_duration / mpz_class("1000");
  else if(unit == "ns")
    return full_duration / mpz_class("1000000");
  else if(unit == "us")
    return full_duration / mpz_class("1000000000");
  else if(unit == "ms")
    return full_duration / mpz_class("1000000000000");
  else if(unit == "s")
    return full_duration / mpz_class("1000000000000000");
  else
    return 0;
}

std::ostream& saif::SaifDB::streamout( std::ostream& os) const {
  os << "(SAIFILE" << std::endl;
  os << "(SAIFVERSION \"" << version << "\")" << std::endl;
  os << "(DIRECTION \"" << direction << "\")" << std::endl;
  os << "(DESIGN)" << std::endl;
  os << "(DATE \"" << date << "\")" << std::endl;
  os << "(VENDOR \"" << vendor << "\")" << std::endl;
  os << "(PROGRAM_NAME \"" << program_name << "\")" << std::endl;
  os << "(VERSION \"" << tool_version << "\")" << std::endl;
  os << "(DIVIDER " << divider << ")" << std::endl;
  os << "(TIMESCALE " << timescale.first.get_str() << " " << timescale.second << ")" << std::endl;
  os << "(DURATION " << duration.get_str() << ")" << std::endl;
  top->streamout(os, top_name);
  os << ")" << std::endl;
  return os;
}
