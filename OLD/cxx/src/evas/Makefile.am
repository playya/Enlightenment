## Process this file with automake to produce Makefile.in

INCLUDES = -I../common -I../ecore

AM_CXXFLAGS = @PACKAGE_CFLAGS@ \
	 -Wall

noinst_LTLIBRARIES = libeflpp_evas.la

libeflpp_evas_la_SOURCES = \
	eflpp_evas.cpp\
	eflpp_evasfont.cpp\
	eflpp_evastextblockstyle.cpp\
	eflpp_evasobject.cpp\
	eflpp_evascanvas.cpp\
	eflpp_evasimage.cpp\
	eflpp_evastext.cpp
    
libeflpp_includedir = $(pkgincludedir)

libeflpp_include_DATA = \
	eflpp_evasevent.h\
	eflpp_evasfont.h\
	eflpp_evastextblockstyle.h\
	eflpp_evas.h\
	eflpp_evasutils.h\
	eflpp_evasobject.h\
	eflpp_evascanvas.h\
	eflpp_evasimage.h\
	eflpp_evastext.h

EXTRA_DIST = $(libeflpp_include_DATA)
