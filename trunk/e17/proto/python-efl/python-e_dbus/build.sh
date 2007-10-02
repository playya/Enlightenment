#!/bin/sh

PREFIX="$HOME/usr"
C_FILES="module.c"

for arg in $@; do
    case $arg in
        --force)
            touch $C_FILES
            ;;
        *)
            PREFIX="$arg"
            ;;
    esac
done

PYINCDIR=`python -c "import distutils.sysconfig; print distutils.sysconfig.get_python_inc()"`
INCDIR="$(dirname $PREFIX)/$PYINCDIR"

python setup.py build_ext --include-dirs="$INCDIR" develop --prefix="$PREFIX" install_headers --install-dir="$INCDIR"
