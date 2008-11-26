#!/bin/sh

set -e

HOST=${1:-'localhost'}

function bench() {
    cat <<EOF >$1.conf
rundir = .
logdir = .
loglevel = 4

sessions = bench

session.bench.module = client
session.bench.sendv = 0
session.bench.host = $HOST
session.bench.serv = 7000
session.bench.conns = $1
session.bench.echos = 5000

module.client.path = ./modclient
EOF

    $AUG_HOME/bin/daug -f $1.conf
    mv bench.dat $1.dat
    rm -f $1.conf
}

for n in 4 8 16 32 64 128; do
    bench $n
done
