require 'log'

# Return false from accepted().

module RbTest6
    def RbTest6.stop
        Log.debug("stop()")
    end
    def RbTest6.start(sname)
        Log.debug("start(): #{sname}")
        AugRb.tcplisten("0.0.0.0", "1234", nil)
        AugRb.tcpconnect("127.0.0.1", "1234", nil)
    end
    def RbTest6.closed(sock)
        Log.debug("closed(): #{sock}")
    end
    def RbTest6.accepted(sock, addr, port)
        Log.debug("accepted(): #{sock}")
        false
    end
    def RbTest6.connected(sock, addr, port)
        Log.debug("connected(): #{sock}")
    end
end
