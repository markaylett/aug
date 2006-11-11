import augas

def load():
    augas.writelog(augas.LOGINFO, "load()")

def unload():
    augas.writelog(augas.LOGINFO, "unload()")

def close(sid, ses):
    augas.writelog(augas.LOGINFO, "close()")

def open(sid, serv, peer):
    augas.writelog(augas.LOGINFO, "open(): %s, %s" % (serv, peer))
    # augas.settimer(0, 1000, "this is a test")

def data(sid, ses, buf):
    augas.writelog(augas.LOGINFO, "data(): %s" % buf)
    augas.send(sid, buf, augas.SESSELF)

def rdexpire(sid, ses, ms):
    augas.writelog(augas.LOGINFO, "rdexpire()")

def wrexpire(sid, ses, ms):
    augas.writelog(augas.LOGINFO, "wrexpire()")

def stop(sid, ses):
    augas.writelog(augas.LOGINFO, "stop()")

def event(type, arg):
    augas.writelog(augas.LOGINFO, "event(): %i, %s" % (type, arg))

def expire(arg, id, ms):
    augas.writelog(augas.LOGINFO, "expire(): %s" % arg)

def reconf():
    augas.writelog(augas.LOGINFO, "reconf()")
