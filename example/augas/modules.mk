ifndef AUG_HOME
$(error please define AUG_HOME)
endif

CFLAGS = -I$(AUG_HOME)/include
CXXFLAGS = $(CFLAGS)

CMODULES = modminimal
CXXMODULES = modserver modclient

modminimal_OBJS = minimal.o
modserver_OBJS = server.o
modclient_OBJS = client.o

include $(AUG_HOME)/etc/aug.mk
