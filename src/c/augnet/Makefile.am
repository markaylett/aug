include $(top_srcdir)/src/c/lang.mk

modincludedir = $(includedir)/augnet
modinclude_HEADERS = \
	auth.h \
	base64.h \
	chans.h \
	config.h \
	fix.h \
	http.h \
	inet.h \
	mar.h \
	netevent.h \
	object.h \
	ssl.h \
	tcpconnect.h \
	types.h \
	writer.h

noinst_HEADERS =

lib_LTLIBRARIES = libaugnet.la

libaugnet_la_SOURCES = \
	auth.c \
	base64.c \
	chans.c \
	config.c \
	fix.c \
	http.c \
	inet.c \
	mar.c \
	netevent.c \
	object.c \
	ssl.c \
	tcpconnect.c \
	types.c \
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
	chanstest
# 	base64test
# 	chanstest
# 	fixtest
# 	httptest
# 	neteventtest
TESTS = \
	chanstest
# 	base64test
# 	chanstest
# 	httptest
# 	neteventtest

# base64test_SOURCES = base64test.c
# base64test_LDADD = \
# 	$(top_builddir)/src/c/augnet/libaugnet.la

chanstest_SOURCES = chanstest.c
chanstest_LDADD = \
	$(top_builddir)/src/c/augnet/libaugnet.la

# fixtest_SOURCES = fixtest.c
# fixtest_LDADD = \
# 	$(top_builddir)/src/c/augnet/libaugnet.la

# httptest_SOURCES = httptest.c
# httptest_LDADD = \
# 	$(top_builddir)/src/c/augnet/libaugnet.la

# neteventtest_SOURCES = neteventtest.c
# neteventtest_LDADD = \
# 	$(top_builddir)/src/c/augnet/libaugnet.la
