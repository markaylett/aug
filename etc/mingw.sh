#!/bin/sh

BOOST_ROOT='C:\\boost'; export BOOST_ROOT

CFLAGS='-O2 -Wall -Werror -pedantic'; export CFLAGS
CXXFLAGS="$CFLAGS -Wno-deprecated"; export CXXFLAGS

rm -f config.cache && sh ./configure \
    --enable-maintainer-mode \
    --prefix=$HOME
