## Process this file with automake to produce Makefile.in

INCLUDES =

AM_CXXFLAGS =\
         @PACKAGE_CFLAGS@ \
	 -I$(top_srcdir)/src/ecore/ \
	 -I$(top_srcdir)/src/evas/ \
	 -I$(top_srcdir)/src/esmart/ \
	 -I$(top_srcdir)/src/common/ \
	 -Wall


bin_PROGRAMS = eflpp_esmart_group

eflpp_esmart_group_SOURCES = \
	main.cpp 

eflpp_esmart_group_LDFLAGS = @PACKAGE_LIBS@

eflpp_esmart_group_LDADD = $(top_builddir)/src/libeflpp.la

eflpp_esmart_group_DEPENDENCIES = $(top_builddir)/src/libeflpp.la
