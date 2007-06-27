require 'log'

# settimer()

module RbTest4
    def RbTest4.stop
        Log.debug("stop()")
    end
    def RbTest4.start(sname)
        Log.debug("start(): #{sname}")
        AugRt.settimer(1000, "our timer")
    end
    def RbTest4.expire(timer, ms)
        Log.debug("expire(): #{timer}")
        if timer.user != "our timer"
            Log.error("unexpected user in expire()")
        end
        AugRt.stopall
    end
end
