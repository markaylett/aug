include $(top_srcdir)/src/c/lang.mk

modincludedir = $(includedir)/augnet
modinclude_HEADERS = \
	config.h \
	conn.h \
	inet.h \
	parser.h \
	types.h \
	utility.h

noinst_HEADERS =

lib_LTLIBRARIES = libaugnet.la

libaugnet_la_SOURCES = \
	config.c \
	conn.c \
	inet.c \
	parser.c \
	types.c \
	utility.c

libaugnet_la_LDFLAGS = \
	-no-undefined \
	-version-info 0:0:0

libaugnet_la_LIBADD = \
	$(top_srcdir)/src/c/augutil/libaugutil.la \
	$(top_srcdir)/src/c/augmar/libaugmar.la \
	$(top_srcdir)/src/c/augsys/libaugsys.la

check_PROGRAMS = augnet-test
TESTS = $(check_PROGRAMS)

augnet_test_SOURCES = test.c
augnet_test_LDADD = \
	$(top_srcdir)/src/c/augnet/libaugnet.la \
	$(WINSOCK) \
	-lm
