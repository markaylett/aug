#!/bin/sh

CFLAGS='-O2 -Wall -pedantic'; export CFLAGS
CXXFLAGS="$CFLAGS -Wno-deprecated"; export CXXFLAGS

rm -f config.cache && sh ./configure \
    --enable-maintainer-mode \
    --build=i586-pc-linux-gnu \
    --host=i586-mingw32msvc \
    --prefix=$HOME/i586-mingw32msvc
