EXTRA_DIST = \
	networdstest.sh \
	networdstest.stdin \
	networdstest.stdout \
	shelllexertest.sh \
	shellwordstest.sh \
	shellwordstest.stdin \
	shellwordstest.stdout

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
	hires.h \
	lexer.h \
	list.h \
	log.h \
	md5.h \
	networds.h \
	object.h \
	path.h \
	pwd.h \
	shellwords.h \
	tasks.h \
	timer.h \
	tmspec.h \
	types.h \
	xstr.h

noinst_HEADERS =

lib_LTLIBRARIES = libaugutil.la

libaugutil_la_SOURCES = \
	config.c \
	conv.c \
	event.c \
	file.c \
	getopt.c \
	hires.c \
	lexer.c \
	list.c \
	log.c \
	md5.c \
	networds.c \
	object.c \
	path.c \
	pwd.c \
	shellwords.c \
	tasks.c \
	timer.c \
	tmspec.c \
	types.c \
	xstr.c

libaugutil_la_LDFLAGS = \
	-no-undefined \
	-version-info 1:0:0

libaugutil_la_LIBADD = \
	$(top_builddir)/src/c/augctx/libaugctx.la \
	$(top_builddir)/src/c/augsys/libaugsys.la

check_PROGRAMS = \
	taskstest
# 	eventtest
# 	hirestest
# 	networdstest
# 	shelllexertest
# 	shellwordstest

TESTS = \
	taskstest
# 	eventtest
# 	hirestest
# 	networdstest.sh
# 	shelllexertest.sh
# 	shellwordstest.sh

# eventtest_SOURCES = eventtest.c
# eventtest_LDADD = \
# 	$(top_builddir)/src/c/augutil/libaugutil.la

# hirestest_SOURCES = hirestest.c
# hirestest_LDADD = \
# 	$(top_builddir)/src/c/augutil/libaugutil.la

# networdstest_SOURCES = networdstest.c
# networdstest_LDADD = \
# 	$(top_builddir)/src/c/augutil/libaugutil.la

# shelllexertest_SOURCES = shelllexertest.c
# shelllexertest_LDADD = \
# 	$(top_builddir)/src/c/augutil/libaugutil.la

# shellwordstest_SOURCES = shellwordstest.c
# shellwordstest_LDADD = \
# 	$(top_builddir)/src/c/augutil/libaugutil.la

taskstest_SOURCES = taskstest.c
taskstest_LDADD = \
	$(top_builddir)/src/c/augutil/libaugutil.la
