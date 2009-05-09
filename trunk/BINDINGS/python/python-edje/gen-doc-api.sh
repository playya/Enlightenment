#!/bin/bash

pyver=`python -c 'import sys; print "%d.%d" % sys.version_info[0:2]'`
regex=lib\.[a-z0-9]*-[a-z0-9]*-$pyver
destdir="doc/api/html"

for d in `ls build`; do
    if [[ $d == $regex ]]; then
        break
    fi
done

export PYTHONPATH="`pwd`/build/$d"

if [ -d $destdir ]; then
    mkdir -p $destdir
fi

epydoc --html -o $destdir \
    -n "Python-Edje API" \
    edje.c_edje \
    edje.decorators
