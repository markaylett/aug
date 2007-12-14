#!/bin/sh
./shellwordstest <shellwordstest.in | egrep -v '^[[:space:]]*$' | sed '/\r/d' >tmp.$$
cmp -s tmp.$$ shellwordstest.out
ret=$?
rm -f tmp.$$
exit $ret
