include $(top_srcdir)/src/cpp/lang.mk

noinst_PROGRAMS = \
	heartbeat \
	mcastrecv \
	mcastsend \
	mplexd \
	timerd

AUGLIBS= \
	$(top_srcdir)/src/c/augnet/libaugnet.la \
	$(top_srcdir)/src/c/augsrv/libaugsrv.la \
	$(top_srcdir)/src/c/augutil/libaugutil.la \
	$(top_srcdir)/src/c/augmar/libaugmar.la \
	$(top_srcdir)/src/c/augsys/libaugsys.la

heartbeat_SOURCES = \
	heartbeat.cpp
heartbeat_LDADD = $(AUGLIBS)

mcastrecv_SOURCES = \
	mcastrecv.cpp
mcastrecv_LDADD = $(AUGLIBS)

mcastsend_SOURCES = \
	mcastsend.cpp
mcastsend_LDADD = $(AUGLIBS)

mplexd_SOURCES = \
	mplexd.cpp
mplexd_LDADD = $(AUGLIBS)

timerd_SOURCES = \
	timerd.cpp
timerd_LDADD = $(AUGLIBS)
