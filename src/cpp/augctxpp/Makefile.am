include $(top_srcdir)/src/cpp/lang.mk

modincludedir = $(includedir)/augctxpp
modinclude_HEADERS = \
	base.hpp \
	config.hpp \
	errinfo.hpp \
	exception.hpp \
	types.hpp \
	utility.hpp

noinst_LTLIBRARIES = libaugctxpp.la

libaugctxpp_la_SOURCES = \
	base.cpp \
	config.cpp \
	errinfo.cpp \
	exception.cpp \
	types.cpp \
	utility.cpp

libaugctxpp_la_LDFLAGS = \
	-no-undefined

libaugctxpp_la_LIBADD = \
	$(top_builddir)/src/c/augctx/libaugctx.la

check_PROGRAMS = \
	exceptiontest \
	maintest

TESTS = \
	exceptiontest \
	maintest

exceptiontest_SOURCES = exceptiontest.cpp
exceptiontest_LDADD = \
	$(top_builddir)/src/cpp/augctxpp/libaugctxpp.la \
	$(top_builddir)/src/c/augctx/libaugctx.la

maintest_SOURCES = maintest.cpp
maintest_LDADD = \
	$(top_builddir)/src/cpp/augctxpp/libaugctxpp.la \
	$(top_builddir)/src/c/augctx/libaugctx.la
