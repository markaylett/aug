from augas import *
import log

def opensess(sname):
    log.info("connecting to client's service")
    cid = tcpconnect(sname, "localhost", getenv("session.modclient.to"))
    settimer(sname, 0, 5000, cid)

def expire(sname, tid, user, ms):
    send(sname, user, 'ping', SNDSELF)

def notconn(sname, cid, user):
    log.info('failed to open client connection')

def data(sname, cid, user, buf):
    log.debug("received: %s" % buf)
