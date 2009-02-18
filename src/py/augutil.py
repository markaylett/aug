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

import re

Quit = object()

class LineParser:
    def __init__(self):
        self.tail = ""

    def parse(self, data):
        data = self.tail + data
        ls = data.split("\n")
        self.tail = ls.pop()
        for l in ls:
            yield l.rstrip("\r")

class Interpreter:
    def __init__(self, handler):
        self.handler = handler

    def interpret(self, line):
        line = line.strip()
        if len(line) == 0:
            return None
        toks = re.split(r"\s+", line)
        try:
            x = apply(getattr(self.handler, "do_" + toks[0].lower()), toks[1:])
            if x is not Quit:
                if x is None:
                    x = "+OK"
                elif hasattr(x, "__iter__"):
                    x = reduce(lambda y, z: y + str(z) + "\r\n", \
                               x, "+OK\r\n") + ".";
                else:
                    x = "+OK " + str(x)
        except AttributeError, e:
            x = "-ERR invalid command: %s" % e
        except TypeError, e:
            x = "-ERR invalid arguments: %s" % e
        except Exception, e:
            x = "-ERR exception: %s" % e
        return x
