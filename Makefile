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
# Top level Makefile
# Due to be replace by autoconf tool suite
# 06/02/2012   Wei Song
#
#

# global variables
#TARGET = RELEASE
TARGET = DEBUG

export BISON_EXE = bison
export FLEX_EXE = flex
export CXX = g++

# OGDF library
export OGDF_PATH = $(PWD)/../OGDF

ifeq ($(TARGET),RELEASE)
	export OGDF_LIB = $(OGDF_PATH)/_release/libOGDF.a
	export CXXFLAGS = -std=c++0x -Wall -Wextra -o2
else
	export OGDF_LIB = $(OGDF_PATH)/_debug/libOGDF.a
	export CXXFLAGS = -std=c++0x -Wall -Wextra -g
endif

export LINKFLAGS = -lboost_regex -lgmpxx -lgmp -lboost_system -lboost_filesystem -lboost_program_options -ltcl -lboost_graph -lpthread $(OGDF_LIB)

# targets
SUBDIRS = preproc averilog netlist shell shell/cmd cpptcl sdfg pugixml tool
TESTDIRS = preproc/test shell/test
BISONDIRS = averilog.bison

# actions

all: bison subdirs testdirs
	-mkdir bin
	-cp shell/test/avs_shell bin/

debug: 

.PHONY: bison subdirs $(SUBDIRS) clean testdirs $(TESTDIRS)

bison: $(BISONDIRS)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

shell.bison:
	$(MAKE) bison -C shell

averilog.bison:
	$(MAKE) bison -C averilog

testdirs: $(TESTDIRS)

$(TESTDIRS):
	$(MAKE) -C $@

clean:
	-rm *.o
	-for d in $(SUBDIRS); do $(MAKE) -C $$d clean; done
	-for d in $(TESTDIRS); do $(MAKE) -C $$d clean; done
	-rm bin/avs_shell

