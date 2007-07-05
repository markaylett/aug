require 'log'

module RbTest7
    def RbTest7.stop
        Log.debug("stop()")
    end
    def RbTest7.start(sname)
        Log.debug("start(): #{sname}")
        AugRt.stopall
    end
end
