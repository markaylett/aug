from augas import *
import log

def opensess(sname):
    log.info("connecting to client's service")
    for x in xrange(1, 10):
        cid = tcpconnect(sname, "localhost", getenv("session.modclient.to"))
        settimer(sname, 0, x * 100, cid)

def expire(sname, tid, user, ms):
    send(sname, user, 'ping', SNDSELF)

def notconn(sname, cid, user):
    log.info('failed to open client connection')

def data(sname, cid, user, buf):
    log.info("received: %d %s" % (cid, buf))
