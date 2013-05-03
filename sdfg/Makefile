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
# 17/09/2012   Wei Song
#
#

# variables
INCDIRS = -I./ -I../ -I$(OGDF_PATH)
COMM_HEADERS = sdfg.hpp dfg_common.hpp

SDFG_TARGET = dfg_common.o dfg_node.o dfg_edge.o dfg_path.o dfg_graph.o
AVS_SDFG_TARGET = dfg_node_avs.o dfg_graph_avs.o
RTREE_TARGET = rtree.o

# actions
all: $(SDFG_TARGET) $(AVS_SDFG_TARGET) $(RTREE_TARGET)

$(SDFG_TARGET): %.o:%.cpp %.hpp $(COMM_HEADERS)
	$(CXX) $(INCDIRS) $(CXXFLAGS) -c $< -o $@

$(AVS_SDFG_TARGET): %_avs.o:%_avs.cpp %.hpp $(COMM_HEADERS)
	$(CXX) $(INCDIRS) $(CXXFLAGS) -c $< -o $@

$(RTREE_TARGET): %.o:%.cpp %.hpp $(COMM_HEADERS)
	$(CXX) $(INCDIRS) $(CXXFLAGS) -c $< -o $@

.PHONY: clean

clean:
	-rm *.o
