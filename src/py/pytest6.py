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

# Return false from accepted().

def stop():
    log.debug("stop()")

def start(sname):
    log.debug("start(): %s" % sname)
    tcplisten("0.0.0.0", "1234", None, None)
    tcpconnect("127.0.0.1", "1234", None, None)

def closed(sock):
    log.debug("closed(): %s" % sock)

def accepted(sock, name):
    log.debug("accepted(): %s" % sock)
    return False

def connected(sock, name):
    log.debug("connected(): %s" % sock)
