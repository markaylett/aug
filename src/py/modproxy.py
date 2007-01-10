from augas import *
import log

pairs = {}

def opensess(sname):
    log.info("binding proxy listener")
    tcplisten(sname, "0.0.0.0", getenv("session.modproxy.serv"), None)

def close(sname, id, user):
    global pairs
    if pairs.has_key(id):
        to = pairs[id]
        log.info("closing proxy pair: (%d, %d)" % (id, to))
        del pairs[id]
        del pairs[to]
        shutdown(to)

def openconn(sname, cid, user, addr, port):
    global pairs
    if user == None:
        log.info("opening proxy pair")
        to = tcpconnect(sname, "localhost", getenv("session.modproxy.to"), cid)
        pairs[cid] = to
        pairs[to] = cid

def data(sname, cid, user, buf):
    send(sname, pairs[cid], buf, SNDSELF)
