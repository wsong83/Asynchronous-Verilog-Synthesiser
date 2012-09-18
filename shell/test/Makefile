# Copyright (c) 2011-2012 Wei Song <songw@cs.man.ac.uk> 
#    Advanced Processor Technologies Group, School of Computer Science
#    University of Manchester, Manchester M13 9PL UK
#
#    This source code is free software; you can redistribute it
#    and/or modify it in source code form under the terms of the GNU
#    General Public License as published by the Free Software
#    Foundation; either version 2 of the License, or (at your option)
#    any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
#
# Makefile
# Due to be replace by autoconf tool suite
# 14/05/2012   Wei Song
#
#

# variables
INCDIRS = -I../../
TARGETS = avs_shell
OTHER_OBJS = $(wildcard ../../netlist/*.o)
OTHER_OBJS += $(wildcard ../../shell/*.o)
OTHER_OBJS += $(wildcard ../../shell/cmd/*.o)
OTHER_OBJS += $(wildcard ../../preproc/*.o)
OTHER_OBJS += $(wildcard ../../averilog/*.o)
OTHER_OBJS += $(wildcard ../../cpptcl/*.o)
OTHER_OBJS += $(wildcard ../../sdfg/*.o)

# targets
all: $(TARGETS)

$(TARGETS): % : %.cpp $(OTHER_OBJS)
	$(CXX) $(INCDIRS) $(CXXFLAGS) $< $(OTHER_OBJS) $(LINKFLAGS) -o $@

.PHONY: clean

clean:
	-rm *.o $(TARGETS)

