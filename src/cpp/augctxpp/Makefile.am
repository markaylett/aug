include $(top_srcdir)/src/cpp/lang.mk

modincludedir = $(includedir)/augctxpp
modinclude_HEADERS = \
	base.hpp \
	config.hpp \
	errinfo.hpp \
	exception.hpp \
	mpool.hpp \
	types.hpp \
	utility.hpp

noinst_LTLIBRARIES = libaugctxpp.la

libaugctxpp_la_SOURCES = \
	base.cpp \
	config.cpp \
	errinfo.cpp \
	exception.cpp \
	mpool.cpp \
	types.cpp \
	utility.cpp

libaugctxpp_la_LDFLAGS = \
	-no-undefined

libaugctxpp_la_LIBADD = \
	$(top_builddir)/src/c/augctx/libaugctx.la

check_PROGRAMS = \
	basetest \
	errinfotest \
	exceptiontest \
	mpooltest

TESTS = \
	basetest \
	errinfotest \
	exceptiontest \
	mpooltest

basetest_SOURCES = basetest.cpp
basetest_LDADD = \
	$(top_builddir)/src/cpp/augctxpp/libaugctxpp.la \
	$(top_builddir)/src/c/augctx/libaugctx.la

errinfotest_SOURCES = errinfotest.cpp
errinfotest_LDADD = \
	$(top_builddir)/src/cpp/augctxpp/libaugctxpp.la \
	$(top_builddir)/src/c/augctx/libaugctx.la

exceptiontest_SOURCES = exceptiontest.cpp
exceptiontest_LDADD = \
	$(top_builddir)/src/cpp/augctxpp/libaugctxpp.la \
	$(top_builddir)/src/c/augctx/libaugctx.la

mpooltest_SOURCES = mpooltest.cpp
mpooltest_LDADD = \
	$(top_builddir)/src/cpp/augctxpp/libaugctxpp.la \
	$(top_builddir)/src/c/augctx/libaugctx.la
