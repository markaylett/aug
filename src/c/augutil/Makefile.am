include $(top_srcdir)/src/c/lang.mk

modincludedir = $(includedir)/augutil
modinclude_HEADERS = \
	config.h \
	conv.h \
	dstr.h \
	file.h \
	getopt.h \
	lexer.h \
	list.h \
	log.h \
	path.h \
	signal.h \
	timer.h

noinst_HEADERS =

lib_LTLIBRARIES = libaugutil.la

libaugutil_la_SOURCES = \
	config.c \
	conv.c \
	dstr.c \
	file.c \
	getopt.c \
	lexer.c \
	list.c \
	log.c \
	path.c \
	signal.c \
	timer.c

libaugutil_la_LDFLAGS = \
	-no-undefined \
	-version-info 0:0:0

libaugutil_la_LIBADD = \
	$(top_srcdir)/src/c/augsys/libaugsys.la

check_PROGRAMS = augutil-test
TESTS = $(check_PROGRAMS)

augutil_test_SOURCES = test.c
augutil_test_LDADD = \
	$(top_srcdir)/src/c/augutil/libaugutil.la \
	$(EXTRALIBS)
