SUBDIRS = groups images img macros parts programs

THEME = winter
EDC = $(THEME).edc
EET = $(EDC:%.edc=%.eet)
SUFFIXES = .edc .eet

CLEANFILES = $(EET)
EXTRA_DIST = $(EDC)

themedir = $(pkgdatadir)/themes
theme_DATA = $(EET)

all: $(EDC)

.edc.eet:
	edje_cc -v -id img -fd ../../fonts $< $@

preview: $(EET)
	edje $(EET) eplayer

