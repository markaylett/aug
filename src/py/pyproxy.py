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

from augpy import *
import log

pairs = {}

def start(sname):
    log.info("binding proxy listener")
    tcplisten("0.0.0.0", getenv("session.pyproxy.serv"), None, None)

def closed(sock):
    global pairs
    if pairs.has_key(sock):
        to = pairs[sock]
        log.info("closing proxy pair: %s -> %s" % (sock, to))
        del pairs[sock]
        del pairs[to]
        shutdown(to, 0)

def accepted(sock, name):
    global pairs
    log.info("opening proxy pair")
    # connected() will always be called after tcpconnect() returns.
    to = tcpconnect("localhost", getenv("session.pyproxy.to"), None, None)
    pairs[sock] = to
    pairs[to] = sock

def connected(sock, name):
    log.info("proxy pair established")

def recv(sock, buf):
    # It is safe to send to a connection that has not been fully established.
    if pairs.has_key(sock):
        send(pairs[sock], buf)
