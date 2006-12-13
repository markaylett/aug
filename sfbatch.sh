#!/bin/sh
SRCDIR=$HOME/src/aug
SERVERS="alpha-linux1 ppc-osx3"

(cd $SRCDIR && svn update)
for s in $SERVERS; do
    ssh $s 'cd $SRCDIR && ./configure --prefix=$HOME && make clean check'
done
