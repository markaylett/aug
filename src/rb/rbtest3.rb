require 'log'

# getenv()

module RbTest3
    def RbTest3.stop
        Log.debug("stop()")
    end
    def RbTest3.start(sname)
        Log.debug("start() #{sname}")
        if AugRb.getenv("RbTest3") != "test value"
            Log.error("unexpected value from getenv()")
        end
        if AugRb.getenv("badname") != nil
            Log.error("unexpected value from getenv()")
        end
        if AugRb.getenv("badname", 101) != 101
            Log.error("unexpected value from getenv()")
        end
        AugRb.stopall
    end
end
