#!/bin/sh
./shelllexertest <shellwordstest.stdin | egrep -v '^[[:space:]]*$' | sed '/\r/d' >tmp.$$
cmp -s tmp.$$ shellwordstest.stdout
ret=$?
rm -f tmp.$$
exit $ret
