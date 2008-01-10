include $(top_srcdir)/src/c/lang.mk

modincludedir = $(includedir)/augnet
modinclude_HEADERS = \
	auth.h \
	base64.h \
	config.h \
	connector.h \
	extend.h \
	file.h \
	fix.h \
	heartbeat.h \
	http.h \
	inet.h \
	mar.h \
	nbfile.h \
	ssl.h \
	types.h \
	writer.h

noinst_HEADERS =

lib_LTLIBRARIES = libaugnet.la

libaugnet_la_SOURCES = \
	auth.c \
	base64.c \
	config.c \
	connector.c \
	extend.c \
	file.c \
	fix.c \
	heartbeat.c \
	http.c \
	inet.c \
	mar.c \
	nbfile.c \
	ssl.c \
	types.c \
	writer.c

libaugnet_la_LDFLAGS = \
	-no-undefined \
	-version-info 1:0:0

libaugnet_la_LIBADD = \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augmar/libaugmar.la \
	$(top_builddir)/src/c/augsys/libaugsys.la

check_PROGRAMS = \
	base64test \
	fixtest \
	httptest
TESTS = \
	base64test \
	httptest

base64test_SOURCES = base64test.c
base64test_LDADD = \
	$(top_builddir)/src/c/augnet/libaugnet.la

fixtest_SOURCES = fixtest.c
fixtest_LDADD = \
	$(top_builddir)/src/c/augnet/libaugnet.la

httptest_SOURCES = httptest.c
httptest_LDADD = \
	$(top_builddir)/src/c/augnet/libaugnet.la
