#!/usr/bin/env python

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

#import sys
from augpy import *
from augutil import *
import log

# void writelog(level, msg)
# string error()
# void reconfall()
# void stopall()
# void post(to, type, user)
# void dispatch(to, type, user)
# string getenv(name, def)
# string getsession()
# void shutdown(sock, flags)
# int tcpconnect(host, serv, sslctx, user)
# int tcplisten(host, serv, sslctx, user)
# void send(sock, buffer buf)
# void setrwtimer(sock, ms, flags)
# void resetrwtimer(sock, ms, flags)
# void cancelrwtimer(sock, flags)
# int settimer(ms, user)
# bool resettimer(timer, ms)
# bool canceltimer(timer)

class Handler:
    def __init__(self):
        self.props = {}

    def do_exit(self):
        return Quit

    def do_quit(self):
        return Quit

    def do_get(self, x):
        return self.props[x]

    def do_ls(self):
        return self.props.iteritems()

    def do_set(self, x, y):
        self.props[x] = y

    def do_unset(self, x):
        del self.props[x]

interp = Interpreter(Handler())

# for line in sys.stdin:
#      x = interp.interpret(line)
#      if x == Quit:
#          sys.exit()
#      elif x != None:
#          print x

def stop():
    log.debug("stop()")

def start(sname):
    log.debug("start(): %s" % sname)
    tcplisten("0.0.0.0", getenv("session.pyskel.serv"), None, None)

def reconf():
    log.debug("reconf()")

def event(frm, type, user):
    log.debug("event()")

def closed(sock):
    log.info("closed(): %s" % sock)

def teardown(sock):
    log.debug("teardown(): %s" % sock)
    shutdown(sock, 0)

def accepted(sock, name):
    log.info("accepted(): %s" % sock)
    sock.user = LineParser()
    setrwtimer(sock, 15000, TIMRD)
    send(sock, "+OK hello\r\n")

def connected(sock, name):
    log.debug("connected(): %s" % sock)

def recv(sock, buf):
    log.debug("recv(): %s" % sock)
    global interp
    for line in sock.user.parse(str(buf)):
        x = interp.interpret(line)
        if x == Quit:
            send(sock, "+OK goodbye\r\n")
            shutdown(sock, 0)
        elif x != None:
            send(sock, x + "\r\n")

def error(sock, desc):
    log.debug("error(): %s" % sock)

def rdexpire(sock, ms):
    log.debug("rdexpire(): %s" % sock)
    send(sock, "+OK hello\r\n")

def wrexpire(sock, ms):
    log.debug("wrexpire(): %s" % sock)

def expire(timer, ms):
    log.debug("expire(): %s" % timer)
