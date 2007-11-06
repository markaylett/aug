module RbStop
    def RbStop.start(sname)
        AugRb.settimer(5000, nil)
    end
    def RbStop.expire(timer, ms)
        AugRb.stopall
    end
end
