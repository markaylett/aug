module Log
    def Log.crit(s)
        AugRb.writelog(AugRb::LOGCRIT, s)
    end
    def Log.error(s)
        AugRb.writelog(AugRb::LOGERROR, s)
    end
    def Log.warn(s)
        AugRb.writelog(AugRb::LOGWARN, s)
    end
    def Log.notice(s)
        AugRb.writelog(AugRb::LOGNOTICE, s)
    end
    def Log.info(s)
        AugRb.writelog(AugRb::LOGINFO, s)
    end
    def Log.debug(s)
        AugRb.writelog(AugRb::LOGDEBUG, s)
    end
end
