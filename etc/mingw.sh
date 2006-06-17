#!/bin/sh

BOOST_ROOT='C:\\boost'; export BOOST_ROOT

CC='mingw32-gcc -mno-cygwin'; export CC
CFLAGS='-O2 -Wall -Werror -pedantic'; export CFLAGS

CXX='mingw32-g++ -mno-cygwin'; export CXX
CXXFLAGS="$CFLAGS -Wno-deprecated"; export CXXFLAGS

rm -f config.cache && sh ./configure \
    --enable-maintainer-mode \
    --host=i386-mingw32msvc \
    --build=mingw32 \
    --prefix=$HOME
