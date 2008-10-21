module Log
    def self.crit(s)
        AugRb.writelog(AugRb::LOGCRIT, s)
    end
    def self.error(s)
        AugRb.writelog(AugRb::LOGERROR, s)
    end
    def self.warn(s)
        AugRb.writelog(AugRb::LOGWARN, s)
    end
    def self.notice(s)
        AugRb.writelog(AugRb::LOGNOTICE, s)
    end
    def self.info(s)
        AugRb.writelog(AugRb::LOGINFO, s)
    end
    def self.debug(s)
        AugRb.writelog(AugRb::LOGDEBUG, s)
    end
end
