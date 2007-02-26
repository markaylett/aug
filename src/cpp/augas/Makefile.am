include $(top_srcdir)/src/cpp/lang.mk

EXTRA_DIST = \
	test.sh

SUBDIRS = \
	etc

noinst_HEADERS = \
	buffer.hpp \
	clntconn.hpp \
	conn.hpp \
	exception.hpp \
	listener.hpp \
	manager.hpp \
	module.hpp \
	object.hpp \
	options.hpp \
	rwtimer.hpp \
	serv.hpp \
	servconn.hpp \
	utility.hpp

bin_PROGRAMS = augasd

augasd_SOURCES = \
	augasd.cpp \
	buffer.cpp \
	clntconn.cpp \
	conn.cpp \
	exception.cpp \
	listener.cpp \
	manager.cpp \
	module.cpp \
	object.cpp \
	options.cpp \
	rwtimer.cpp \
	servconn.cpp \
	serv.cpp \
	utility.cpp

augasd_LDADD = \
	$(top_builddir)/src/c/augnet/libaugnet.la \
	$(top_builddir)/src/c/augsrv/libaugsrv.la \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augmar/libaugmar.la \
	$(top_builddir)/src/c/augsys/libaugsys.la

check_PROGRAMS = \
	augaspp-test

TESTS = \
	augaspp-test

augaspp_test_SOURCES = test.cpp
augaspp_test_LDADD =
