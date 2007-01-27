#!/bin/sh
for f in etc/*.conf; do
    cmd="./augasd -f $f test"
    echo "running $cmd"
    n=`$cmd 2>&1 >/dev/null | wc -l`
    if [ $n != "0" ]; then
        exit 1
    fi
done
