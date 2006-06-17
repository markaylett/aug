#!/bin/sh

BOOST_HOME='C:\\boost'; export BOOST_HOME

CC='gcc -mno-cygwin'; export CC
CFLAGS='-O2 -Wall -Werror -pedantic'; export CFLAGS

CXX='g++ -mno-cygwin'; export CXX
CXXFLAGS="$CFLAGS -Wno-deprecated"; export CXXFLAGS

rm -f config.cache && sh ./configure \
    --enable-maintainer-mode \
    --prefix=$HOME
