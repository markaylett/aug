require 'log'

module RbTest7
    def self.stop
        Log.debug("stop()")
    end
    def self.start(sname)
        Log.debug("start(): #{sname}")
        AugRb.stopall
    end
end
