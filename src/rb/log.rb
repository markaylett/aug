module Log
    def Log.crit(s)
        Augrt.writelog(Augrt::LOGCRIT, s)
    end
    def Log.error(s)
        Augrt.writelog(Augrt::LOGERROR, s)
    end
    def Log.warn(s)
        Augrt.writelog(Augrt::LOGWARN, s)
    end
    def Log.notice(s)
        Augrt.writelog(Augrt::LOGNOTICE, s)
    end
    def Log.info(s)
        Augrt.writelog(Augrt::LOGINFO, s)
    end
    def Log.debug(s)
        Augrt.writelog(Augrt::LOGDEBUG, s)
    end
end
