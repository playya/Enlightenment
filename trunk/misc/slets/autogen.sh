#!/bin/sh

DIE=0

(autoconf --version) < /dev/null > /dev/null 2>&1 || {
        echo
        echo "You must have autoconf installed to compile Epplets."
        echo "Download the appropriate package for your distribution,"
        echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
        DIE=1
}

(automake --version) < /dev/null > /dev/null 2>&1 || {
        echo
        echo "You must have automake installed to compile Epplets."
        echo "Download the appropriate package for your distribution,"
        echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
        DIE=1
}
        
if test "$DIE" -eq 1; then
        exit 1
fi

set -x

aclocal -I . $ACLOCAL_FLAGS
autoheader
automake --add-missing
autoconf

set +x

if [ -z "$NOCONFIGURE" ]; then

if [ -x config.status -a -z "$*" ]; then
  ./config.status --recheck
else
  if test -z "$*"; then
    echo "I am going to run ./configure with no arguments - if you wish "
    echo "to pass any to it, please specify them on the $0 command line."
    echo "If you do not wish to run ./configure, press Ctrl-C now."
    trap 'echo "configure aborted" ; exit 0' 1 2 15
    sleep 1
  fi
  ./configure "$@"
fi

echo
echo "Now type:"
echo
echo "make"
echo "make install"
echo
echo "have fun."

fi
