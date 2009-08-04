## Process this file with automake to produce Makefile.in
MAINTAINERCLEANFILES = Makefile.in

INCLUDES = \
-Wall -g \
-I$(top_srcdir)/lib \
@ELEMENTARY_CFLAGS@

bin_PROGRAMS = edje_editor

edje_editor_SOURCES = \
edje_editor_window.c \
edje_editor_window.h \
edje_editor_tree.c \
edje_editor_tree.h \
edje_editor_canvas.c \
edje_editor_canvas.h \
edje_editor_toolbar.c \
edje_editor_toolbar.h \
edje_editor_dialogs.c \
edje_editor_dialogs.h \
edje_editor_consolle.c \
edje_editor_consolle.h \
edje_editor_group.c \
edje_editor_group.h \
edje_editor_part.c \
edje_editor_part.h \
edje_editor_position.c \
edje_editor_position.h \
main.c \
main.h


edje_editor_LDADD = @ELEMENTARY_LIBS@
