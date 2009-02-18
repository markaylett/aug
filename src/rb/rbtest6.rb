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

require 'log'

# Return false from accepted().

module RbTest6
    def self.stop
        Log.debug("stop()")
    end
    def self.start(sname)
        Log.debug("start(): #{sname}")
        AugRb.tcplisten("0.0.0.0", "1234", nil, nil)
        AugRb.tcpconnect("127.0.0.1", "1234", nil, nil)
    end
    def self.closed(sock)
        Log.debug("closed(): #{sock}")
    end
    def self.accepted(sock, name)
        Log.debug("accepted(): #{sock}")
        false
    end
    def self.connected(sock, name)
        Log.debug("connected(): #{sock}")
    end
end
