from augas import *
import log

class Client:
    def __del__(self):
        log.info("destroying State object")

    def __init__(self, sock):
        self.sock = sock
        self.timer = settimer(sock.sname, 10, self)
        self.n = 10

    def cancel(self):
        self.sock = None
        if self.timer != None:
            canceltimer(self.timer)

    def done(self):
        self.n = self.n - 1
        return self.n < 0

    def expire(self):
        if not self.done():
            send(self.sock, "hello, world!\n")
        else:
            log.info("done: shutting client connection")
            shutdown(self.sock)
            self.timer = None
            return 0

def init(sname):
    log.info("connecting to client's service")
    for x in xrange(1, 10):
        tcpconnect(sname, "localhost", getenv("session.modclient.to"), None)

def closed(sock):
    if sock.user != None:
        sock.user.cancel()

def connected(sock, addr, port):
    log.info("client established, starting timer")
    sock.user = Client(sock)

def data(sock, buf):
    log.info("received by client: %s" % buf)

def expire(timer, ms):
    return timer.user.expire()
