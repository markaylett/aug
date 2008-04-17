include $(top_srcdir)/src/c/lang.mk

modincludedir = $(includedir)/augmar
modinclude_HEADERS = \
	config.h \
	mar.h \
	types.h

noinst_HEADERS = \
	body_.h \
	file_.h \
	flags_.h \
	format_.h \
	header_.h \
	info_.h \
	mfile_.h \
	seq_.h

lib_LTLIBRARIES = libaugmar.la

libaugmar_la_SOURCES = \
	config.c \
	mar.c \
	types.c \
	body_.c \
	file_.c \
	flags_.c \
	format_.c \
	header_.c \
	info_.c \
	mfile_.c \
	seq_.c

libaugmar_la_LDFLAGS = \
	-no-undefined \
	-version-info 1:0:0

libaugmar_la_LIBADD = \
	$(top_builddir)/src/c/augctx/libaugctx.la \
	$(top_builddir)/src/c/augsys/libaugsys.la

check_PROGRAMS = maintest
TESTS = maintest

maintest_SOURCES = maintest.c
maintest_LDADD = \
	$(top_builddir)/src/c/augmar/libaugmar.la
