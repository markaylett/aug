#!/bin/sh

CC='gcc'; export CC
CFLAGS='-O2 -Wall -Werror -pedantic'; export CFLAGS

CXX='g++'; export CXX
CXXFLAGS="$CFLAGS -Wno-deprecated"; export CXXFLAGS

rm -f config.cache && sh ./configure \
    --enable-maintainer-mode \
    --prefix=$HOME
