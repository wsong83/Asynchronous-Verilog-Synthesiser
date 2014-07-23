#-------------------------------------------------
# gml2pdf project file
#
#-------------------------------------------------

# need C++0x
QMAKE_CXXFLAGS += -std=c++0x

# we build a console application
CONFIG += console
# we want debug and release configurations
CONFIG += qt debug warn_on

# link to ogdf library
LIBS += ../../sdfg/dfg_common.o ../../sdfg/dfg_range.o ../../sdfg/dfg_node.o ../../sdfg/dfg_edge.o ../../sdfg/dfg_graph.o
LIBS += ../../pugixml/pugixml.o $${OGDF_PATH}/_debug/libOGDF.a
LIBS += -lpthread 
DEFINES += OGDF_DEBUG

# add ogdf include path
INCLUDEPATH += $${OGDF_PATH}

#add include path
INCLUDEPATH += ../..

#-------------------------------------------------
# source files
#-------------------------------------------------

SOURCES += sdfg2pdf.cpp
