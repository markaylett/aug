#!/bin/sh
# diff -Naur ltmain.sh.orig ltmain.sh > local/ltmain.sh.patch

rm -fR autom4te.cache \
    && libtoolize --force --copy \
    && aclocal \
    && autoconf \
    && autoheader \
    && automake --add-missing --foreign --copy
