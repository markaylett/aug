#!/bin/sh

BASEFLAGS='-O2 -Wall -Werror -pedantic'; export BASEFLAGS
CFLAGS="-std=c99 $BASEFLAGS"; export CFLAGS
CXXFLAGS="-std=c++98 $BASEFLAGS -Wno-deprecated"; export CXXFLAGS

rm -f config.cache && sh ./configure \
    --enable-maintainer-mode \
    --prefix=$AUG_HOME
