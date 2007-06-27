module RbEcho
    def RbEcho.start(sname)
        AugRt.tcplisten("0.0.0.0", AugRt.getenv("session.RbEcho.serv"), nil)
    end
    def RbEcho.data(sock, buf)
        AugRt.send(sock, buf)
    end
end
