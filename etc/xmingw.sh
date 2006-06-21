#!/bin/sh

BASEFLAGS='-O2 -Wall -Werror -pedantic'; export BASEFLAGS
CFLAGS="-std=c99 $BASEFLAGS"; export CFLAGS
CXXFLAGS="-std=c++98 $BASEFLAGS -Wno-deprecated"; export CXXFLAGS

rm -f config.cache && sh ./configure \
    --enable-maintainer-mode \
    --build=i586-pc-linux-gnu \
    --host=i586-mingw32msvc \
    --prefix=$AUG_HOME/i586-mingw32msvc
