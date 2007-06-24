module RbEcho
    def RbEcho.start(sname)
        Augrt.tcplisten("0.0.0.0", Augrt.getenv("session.RbEcho.serv"), nil)
    end
    def RbEcho.data(sock, buf)
        Augrt.send(sock, buf)
    end
end
