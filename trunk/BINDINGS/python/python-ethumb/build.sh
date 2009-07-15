#!/bin/sh

PREFIX="$HOME/usr"
PYX_FILES="ethumb/ethumb.c_ethumb.pyx ethumb/ethumb.client.pyx"

for arg in $@; do
    case $arg in
        --force)
            touch $PYX_FILES
            ;;
        *)
            PREFIX="$arg"
            ;;
    esac
done

PYINCDIR=`python -c "import distutils.sysconfig; print distutils.sysconfig.get_python_inc()" | cut -d/ -f3-`
INCDIR="$PREFIX/$PYINCDIR"

python setup.py build_ext --include-dirs="$INCDIR" develop --prefix="$PREFIX" install_headers --install-dir="$INCDIR"
