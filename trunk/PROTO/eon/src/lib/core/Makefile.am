MAINTAINERCLEANFILES = \
Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/include \
@EON_CFLAGS@

CLEANFILES = \
libeon_core_la-eon_grammar.c \
libeon_core_la-eon_grammar.h

noinst_LTLIBRARIES = libeon_core.la

libeon_core_la_SOURCES = \
eon_grammar.y \
eon_lexer.l \
eon_animation.c \
eon_anim_key.c \
eon_anim_basic.c \
eon_anim_matrix.c \
eon_document.c \
eon_cache.c \
eon_clock.c \
eon_coord.c \
eon_engine.c \
eon_input.c \
eon_font.c \
eon_layout.c \
eon_main.c \
eon_object.c \
eon_paint.c \
eon_parser.c \
eon_setter.c \
eon_script.c \
eon_style.c \
eon_style_applier.c \
eon_trigger.c \
eon_value.c

libeon_core_la_YFLAGS = -d
libeon_core_la_LIBADD = @LEXLIB@
