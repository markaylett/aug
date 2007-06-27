module Log
    def Log.crit(s)
        AugRt.writelog(AugRt::LOGCRIT, s)
    end
    def Log.error(s)
        AugRt.writelog(AugRt::LOGERROR, s)
    end
    def Log.warn(s)
        AugRt.writelog(AugRt::LOGWARN, s)
    end
    def Log.notice(s)
        AugRt.writelog(AugRt::LOGNOTICE, s)
    end
    def Log.info(s)
        AugRt.writelog(AugRt::LOGINFO, s)
    end
    def Log.debug(s)
        AugRt.writelog(AugRt::LOGDEBUG, s)
    end
end
