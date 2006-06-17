include $(top_srcdir)/src/c/lang.mk
SUBDIRS = \
	posix \
	win32

AM_CFLAGS = -D_MT
#AM_LDFLAGS = -Wl,-lws2_32

modincludedir = $(includedir)/augsrv
modinclude_HEADERS = \
	config.h \
	control.h \
	daemon.h \
	global.h \
	log.h \
	main.h \
	options.h \
	signal.h \
	types.h

lib_LTLIBRARIES = libaugsrv.la

libaugsrv_la_SOURCES = \
	config.c \
	control.c \
	daemon.c \
	global.c \
	log.c \
	main.c \
	options.c \
	signal.c \
	types.c

libaugsrv_la_LDFLAGS = \
	-no-undefined \
	-version-info 0:0:0

libaugsrv_la_LIBADD = \
	$(top_srcdir)/src/c/augutil/libaugutil.la \
	$(top_srcdir)/src/c/augsys/libaugsys.la

check_PROGRAMS = augsrv-test
TESTS = $(check_PROGRAMS)

augsrv_test_SOURCES = test.c
augsrv_test_LDADD = \
	$(top_srcdir)/src/c/augsrv/libaugsrv.la \
	$(EXTRA_LIBS)
