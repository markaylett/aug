#!/usr/bin/env ruby

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

require 'augutil'
require 'log'

include AugUtil

# void writelog(level, msg)
# string geterror()
# void reconfall()
# void stopall()
# void post(to, type, id, user)
# void dispatch(to, type, id, user)
# string getenv(name, def)
# string getsession()
# void shutdown(sock, flags)
# unsigned tcpconnect(host, serv, sslctx, user)
# unsigned tcplisten(host, serv, sslctx, user)
# void send(sock, buffer buf)
# void setrwtimer(sock, ms, flags)
# void resetrwtimer(sock, ms, flags)
# void cancelrwtimer(sock, flags)
# unsigned settimer(ms, user)
# bool resettimer(timer, ms)
# bool canceltimer(timer)

class Handler
    def initialize
        @props = {}
    end
    def do_exit
        QUIT
    end
    def do_quit
        QUIT
    end
    def do_get(x)
        @props[x]
    end
    def do_ls
        ls = []
        @props.each_pair {|k, v| ls << "#{k} #{v}"}
        ls
    end
    def do_set(x, y)
        @props[x] = y
    end
    def do_unset(x)
        @props.delete(x)
    end
end

module RbSkel
    def self.stop
        Log.debug("stop()")
    end
    def self.start(sname)
        @interp = Interpreter.new(Handler.new)
        Log.debug("start(): #{sname}")
        AugRb.tcplisten("0.0.0.0", AugRb.getenv("session.RbSkel.serv"), nil, nil)
    end
    def self.reconf
        Log.debug("reconf()")
    end
    def self.event(frm, type, id, user)
        Log.debug("event()")
    end
    def self.closed(sock)
        Log.info("closed(): #{sock}")
    end
    def self.teardown(sock)
        Log.debug("teardown(): #{sock}")
        AugRb.shutdown(sock, 0)
    end
    def self.accepted(sock, name)
        Log.info("accepted(): #{sock}")
        sock.user = LineParser.new
        AugRb.setrwtimer(sock, 15000, AugRb::TIMRD)
        AugRb.send(sock, "+OK hello\r\n")
    end
    def self.connected(sock, name)
        Log.debug("connected(): #{sock}")
    end
    def self.recv(sock, buf)
        Log.debug("recv(): #{sock}")
        sock.user.parse(buf) do |line|
            x = @interp.interpret(line)
            if x == QUIT
                AugRb.send(sock, "+OK goodbye\r\n")
                AugRb.shutdown(sock, 0)
            elsif x != nil
                AugRb.send(sock, x + "\r\n")
            end
        end
    end
    def self.error(sock, ms)
        Log.debug("error(): #{sock}")
    end
    def self.rdexpire(sock, ms)
        Log.debug("rdexpire(): #{sock}")
        AugRb.send(sock, "+OK hello\r\n")
    end
    def self.wrexpire(sock, ms)
        Log.debug("wrexpire(): #{sock}")
    end
    def self.expire(timer, ms)
        Log.debug("expire(): #{sock}")
    end
end
