include $(top_srcdir)/src/cpp/lang.mk

EXTRA_DIST = \
	test.sh

SUBDIRS = \
	etc

noinst_HEADERS = \
	exception.hpp \
	module.hpp \
	options.hpp \
	session.hpp \
	ssl.hpp \
	utility.hpp

bin_PROGRAMS = daug

daug_SOURCES = \
	daug.cpp \
	exception.cpp \
	module.cpp \
	options.cpp \
	session.cpp \
	ssl.cpp \
	utility.cpp

daug_LDADD = \
	$(top_builddir)/src/cpp/augrtpp/libaugrtpp.la \
	$(top_builddir)/src/c/augnet/libaugnet.la \
	$(top_builddir)/src/c/augsrv/libaugsrv.la \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augmar/libaugmar.la \
	$(top_builddir)/src/c/augsys/libaugsys.la

check_PROGRAMS = \
	augrtpp-test

TESTS = \
	augrtpp-test

augrtpp_test_SOURCES = test.cpp
augrtpp_test_LDADD =
