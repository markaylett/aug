#!/bin/sh

AUG_HOME='C:/aug'; export AUG_HOME
BOOST_HOME='C:/boost'; export BOOST_HOME

CFLAGS='-O2 -Wall -Werror -pedantic'; export CFLAGS
CXXFLAGS="$CFLAGS -Wno-deprecated"; export CXXFLAGS

rm -f config.cache && sh ./configure \
    --enable-maintainer-mode \
    --prefix=$AUG_HOME
