
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS	     = -I. \
		       -I$(top_srcdir)/src/lib \
		       -I$(top_srcdir)/src/lib/include \
		       -DPACKAGE_BIN_DIR=\"$(bindir)\" \
		       -DPACKAGE_LIB_DIR=\"$(libdir)\" \
		       -DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
			@WIN32_CPPFLAGS@ \
			@EVIL_CFLAGS@ \
		       @FREETYPE_CFLAGS@ \
		       @EINA_CFLAGS@

noinst_LTLIBRARIES      = libevas_cache.la
libevas_cache_la_SOURCES  = \
evas_cache_image.c \
evas_cache_engine_image.c \
evas_preload.c

libevas_cache_la_LIBAD = @EVIL_LIBS@

libevas_cache_la_DEPENDENCIES = $(top_builddir)/config.h

EXTRA_DIST = evas_cache.h
