#! /bin/sh

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

THEDIR="`pwd`"
cd "$srcdir"
DIE=0

set -x
autoheader
libtoolize --ltdl --force --copy
aclocal
automake --foreign --add-missing
autoconf

set +x

if test -z "$*"; then
        echo "I am going to run ./configure with no arguments - if you wish "
        echo "to pass any to it, please specify them on the $0 command line."
        echo ""
fi

cd "$THEDIR"

$srcdir/configure "$@"

echo "Now type:"
echo
echo "make"
echo "make install"
echo
echo "have fun."

