from augas import *
import log

class State:
    def __del__(self):
        log.info("destroying State object")        
    
    def __init__(self, sname, cid):
        self.cid = cid
        self.tid = settimer(sname, 0, 100, self)
        self.n = 10

    def cancel(self, sname):
        canceltimer(sname, self.tid)

    def done(self):
        self.n = self.n - 1
        return self.n < 0

def opensess(sname):
    log.info("connecting to client's service")
    for x in xrange(1, 5):
        tcpconnect(sname, "localhost", getenv("session.modclient.to"), None)

def close(sname, id, user):
    user.cancel(sname)

def connect(sname, cid, user, addr, port):
    log.info("client established, starting timer")
    return State(sname, cid)

def expire(sname, tid, user, ms):
    if not user.done():
        send(sname, user.cid, "hello, world!", SNDSELF)
    else:
        log.info("done: shutting client connection")
        shutdown(user.cid)
        stop()
        return 0

def data(sname, cid, user, buf):
    log.info("received by client: %s" % buf)
