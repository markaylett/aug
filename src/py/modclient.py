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

    def process(self, sname):
        if not self.done():
            send(sname, self.cid, "hello, world!", SNDSELF)
        else:
            log.info("done: shutting client connection")
            shutdown(self.cid)
            return 0

class Stop:
    def __del__(self):
        log.info("destroying Stop object")

    def __init__(self, sname):
        settimer(sname, 0, 5000, self)

    def process(self, sname):
        stop()
        return 0

stopper = None

def term(sname):
    global stopper
    stopper = None

def init(sname):
    global stopper
    stopper = Stop(sname)
    log.info("connecting to client's service")
    for x in xrange(1, 2):
        tcpconnect(sname, "localhost", getenv("session.modclient.to"), None)

def closed(sname, id, user):
    if user != None:
        user.cancel(sname)

def connected(sname, cid, user, addr, port):
    log.info("client established, starting timer")
    return State(sname, cid)

def expire(sname, tid, user, ms):
    return user.process(sname)

def data(sname, cid, user, buf):
    log.info("received by client: %s" % buf)
