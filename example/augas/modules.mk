ifndef AUG_HOME
$(error please define AUG_HOME)
endif

CFLAGS = -I$(AUG_HOME)/include
CXXFLAGS = $(CFLAGS)
LDFLAGS = -L$(AUG_HOME)/lib

CMODULES = modminimal
CXXMODULES = modclient modhttp modserver

modclient_OBJS = client.o
modclient_LIBS = augsys

modhttp_OBJS = http.o
modhttp_LIBS = augnet augutil augmar augsys

modminimal_OBJS = minimal.o

modserver_OBJS = server.o

include $(AUG_HOME)/etc/aug.mk

bench: all
	$(AUG_HOME)/bin/augasd -f bench.conf test

http: all
	$(AUG_HOME)/bin/augasd -f http.conf test
