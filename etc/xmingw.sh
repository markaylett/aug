#!/bin/sh

PATH=/opt/xmingw/bin:$PATH; export PATH

CFLAGS='-O2 -Wall'; export CFLAGS
CXXFLAGS="$CFLAGS -Wno-deprecated"; export CXXFLAGS

rm -f config.cache && sh ./configure \
    --enable-maintainer-mode \
    --prefix=$HOME/i386-mingw32msvc \
    --host=i386-mingw32msvc \
    --build=i686-pc-linux-gnu
