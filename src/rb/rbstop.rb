module RbStop
    def RbStop.start(sname)
        AugRt.settimer(5000, nil)
    end
    def RbStop.expire(timer, ms)
        AugRt.stopall
    end
end
