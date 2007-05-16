#!/bin/sh

for f in etc/*.conf; do
    cmd="./daug -f $f test"
    echo "running [$cmd]..."
    err=`$cmd 2>&1 >/dev/null | wc -l`
    if [ $err != "0" ]; then
        exit 1
    fi
done
