#!/bin/sh

BOOST_ROOT='C:\\boost'; export BOOST_ROOT

CC='gcc -mno-cygwin'; export CC
CFLAGS='-O2 -Wall -Werror'; export CFLAGS

CXX='g++ -mno-cygwin'; export CXX
CXXFLAGS="$CFLAGS -Wno-deprecated"; export CXXFLAGS

rm -f config.cache && sh ./configure \
    --enable-maintainer-mode \
    --build=mingw32 \
    --prefix=$HOME
