#!/bin/sh

rm -rf autom4te.cache
rm -f aclocal.m4 ltmain.sh

./bootstrap && ./configure $@
