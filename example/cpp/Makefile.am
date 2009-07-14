include $(top_srcdir)/src/cpp/lang.mk

noinst_PROGRAMS = \
	muxerd \
	profile \
	tcpclient \
	tcpserver \
	timerd

if ENABLE_MULTICAST
noinst_PROGRAMS += \
	mcastrecv \
	mcastsend \
	packet
endif

AUGLIBS= \
	$(top_builddir)/src/c/augnet/libaugnet.la \
	$(top_builddir)/src/c/augserv/libaugserv.la \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augmar/libaugmar.la \
	$(top_builddir)/src/c/augsys/libaugsys.la

muxerd_SOURCES = \
	muxerd.cpp
muxerd_LDADD = $(AUGLIBS)

profile_SOURCES = \
	profile.cpp
profile_LDADD = $(AUGLIBS)

tcpclient_SOURCES = \
	tcpclient.cpp
tcpclient_LDADD = $(AUGLIBS)

tcpserver_SOURCES = \
	tcpserver.cpp
tcpserver_LDADD = $(AUGLIBS)

timerd_SOURCES = \
	timerd.cpp
timerd_LDADD = $(AUGLIBS)

if ENABLE_MULTICAST
mcastrecv_SOURCES = \
	mcastrecv.cpp
mcastrecv_LDADD = $(AUGLIBS)

mcastsend_SOURCES = \
	mcastsend.cpp
mcastsend_LDADD = $(AUGLIBS)

packet_SOURCES = \
	packet.cpp
packet_LDADD = $(AUGLIBS)
endif
