module RbEcho
    def self.start(sname)
        AugRb.tcplisten("0.0.0.0", AugRb.getenv("session.RbEcho.serv"), nil, nil)
    end
    def self.data(sock, buf)
        AugRb.send(sock, buf)
    end
end
