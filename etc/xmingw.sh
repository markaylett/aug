#!/bin/sh

CC='i586-mingw32msvc-gcc'; export CC
CFLAGS='-O2 -Wall -pedantic'; export CFLAGS

CXX='i586-mingw32msvc-g++'; export CXX
CXXFLAGS="$CFLAGS -Wno-deprecated"; export CXXFLAGS

rm -f config.cache && sh ./configure \
    --enable-maintainer-mode \
    --build=i586-pc-linux-gnu \
    --host=i586-pc-mingw32msvc \
    --target=i586-pc-mingw32msvc \
    --prefix=$HOME/i586-mingw32msvc
