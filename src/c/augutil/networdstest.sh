#!/bin/sh
./networdstest <networdstest.stdin | sed '/\r/d' >tmp.$$
cmp -s tmp.$$ networdstest.stdout
ret=$?
rm -f tmp.$$
exit $ret
