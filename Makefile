# Copyright (c) 2011 Wei Song <songw@cs.man.ac.uk> 
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
export BISON_EXE = ~/tool/local/bin/bison
export FLEX_EXE = ~/tool/local/bin/flex
export CXX = g++

export CXXFLAGS = -Wall -g

# targets
SUBDIRS = common averilog/src
TESTDIRS = averilog/test common/test

# actions

all: subdirs

test: subdirs testdirs

.PHONY: subdirs $(SUBDIRS) clean testdirs $(TESTDIRS)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

testdirs: $(TESTDIRS)

$(TESTDIRS):
	$(MAKE) -C $@

clean:
	-rm *.o
	-for d in $(SUBDIRS); do $(MAKE) -C $$d clean; done
	-for d in $(TESTDIRS); do $(MAKE) -C $$d clean; done

