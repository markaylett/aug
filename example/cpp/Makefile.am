include $(top_srcdir)/src/cpp/lang.mk

noinst_PROGRAMS = \
	mplexd \
	calc \
	tcpclient \
	timerd

if ENABLE_MULTICAST
noinst_PROGRAMS += \
	heartbeat \
	mcastrecv \
	mcastsend
endif

AUGLIBS= \
	$(top_builddir)/src/c/augnet/libaugnet.la \
	$(top_builddir)/src/c/augsrv/libaugsrv.la \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augmar/libaugmar.la \
	$(top_builddir)/src/c/augsys/libaugsys.la

mplexd_SOURCES = \
	mplexd.cpp
mplexd_LDADD = $(AUGLIBS)

calc_SOURCES = \
	calc.cpp

tcpclient_SOURCES = \
	tcpclient.cpp
tcpclient_LDADD = $(AUGLIBS)

timerd_SOURCES = \
	timerd.cpp
timerd_LDADD = $(AUGLIBS)

if ENABLE_MULTICAST
heartbeat_SOURCES = \
	heartbeat.cpp
heartbeat_LDADD = $(AUGLIBS)

mcastrecv_SOURCES = \
	mcastrecv.cpp
mcastrecv_LDADD = $(AUGLIBS)

mcastsend_SOURCES = \
	mcastsend.cpp
mcastsend_LDADD = $(AUGLIBS)
endif
