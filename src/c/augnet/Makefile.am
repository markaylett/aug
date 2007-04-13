include $(top_srcdir)/src/c/lang.mk

modincludedir = $(includedir)/augnet
modinclude_HEADERS = \
	auth.h \
	base64.h \
	config.h \
	connector.h \
	file.h \
	fix.h \
	http.h \
	inet.h \
	mar.h \
	types.h \
	writer.h

noinst_HEADERS =

lib_LTLIBRARIES = libaugnet.la

libaugnet_la_SOURCES = \
	auth.c \
	base64.c \
	config.c \
	connector.c \
	file.c \
	fix.c \
	http.c \
	inet.c \
	mar.c \
	types.c \
	writer.c

libaugnet_la_LDFLAGS = \
	-no-undefined \
	-version-info 1:0:0

libaugnet_la_LIBADD = \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augmar/libaugmar.la \
	$(top_builddir)/src/c/augsys/libaugsys.la

check_PROGRAMS = augnet-base64test augnet-fixtest augnet-httptest
TESTS = augnet-base64test augnet-httptest

augnet_base64test_SOURCES = base64test.c
augnet_base64test_LDADD = \
	$(top_builddir)/src/c/augnet/libaugnet.la

augnet_fixtest_SOURCES = fixtest.c
augnet_fixtest_LDADD = \
	$(top_builddir)/src/c/augnet/libaugnet.la

augnet_httptest_SOURCES = httptest.c
augnet_httptest_LDADD = \
	$(top_builddir)/src/c/augnet/libaugnet.la
