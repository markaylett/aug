from augas import *
import log

pairs = {}

def init(sname):
    log.info("binding proxy listener")
    tcplisten(sname, "0.0.0.0", getenv("session.modproxy.serv"), None)

def closed(sname, id, user):
    global pairs
    if pairs.has_key(id):
        to = pairs[id]
        log.info("closing proxy pair: (%d, %d)" % (id, to))
        del pairs[id]
        del pairs[to]
        shutdown(to)

def accept(sname, cid, user, addr, port):
    global pairs
    log.info("opening proxy pair")
    # connected() will always be called after tcpconnect() returns.
    to = tcpconnect(sname, "localhost", getenv("session.modproxy.to"), cid)
    pairs[cid] = to
    pairs[to] = cid

def connected(sname, cid, user, addr, port):
    log.info("proxy pair established")

def data(sname, cid, user, buf):
    # It is safe to send to a connection that has not been fully established.
    if pairs.has_key(cid):
        send(pairs[cid], buf)
