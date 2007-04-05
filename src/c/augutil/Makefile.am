include $(top_srcdir)/src/c/lang.mk
SUBDIRS = \
	posix \
	win32

modincludedir = $(includedir)/augutil
modinclude_HEADERS = \
	config.h \
	conv.h \
	event.h \
	file.h \
	getopt.h \
	lexer.h \
	list.h \
	log.h \
	md5.h \
	path.h \
	pwd.h \
	strbuf.h \
	timer.h \
	tmspec.h \
	var.h

noinst_HEADERS =

lib_LTLIBRARIES = libaugutil.la

libaugutil_la_SOURCES = \
	config.c \
	conv.c \
	event.c \
	file.c \
	getopt.c \
	lexer.c \
	list.c \
	log.c \
	md5.c \
	path.c \
	pwd.c \
	strbuf.c \
	timer.c \
	tmspec.c \
	var.c

libaugutil_la_LDFLAGS = \
	-no-undefined \
	-version-info 0:0:0

libaugutil_la_LIBADD = \
	$(top_builddir)/src/c/augsys/libaugsys.la

check_PROGRAMS = augutil-test
TESTS = augutil-test

augutil_test_SOURCES = test.c
augutil_test_LDADD = \
	$(top_builddir)/src/c/augutil/libaugutil.la
