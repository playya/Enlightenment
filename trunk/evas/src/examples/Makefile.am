MAINTAINERCLEANFILES = Makefile.in

pkglibdir = $(datadir)/$(PACKAGE)/examples

#put here all EDCs one needs to the tests
EDCS = aspect.edc

filesdir = $(datadir)/$(PACKAGE)/examples
files_DATA =

if BUILD_EXAMPLES

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
-DPACKAGE_EXAMPLES_DIR=\"$(datadir)/$(PACKAGE)/examples\" \
@EINA_CFLAGS@ \
@FREETYPE_CFLAGS@ \
@PIXMAN_CFLAGS@ \
@FRIBIDI_CFLAGS@ \
@EET_CFLAGS@ \
@FONTCONFIG_CFLAGS@ \
@pthread_cflags@

AM_CFLAGS = @WIN32_CFLAGS@

pkglib_PROGRAMS =

if BUILD_ENGINE_BUFFER
AM_CPPFLAGS += -I$(top_srcdir)/src/modules/engines/buffer

pkglib_PROGRAMS += evas_buffer_simple
evas_buffer_simple_SOURCES = evas-buffer-simple.c
evas_buffer_simple_LDADD = $(top_builddir)/src/lib/libevas.la
endif

pkglib_PROGRAMS += evas_init_shutdown
evas_init_shutdown_SOURCES = evas-init-shutdown.c
evas_init_shutdown_LDADD = $(top_builddir)/src/lib/libevas.la

#the ones using ecore_evas follow
AM_CPPFLAGS += @ECORE_EVAS_CFLAGS@

pkglib_PROGRAMS += evas_images
evas_images_SOURCES = evas-images.c
evas_images_LDADD = $(top_builddir)/src/lib/libevas.la @ECORE_EVAS_LIBS@

pkglib_PROGRAMS += evas_events
evas_events_SOURCES = evas-events.c
evas_events_LDADD = $(top_builddir)/src/lib/libevas.la @ECORE_EVAS_LIBS@

pkglib_PROGRAMS += evas_object_manipulation
evas_object_manipulation_SOURCES = evas-object-manipulation.c
evas_object_manipulation_LDADD = $(top_builddir)/src/lib/libevas.la @ECORE_EVAS_LIBS@

pkglib_PROGRAMS += evas_hints
evas_hints_SOURCES = evas-hints.c
evas_hints_LDADD = $(top_builddir)/src/lib/libevas.la @ECORE_EVAS_LIBS@

pkglib_PROGRAMS += evas_stacking
evas_stacking_SOURCES = evas-stacking.c
evas_stacking_LDADD = $(top_builddir)/src/lib/libevas.la @ECORE_EVAS_LIBS@

pkglib_PROGRAMS += evas_images2
evas_images2_SOURCES = evas-images2.c
evas_images2_LDADD = $(top_builddir)/src/lib/libevas.la @ECORE_EVAS_LIBS@

pkglib_PROGRAMS += evas_text
evas_text_SOURCES = evas-text.c
evas_text_LDADD = $(top_builddir)/src/lib/libevas.la @ECORE_EVAS_LIBS@

pkglib_PROGRAMS += evas_smart_object
evas_smart_object_SOURCES = evas-smart-object.c
evas_smart_object_LDADD = $(top_builddir)/src/lib/libevas.la @ECORE_EVAS_LIBS@

pkglib_PROGRAMS += evas_box
evas_box_SOURCES = evas-box.c
evas_box_LDADD = $(top_builddir)/src/lib/libevas.la @ECORE_EVAS_LIBS@ @EINA_LIBS@

#the ones using ecore_evas and edje follow
AM_CPPFLAGS += @EDJE_CFLAGS@

pkglib_PROGRAMS += evas_aspect_hints
evas_aspect_hints_DEPS = $(srcdir)/aspect.edc
evas_aspect_hints_SOURCES = evas-aspect-hints.c
evas_aspect_hints_LDADD = $(top_builddir)/src/lib/libevas.la @ECORE_EVAS_LIBS@ @EDJE_LIBS@

pkglib_PROGRAMS += evas_map_utils
evas_map_utils_SOURCES = evas-map-utils.c
evas_map_utils_LDADD = $(top_builddir)/src/lib/libevas.la @ECORE_EVAS_LIBS@

aspect.edj: ${evas_aspect_hints_DEPS}

.edc.edj:
	$(edje_cc) -v -id $(srcdir) $< $(builddir)/$(@F)

EDJS = $(EDCS:%.edc=%.edj)

files_DATA += $(EDJS)

endif # if BUILD_EXAMPLES

if INSTALL_EXAMPLES

#put here additional data when installing examples
files_DATA += $(srcdir)/enlightenment.png \
	$(srcdir)/red.png \
	$(srcdir)/im1.png \
	$(srcdir)/cube1.png

files_DATA += \
	$(EDCS) \
	$(srcdir)/evas-buffer-simple.c \
	$(srcdir)/evas-init-shutdown.c \
	$(srcdir)/evas-images.c \
	$(srcdir)/evas-images2.c \
	$(srcdir)/evas-object-manipulation.c \
	$(srcdir)/evas-events.c \
	$(srcdir)/evas-aspect-hints.c \
	$(srcdir)/evas-hints.c \
	$(srcdir)/evas-stacking.c \
	$(srcdir)/evas-map-utils.c \
	$(srcdir)/evas-text.c \
	$(srcdir)/evas-smart-object.c \
	$(srcdir)/evas-box.c
endif

EXTRA_DIST = $(EDCS) \
	$(srcdir)/evas-buffer-simple.c \
	$(srcdir)/evas-init-shutdown.c \
	$(srcdir)/evas-images.c \
	$(srcdir)/evas-images2.c \
	$(srcdir)/evas-object-manipulation.c \
	$(srcdir)/evas-events.c \
	$(srcdir)/evas-aspect-hints.c \
	$(srcdir)/evas-hints.c \
	$(srcdir)/evas-stacking.c \
	$(srcdir)/evas-map-utils.c \
	$(srcdir)/evas-text.c \
	$(srcdir)/evas-smart-object.c \
	$(srcdir)/evas-box.c \
	$(srcdir)/enlightenment.png \
	$(srcdir)/red.png \
	$(srcdir)/im1.png \
	$(srcdir)/cube1.png
