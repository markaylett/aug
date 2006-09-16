include $(top_srcdir)/src/cpp/lang.mk

modincludedir = $(includedir)/augsyspp
modinclude_HEADERS = \
	base.hpp \
	config.hpp \
	endpoint.hpp \
	exception.hpp \
	inetaddr.hpp \
	mcast.hpp \
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

if ENABLE_CPPBUILD

noinst_LTLIBRARIES = libaugsyspp.la

libaugsyspp_la_SOURCES = \
	base.cpp \
	config.cpp \
	endpoint.cpp \
	exception.cpp \
	inetaddr.cpp \
	mcast.cpp \
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
	$(top_srcdir)/src/c/augsys/libaugsys.la

check_PROGRAMS = \
	augsyspp-test \
	augsyspp-smartptrtest

TESTS = \
	augsyspp-test \
	augsyspp-smartptrtest

augsyspp_test_SOURCES = test.cpp
augsyspp_test_LDADD = \
	$(top_srcdir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_srcdir)/src/c/augsys/libaugsys.la

augsyspp_smartptrtest_SOURCES = smartptrtest.cpp
augsyspp_smartptrtest_LDADD = \
	$(top_srcdir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_srcdir)/src/c/augsys/libaugsys.la

endif
