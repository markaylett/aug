#!/bin/sh

AUG_HOME='C:/aug'; export AUG_HOME
BOOST_HOME='C:/boost'; export BOOST_HOME

BASEFLAGS='-O2 -Wall -Werror -pedantic'

CC='gcc -mno-cygwin'; export CC
CFLAGS="-std=c99 $BASEFLAGS"; export CFLAGS

CXX='g++ -mno-cygwin'; export CXX
CXXFLAGS="-std=c++98 $BASEFLAGS -Wno-deprecated"; export CXXFLAGS

rm -f config.cache && sh ./configure \
    --enable-maintainer-mode \
    --prefix=$AUG_HOME
