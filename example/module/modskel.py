import augas

def closesess(sname):
    augas.writelog(augas.LOGINFO, "closesess()")

def opensess(sname):
    augas.writelog(augas.LOGINFO, "opensess()")
    augas.settimer(sname, 0, 1000, "this is a test")

def event(sname, type, user):
    augas.writelog(augas.LOGINFO, "event()")

def expire(sname, tid, user, ms):
    augas.writelog(augas.LOGINFO, "expire()")

def reconf(sname):
    augas.writelog(augas.LOGINFO, "reconf()")

def closeconn(sname, cid, user):
    augas.writelog(augas.LOGINFO, "closeconn()")

def openconn(sname, cid, addr, port):
    augas.writelog(augas.LOGINFO, "openconn()")

def notconn(sname, cid, user):
    augas.writelog(augas.LOGINFO, "notconn()")

def data(sname, cid, user, buf):
    augas.writelog(augas.LOGINFO, "data()")
    # augas.send(cid, buf, augas.SESSELF)

def rdexpire(sname, cid, user, ms):
    augas.writelog(augas.LOGINFO, "rdexpire()")

def wrexpire(sname, cid, user, ms):
    augas.writelog(augas.LOGINFO, "wrexpire()")

def teardown(sname, cid, user):
    augas.writelog(augas.LOGINFO, "teardown()")
