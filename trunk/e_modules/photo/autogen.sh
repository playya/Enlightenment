#!/bin/sh

rm -Rf  aclocal.m4 autom4te.cache/ ltmain.sh config.log config.status config.cache configure configure.scan libtool Makefile \
	data/Makefile.in data/Makefile \
	data/images/Makefile.in data/images/Makefile \
	data/themes/Makefile.in data/themes/Makefile \
	data/themes/minimal/Makefile.in data/themes/minimal/Makefile \
	src/Makefile src/Makefile.in \
	src/module/Makefile src/module/Makefile.in \
	po/Makefile

echo "Running autopoint..." ; autopoint -f || :
echo "Running aclocal..." ; aclocal $ACLOCAL_FLAGS -I m4 || exit 1
echo "Running autoheader..." ; autoheader || exit 1
echo "Running autoconf..." ; autoconf || exit 1
echo "Running libtoolize..." ; (libtoolize --copy --automake || glibtoolize --automake) || exit 1
echo "Running automake..." ; automake --add-missing --copy --gnu || exit 1
echo "Generating gettext photo.pot template"; \
xgettext \
--output photo.pot \
--output-dir=po \
--language=C \
--add-location \
--keyword=D_ \
--sort-by-file \
--copyright-holder="TODO" \
--foreign-user \
`find . -name "*.[ch]" -print` || exit 1

if [ -z "$NOCONFIGURE" ]; then
	./configure "$@"
fi
