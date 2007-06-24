module RbStop
    def RbStop.start(sname)
        Augrt.settimer(5000, nil)
    end
    def RbStop.expire(timer, ms)
        Augrt.stopall()
    end
end
