#!/bin/sh
# Run this to generate all the initial makefiles, etc.

abort () {
    echo "$1 not found or command failed. Aborting!"
    exit 1
}

srcdir=`dirname $0`
PKG_NAME="the package."

DIE=0

(autoconf --version) < /dev/null > /dev/null 2>&1 || {
  echo
  echo "**Error**: You must have \`autoconf' installed to."
  echo "Download the appropriate package for your distribution,"
  echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
  DIE=1
}

(grep "^AM_PROG_LIBTOOL" $srcdir/configure.in >/dev/null) && {
  (libtool --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "**Error**: You must have \`libtool' installed."
    echo "Get ftp://ftp.gnu.org/pub/gnu/libtool-1.2d.tar.gz"
    echo "(or a newer version if it is available)"
    DIE=1
  }
}

(automake --version) < /dev/null > /dev/null 2>&1 || {
  echo
  echo "**Error**: You must have \`automake' installed."
  echo "Get ftp://ftp.gnu.org/pub/gnu/automake-1.3.tar.gz"
  echo "(or a newer version if it is available)"
  DIE=1
  NO_AUTOMAKE=yes
}


# if no automake, don't bother testing for aclocal
test -n "$NO_AUTOMAKE" || (aclocal --version) < /dev/null > /dev/null 2>&1 || {
  echo
  echo "**Error**: Missing \`aclocal'.  The version of \`automake'"
  echo "installed doesn't appear recent enough."
  echo "Get ftp://ftp.gnu.org/pub/gnu/automake-1.3.tar.gz"
  echo "(or a newer version if it is available)"
  DIE=1
}

if test "$DIE" -eq 1; then
  exit 1
fi

if test -z "$*"; then
  echo "**Warning**: I am going to run \`configure' with no arguments."
  echo "If you wish to pass any to it, please specify them on the"
  echo \`$0\'" command line."
  echo
fi

case $CC in
xlc )
  am_opt=--include-deps;;
esac

for coin in `find $srcdir -name configure.in -print`
do 
  dr=`dirname $coin`
  if test -f $dr/NO-AUTO-GEN; then
    echo skipping $dr -- flagged as no auto-gen
  else
    echo processing $dr
    macrodirs=`sed -n -e 's,AM_ACLOCAL_INCLUDE(\(.*\)),\1,gp' < $coin`
    ( cd $dr
      aclocalinclude="$ACLOCAL_FLAGS"
      for k in $macrodirs; do
  	if test -d $k; then
          aclocalinclude="$aclocalinclude -I $k"
  	##else 
	##  echo "**Warning**: No such directory \`$k'.  Ignored."
        fi
      done
      if grep "^AM_PROG_LIBTOOL" configure.in >/dev/null; then
	echo "Running libtoolize..."
	libtoolize --force --copy || abort "libtoolize"
      fi
      echo "Running aclocal $aclocalinclude ..."
      aclocal $aclocalinclude || abort "aclocal"
      if grep "^AM_CONFIG_HEADER" configure.in >/dev/null; then
	echo "Running autoheader..."
	autoheader || abort "autoheader"
      fi
      echo "Running automake --gnu $am_opt ..."
      automake --add-missing --gnu $am_opt || abort "automake"
      echo "Running autoconf ..."
      autoconf || abort "autoconf"
    )
  fi
done

#conf_flags="--enable-maintainer-mode --enable-compile-warnings" #--enable-iso-c

if test x$NOCONFIGURE = x; then
  echo Running $srcdir/configure $conf_flags "$@" ...
  $srcdir/configure $conf_flags "$@" \
  && echo Now type \`make\' to compile $PKG_NAME || abort "configure"
else
  echo Skipping configure process.
fi


cd data/borders/
cd borderless
ln -sf selected-0.sticky-0.shaded-0.bits.db selected-1.sticky-0.shaded-0.bits.db
ln -sf selected-0.sticky-0.shaded-0.bits.db selected-0.sticky-1.shaded-0.bits.db
ln -sf selected-0.sticky-0.shaded-0.bits.db selected-0.sticky-0.shaded-1.bits.db
ln -sf selected-0.sticky-0.shaded-0.bits.db selected-1.sticky-1.shaded-0.bits.db
ln -sf selected-0.sticky-0.shaded-0.bits.db selected-1.sticky-0.shaded-1.bits.db
ln -sf selected-0.sticky-0.shaded-0.bits.db selected-1.sticky-1.shaded-1.bits.db
ln -sf selected-0.sticky-0.shaded-0.bits.db selected-0.sticky-1.shaded-1.bits.db
cd ..
cd default
#ln -sf selected-0.sticky-0.shaded-0.bits.db selected-1.sticky-0.shaded-0.bits.db
ln -sf selected-0.sticky-0.shaded-0.bits.db selected-0.sticky-1.shaded-0.bits.db
ln -sf selected-0.sticky-0.shaded-0.bits.db selected-0.sticky-0.shaded-1.bits.db
ln -sf selected-0.sticky-0.shaded-0.bits.db selected-1.sticky-1.shaded-0.bits.db
ln -sf selected-0.sticky-0.shaded-0.bits.db selected-1.sticky-0.shaded-1.bits.db
ln -sf selected-0.sticky-0.shaded-0.bits.db selected-1.sticky-1.shaded-1.bits.db
ln -sf selected-0.sticky-0.shaded-0.bits.db selected-0.sticky-1.shaded-1.bits.db
cd ../../fonts
ln -sf borzoib.ttf menu.ttf
ln -sf borzoib.ttf text.ttf
