## Process this file with automake to produce Makefile.in

MYAPPNAME=emphasis
SUBDIRS = src data

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess \
                       config.h.in config.h.in~ config.sub configure \
							  install-sh \
                       ltconfig ltmain.sh missing mkinstalldirs depcomp \
                       stamp-h stamp-h.in $(MYAPPNAME)_docs.tar \
                       $(MYAPPNAME)_docs.tar.gz $(MYAPPNAME).c

dist-hook:
	./gendoc; \
	tar cvf $(MYAPPNAME)_docs.tar doc; \
	rm -f $(MYAPPNAME)_docs.tar.gz; \
	gzip -9 $(MYAPPNAME)_docs.tar;
							  

EXTRA_DIST = README \
AUTHORS \
COPYING \
COPYING-PLAIN \
$(MYAPPNAME).c.in \
gendoc \
Doxyfile \
$(MYAPPNAME)_docs.tar.gz
