from augrt import *

def crit(s):
    writelog(LOGCRIT, s)

def error(s):
    writelog(LOGERROR, s)

def warn(s):
    writelog(LOGWARN, s)

def notice(s):
    writelog(LOGNOTICE, s)

def info(s):
    writelog(LOGINFO, s)

def debug(s):
    writelog(LOGDEBUG, s)
