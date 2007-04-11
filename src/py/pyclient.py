from augas import *
import log

class State:
    def __del__(self):
        log.info("destroying State object")

    def __init__(self, sock):
        self.sock = sock
        self.timer = settimer(10, self)
        self.n = 10

    def cancel(self):
        self.sock = None
        if self.timer != None:
            canceltimer(self.timer)

    def more(self):
        self.n = self.n - 1
        return 0 <= self.n

    def expire(self):
        if self.more():
            send(self.sock, "hello, world!")
        else:
            log.info("done: shutting client connection")
            shutdown(self.sock)
            self.timer = None
            return 0

def start(sname):
    log.info("connecting to client's service")
    for x in xrange(1, 10):
        tcpconnect("localhost", getenv("service.pyclient.to"), None)

def reconf():
    pass

def closed(sock):
    if sock.user != None:
        sock.user.cancel()

def connected(sock, addr, port):
    log.info("client established, starting timer")
    sock.user = State(sock)

def data(sock, buf):
    log.info("received by client: %s" % buf)

def expire(timer, ms):
    log.info("timer expired")
    return timer.user.expire()
