#!/bin/sh
./networdstest <networdstest.in >tmp.$$
cmp -s tmp.$$ networdstest.out
ret=$?
rm -f tmp.$$
exit $ret
