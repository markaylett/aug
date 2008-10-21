require 'log'

# settimer()

module RbTest4
    def self.stop
        Log.debug("stop()")
    end
    def self.start(sname)
        Log.debug("start(): #{sname}")
        AugRb.settimer(1000, "our timer")
    end
    def self.expire(timer, ms)
        Log.debug("expire(): #{timer}")
        if timer.user != "our timer"
            Log.error("unexpected user in expire()")
        end
        AugRb.stopall
    end
end
