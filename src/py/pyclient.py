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

class State:
    def __del__(self):
        log.info("destroying State object")

    def __init__(self, sock):
        self.sock = sock
        self.timer = settimer(10, self)
        self.n = 10

    def cancel(self):
        self.sock = None
        if self.timer != None:
            canceltimer(self.timer)

    def more(self):
        self.n = self.n - 1
        return 0 <= self.n

    def expire(self):
        if self.more():
            send(self.sock, "hello, world!")
        else:
            log.info("done: shutting client connection")
            shutdown(self.sock, 0)
            self.timer = None
            return 0

def start(sname):
    log.info("connecting to client's service")
    for x in xrange(1, 10):
        tcpconnect("localhost", getenv("session.pyclient.to"), None, None)

def closed(sock):
    if sock.user != None:
        sock.user.cancel()

def connected(sock, name):
    log.info("client established, starting timer")
    sock.user = State(sock)

def recv(sock, buf):
    log.info("received by client: %s" % buf)

def expire(timer, ms):
    log.info("timer expired")
    return timer.user.expire()
