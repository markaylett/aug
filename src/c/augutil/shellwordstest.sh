#!/bin/sh
./shellwordstest <shellwordstest.in | egrep -v '^[[:space:]]*$' >tmp.$$
cmp -s tmp.$$ shellwordstest.out
ret=$?
rm -f tmp.$$
exit $ret
