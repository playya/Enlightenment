#!/bin/sh

rm -rf autom4te.cache
rm -f aclocal.m4 ltmain.sh

touch config.rpath
echo "Running autopoint..." ; autopoint -f || exit 1
touch config.rpath
echo "Running aclocal..." ; aclocal $ACLOCAL_FLAGS -I m4 || exit 1
touch config.rpath
echo "Running autoheader..." ; autoheader || exit 1
touch config.rpath
echo "Running autoconf..." ; autoconf || exit 1
touch config.rpath
echo "Running libtoolize..." ; (libtoolize --copy --automake || glibtoolize --automake) || exit 1
touch config.rpath
echo "Running automake..." ; automake --add-missing --copy --gnu || exit 1
touch config.rpath

echo "Generating gettext etk.pot template"; \
xgettext \
-n \
-C \
-d etk \
-p po \
--copyright-holder="Enlightenment development team" \
--foreign-user \
--msgid-bugs-address="enlightenment-devel@lists.sourceforge.net" \
-k -k_ -kd_ \
--from-code=UTF-8 \
-o etk.pot \
`find . -name "*.[ch]" -print`

if [ -z "$NOCONFIGURE" ]; then
	./configure "$@"
fi
