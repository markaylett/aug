#!/bin/sh
./networdstest <networdstest.in | sed '/\r/d' >tmp.$$
cmp -s tmp.$$ networdstest.out
ret=$?
rm -f tmp.$$
exit $ret
