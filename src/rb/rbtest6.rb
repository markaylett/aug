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
