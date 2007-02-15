from augas import *
import log

def create(sname):
    settimer(sname, 5000, None)

def expire(timer, ms):
    stop()
