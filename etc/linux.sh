#!/bin/sh

CFLAGS='-O2 -Wall -Werror'; export CFLAGS
CXXFLAGS="$CFLAGS -Wno-deprecated"; export CXXFLAGS

rm -f config.cache && sh ./configure \
    --enable-maintainer-mode \
    --prefix=$HOME
