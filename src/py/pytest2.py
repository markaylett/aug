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

# post(), dispatch()

def stop():
    log.debug("stop()")

def start(sname):
    log.debug("start(): %s" % sname)
    dispatch("topic1", "foo", 0, str(101))
    post(sname, "none", 0, None)

def event(frm, type, id, ob):
    log.debug("event(): %s" % ob)
    if type == "foo":
        if int(ob) != 101:
            log.error("unexpected ob data")
        dispatch(frm, "bar", 0, buffer("202"))
    elif type == "bar":
        if int(ob) != 202:
            log.error("unexpected ob data")
    elif type == "none":
        if ob is not None:
            log.error("unexpected ob data")
        stopall()
    else:
        log.error("unexpected type")
