#!/usr/bin/env ruby
require 'augutil'
require 'log'

include AugUtil

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
    def self.event(frm, type, user)
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
