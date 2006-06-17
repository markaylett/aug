#!/bin/sh

CC='cc'
CXX='CC'
CFLAGS='-O'; export CFLAGS
CXXFLAGS="$CFLAGS"; export CXXFLAGS

rm -f config.cache && sh ./configure \
    --prefix=$HOME
