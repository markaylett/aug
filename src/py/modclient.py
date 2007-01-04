from augas import *
import log

def opensess(sname):
    cid = tcpconnect(sname, "localhost", "7001")
    settimer(sname, 0, 1000, cid)

def expire(sname, tid, user, ms):
    send(sname, user, 'ping', SNDSELF)

def data(sname, cid, user, buf):
    log.debug("data: %s" % buf)
