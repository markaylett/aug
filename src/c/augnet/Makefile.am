include $(top_srcdir)/src/c/lang.mk

modincludedir = $(includedir)/augnet
modinclude_HEADERS = \
	config.h \
	connector.h \
	file.h \
	fix.h \
	http.h \
	inet.h \
	types.h \
	writer.h

noinst_HEADERS =

lib_LTLIBRARIES = libaugnet.la

libaugnet_la_SOURCES = \
	config.c \
	connector.c \
	file.c \
	fix.c \
	http.c \
	inet.c \
	types.c \
	writer.c

libaugnet_la_LDFLAGS = \
	-no-undefined \
	-version-info 0:0:0

libaugnet_la_LIBADD = \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augmar/libaugmar.la \
	$(top_builddir)/src/c/augsys/libaugsys.la

check_PROGRAMS = augnet-fixtest augnet-httptest
#TESTS = augnet-fixtest augnet-httptest
TESTS = augnet-httptest

augnet_fixtest_SOURCES = fixtest.c
augnet_fixtest_LDADD = \
	$(top_builddir)/src/c/augnet/libaugnet.la

augnet_httptest_SOURCES = httptest.c
augnet_httptest_LDADD = \
	$(top_builddir)/src/c/augnet/libaugnet.la
