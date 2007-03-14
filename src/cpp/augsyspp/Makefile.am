include $(top_srcdir)/src/cpp/lang.mk

modincludedir = $(includedir)/augsyspp
modinclude_HEADERS = \
	base.hpp \
	config.hpp \
	dlfcn.hpp \
	endpoint.hpp \
	errinfo.hpp \
	exception.hpp \
	inetaddr.hpp \
	lock.hpp \
	mcast.hpp \
	mmap.hpp \
	mplexer.hpp \
	mutex.hpp \
	null.hpp \
	smartfd.hpp \
	smartptr.hpp \
	socket.hpp \
	string.hpp \
	time.hpp \
	types.hpp \
	unistd.hpp \
	utility.hpp

noinst_LTLIBRARIES = libaugsyspp.la

libaugsyspp_la_SOURCES = \
	base.cpp \
	config.cpp \
	dlfcn.cpp \
	endpoint.cpp \
	errinfo.cpp \
	exception.cpp \
	inetaddr.cpp \
	lock.cpp \
	mcast.cpp \
	mmap.cpp \
	mplexer.cpp \
	mutex.cpp \
	null.cpp \
	smartfd.cpp \
	smartptr.cpp \
	socket.cpp \
	string.cpp \
	time.cpp \
	types.cpp \
	unistd.cpp \
	utility.cpp

libaugsyspp_la_LDFLAGS = \
	-no-undefined

libaugsyspp_la_LIBADD = \
	$(top_builddir)/src/c/augsys/libaugsys.la

check_PROGRAMS = \
	augsyspp-test \
	augsyspp-exceptiontest \
	augsyspp-mplexertest \
	augsyspp-smartptrtest

TESTS = \
	augsyspp-test \
	augsyspp-exceptiontest \
	augsyspp-mplexertest \
	augsyspp-smartptrtest

augsyspp_test_SOURCES = test.cpp
augsyspp_test_LDADD = \
	$(top_builddir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_builddir)/src/c/augsys/libaugsys.la

augsyspp_exceptiontest_SOURCES = exceptiontest.cpp
augsyspp_exceptiontest_LDADD = \
	$(top_builddir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_builddir)/src/c/augsys/libaugsys.la

augsyspp_mplexertest_SOURCES = mplexertest.cpp
augsyspp_mplexertest_LDADD = \
	$(top_builddir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_builddir)/src/c/augsys/libaugsys.la

augsyspp_smartptrtest_SOURCES = smartptrtest.cpp
augsyspp_smartptrtest_LDADD = \
	$(top_builddir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_builddir)/src/c/augsys/libaugsys.la
