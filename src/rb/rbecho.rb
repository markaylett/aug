module RbEcho
    def RbEcho.start(sname)
        AugRb.tcplisten("0.0.0.0", AugRb.getenv("session.RbEcho.serv"), nil)
    end
    def RbEcho.data(sock, buf)
        AugRb.send(sock, buf)
    end
end
