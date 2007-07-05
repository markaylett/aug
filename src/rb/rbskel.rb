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
# void shutdown(sock)
# int tcpconnect(host, serv, user)
# int tcplisten(host, serv, user)
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
    def RbSkel.stop
        Log.debug("stop()")
    end
    def RbSkel.start(sname)
        @interp = Interpreter.new(Handler.new)
        Log.debug("start(): #{sname}")
        AugRt.tcplisten("0.0.0.0", AugRt.getenv("session.RbSkel.serv"), nil)
    end
    def RbSkel.reconf
        Log.debug("reconf()")
    end
    def RbSkel.event(frm, type, user)
        Log.debug("event()")
    end
    def RbSkel.closed(sock)
        Log.info("closed(): #{sock}")
    end
    def RbSkel.teardown(sock)
        Log.debug("teardown(): #{sock}")
        AugRt.shutdown(sock)
    end
    def RbSkel.accepted(sock, addr, port)
        Log.info("accepted(): #{sock}")
        sock.user = LineParser.new
        AugRt.setrwtimer(sock, 15000, AugRt::TIMRD)
        AugRt.send(sock, "+OK hello\r\n")
    end
    def RbSkel.connected(sock, addr, port)
        Log.debug("connected(): #{sock}")
    end
    def RbSkel.data(sock, buf)
        Log.debug("data(): #{sock}")
        sock.user.parse(buf) do |line|
            x = @interp.interpret(line)
            if x == QUIT
                AugRt.send(sock, "+OK goodbye\r\n")
                AugRt.shutdown(sock)
            elsif x != nil
                AugRt.send(sock, x + "\r\n")
            end
        end
    end
    def RbSkel.rdexpire(sock, ms)
        Log.debug("rdexpire(): #{sock}")
        AugRt.send(sock, "+OK hello\r\n")
    end
    def RbSkel.wrexpire(sock, ms)
        Log.debug("wrexpire(): #{sock}")
    end
    def RbSkel.expire(timer, ms)
        Log.debug("expire(): #{sock}")
    end
end
