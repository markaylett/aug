from augrt import *
import log

def stop():
    log.debug("stop()")

def start(sname):
    log.debug("start(): %s" % sname)
    stopall()
