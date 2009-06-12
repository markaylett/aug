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
from augutil import *
import log

# tcplisten(), tcpconnect()

(client, listener, server) = (None, None, None)

def stop():
    log.debug("stop()")

def start(sname):
    global client, listener
    log.debug("start(): %s" % sname)
    listener = tcplisten("0.0.0.0", "1234", None, None)
    client = tcpconnect("127.0.0.1", "1234", None, None)

def closed(sock):
    log.debug("closed(): %s" % sock)

def accepted(sock, name):
    global server
    log.debug("accepted(): %s" % sock)
    sock.ob = LineParser()
    server = sock

def connected(sock, name):
    global server
    log.debug("connected(): %s" % sock)
    sock.ob = LineParser()
    send(server, "hello, world!\n")

def recv(sock, buf):
    log.debug("recv(): %s" % sock)
    for line in sock.ob.parse(str(buf)):
        if line != "hello, world!":
            log.error("unexpected line from recv()")
        if sock == server:
            send(sock, line + "\n")
        else:
            stopall()
