include $(top_srcdir)/src/c/lang.mk

modincludedir = $(includedir)/augnet
modinclude_HEADERS = \
	auth.h \
	base64.h \
	chan.h \
	chans.h \
	config.h \
	fix.h \
	http.h \
	inet.h \
	mar.h \
	packet.h \
	ssl.h \
	tcpconnect.h \
	writer.h

noinst_HEADERS =

lib_LTLIBRARIES = libaugnet.la

libaugnet_la_SOURCES = \
	auth.c \
	base64.c \
	chan.c \
	chans.c \
	config.c \
	fix.c \
	http.c \
	inet.c \
	mar.c \
	packet.c \
	ssl.c \
	tcpconnect.c \
	writer.c

libaugnet_la_LDFLAGS = \
	-no-undefined \
	-version-info 1:0:0

libaugnet_la_LIBADD = \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augmar/libaugmar.la \
	$(top_builddir)/src/c/augsys/libaugsys.la \
	$(top_builddir)/src/c/augctx/libaugctx.la

check_PROGRAMS = \
	chanstest \
	base64test \
	chanstest \
	httptest \
	packettest
TESTS = \
	chanstest \
	base64test \
	chanstest \
	httptest \
	packettest

base64test_SOURCES = base64test.c
base64test_LDADD = \
	$(top_builddir)/src/c/augnet/libaugnet.la

chanstest_SOURCES = chanstest.c
chanstest_LDADD = \
	$(top_builddir)/src/c/augnet/libaugnet.la

# TODO: message counts is sometimes 2?
# fixtest_SOURCES = fixtest.c
# fixtest_LDADD = \
# 	$(top_builddir)/src/c/augnet/libaugnet.la \
# 	$(top_builddir)/src/c/augutil/libaugutil.la

httptest_SOURCES = httptest.c
httptest_LDADD = \
	$(top_builddir)/src/c/augnet/libaugnet.la

packettest_SOURCES = packettest.c
packettest_LDADD = \
	$(top_builddir)/src/c/augnet/libaugnet.la
