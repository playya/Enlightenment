MAINTAINERCLEANFILES = Makefile.in

if BUILD_ENGINE_SDL

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/include \
-I$(top_srcdir)/src/engines/enesim/ \
@ECORE_SDL_CFLAGS@ @SDL_CFLAGS@ @EON_CFLAGS@

noinst_LTLIBRARIES = libeon_engine_sdl.la

libeon_engine_sdl_la_SOURCES = \
sdl_engine.c

libeon_engine_sdl_la_LIBADD = \
@ECORE_SDL_LIBS@ @SDL_LIBS@

endif
