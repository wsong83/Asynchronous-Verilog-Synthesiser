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
 * Definition of netlist components.
 * 14/02/2012   Wei Song
 *
 *
 */

#ifndef _H_MODULE_
#define _H_MODULE_

namespace netlist {
  
  class Module : public NetComp {
  public:
    NETLIST_DEFAULT_CON(Module, tModule);
    Module(const MIdentifier& nm)
      : NetComp(tModule), name(nm) {}

    // helpers
    NETLIST_STREAMOUT_FUN_DECL;
    
    // data
    MIdentifier name;
    DataBase<PoIdentifier, Port, true>     db_port;      /* input and output ports, ordered */
    DataBase<VIdentifier, Variable>        db_wire;      /* wires */
    DataBase<VIdentifier, Variable>        db_reg;       /* registers */
    DataBase<VIdentifier, Variable, true>  db_param;     /* parameters, ordered */
    DataBase<VIdentifier, Variable, true>  db_genvar;    /* generate variable, ordered */
    DataBase<IIdentifier, Instance>        db_instance;  /* module instances */

    DataBase<string, Assign>               db_assign;    /* continueous assignments */
    DataBase<BIdentifier, SeqBlock>        db_block;     /* always blocks */

    //name for unnamed items
    BIdentifier unnamed_block;
    IIdentifier unnamed_instance;
    VIdentifier unnamed_var;

  };

  NETLIST_STREAMOUT(Module);


}

#endif
