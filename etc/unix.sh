#!/bin/sh

CC='cc'
CFLAGS='-O'; export CFLAGS

CXX='CC'
CXXFLAGS="$CFLAGS"; export CXXFLAGS

rm -f config.cache && sh ./configure \
    --prefix=$HOME
