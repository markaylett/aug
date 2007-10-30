#!/bin/sh
./networdstest <networdstest.in | egrep -v '^[[:space:]]*$' >tmp.$$
cmp -s tmp.$$ networdstest.out
ret=$?
rm -f tmp.$$
exit $ret
