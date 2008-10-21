module RbStop
    def self.start(sname)
        AugRb.settimer(5000, nil)
    end
    def self.expire(timer, ms)
        AugRb.stopall
    end
end
