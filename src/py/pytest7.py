from augas import *
import log

def stop(sname):
    log.debug("stop(): %s" % sname)

def start(sname):
    log.debug("start(): %s" % sname)
    stopall()
