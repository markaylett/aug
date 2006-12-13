include $(top_srcdir)/src/cpp/lang.mk

modincludedir = $(includedir)/augnetpp
modinclude_HEADERS = \
	config.hpp \
	conn.hpp \
	inet.hpp

noinst_LTLIBRARIES = libaugnetpp.la

libaugnetpp_la_SOURCES = \
	config.cpp \
	conn.cpp \
	inet.cpp

libaugnetpp_la_LDFLAGS = \
	-no-undefined

libaugnetpp_la_LIBADD = \
	$(top_srcdir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_srcdir)/src/c/augnet/libaugnet.la \
	$(top_srcdir)/src/c/augutil/libaugutil.la \
	$(top_srcdir)/src/c/augsys/libaugsys.la

check_PROGRAMS = augnetpp-test
TESTS = augnetpp-test

augnetpp_test_SOURCES = test.cpp
augnetpp_test_LDADD = \
	$(top_srcdir)/src/cpp/augsrvpp/libaugsrvpp.la \
	$(top_srcdir)/src/cpp/augutilpp/libaugutilpp.la \
	$(top_srcdir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_srcdir)/src/c/augnet/libaugnet.la \
	$(top_srcdir)/src/c/augsrv/libaugsrv.la \
	$(top_srcdir)/src/c/augutil/libaugutil.la \
	$(top_srcdir)/src/c/augmar/libaugmar.la \
	$(top_srcdir)/src/c/augsys/libaugsys.la
