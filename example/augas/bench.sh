#!/bin/sh

# Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>
#
# This file is part of Aug written by Mark Aylett.
#
# Aug is released under the GPL with the additional exemption that compiling,
# linking, and/or using OpenSSL is allowed.
#
# Aug is free software; you can redistribute it and/or modify it under the
# terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2 of the License, or (at your option) any later
# version.
#
# Aug is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 51
# Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

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
