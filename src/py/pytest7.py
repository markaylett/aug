from augas import *
import log

def destroy(sname):
    log.debug("destroy(): %s" % sname)

def create(sname):
    log.debug("create(): %s" % sname)
    stop()
