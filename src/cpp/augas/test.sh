#!/bin/sh

case `uname | tr [A-Z] [a-z]` in
    *cygwin* | *mingw* | *win32*)
        modext="dll"
        ;;
    *)
        modext="so"
        ;;
esac

for f in etc/*.conf.in; do
    bn=`basename $f | cut -d. -f1-2`
    g="etc/$bn"
    sed -e "s/[$]modext/$modext/g" -e "s/[$][{]modext[}]/$modext/g" <$f >$g
    cmd="./augasd -f $g test"
    echo "running [$cmd]..."
    err=`$cmd 2>&1 >/dev/null | wc -l`
    if [ $err != "0" ]; then
        exit 1
    fi
done
