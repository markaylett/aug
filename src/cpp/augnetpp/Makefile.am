include $(top_srcdir)/src/cpp/lang.mk

modincludedir = $(includedir)/augnetpp
modinclude_HEADERS = \
	base64.hpp \
	config.hpp \
	connector.hpp \
	file.hpp \
	http.hpp \
	inet.hpp \
	mar.hpp \
	nbfile.hpp \
	writer.hpp

noinst_LTLIBRARIES = libaugnetpp.la

libaugnetpp_la_SOURCES = \
	base64.cpp \
	config.cpp \
	connector.cpp \
	file.cpp \
	http.cpp \
	inet.cpp \
	mar.cpp \
	nbfile.cpp \
	writer.cpp

libaugnetpp_la_LDFLAGS = \
	-no-undefined

libaugnetpp_la_LIBADD = \
	$(top_builddir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_builddir)/src/c/augnet/libaugnet.la \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augsys/libaugsys.la

check_PROGRAMS = \
	augnetpp-test \
	augnetpp-base64test \
	augnetpp-connectortest

TESTS = \
	augnetpp-test \
	augnetpp-base64test \
	augnetpp-connectortest

augnetpp_test_SOURCES = test.cpp
augnetpp_test_LDADD = \
	$(top_builddir)/src/cpp/augutilpp/libaugutilpp.la \
	$(top_builddir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_builddir)/src/c/augnet/libaugnet.la \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augsys/libaugsys.la

augnetpp_base64test_SOURCES = base64test.cpp
augnetpp_base64test_LDADD = \
	$(top_builddir)/src/cpp/augutilpp/libaugutilpp.la \
	$(top_builddir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_builddir)/src/c/augnet/libaugnet.la \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augsys/libaugsys.la

augnetpp_connectortest_SOURCES = connectortest.cpp
augnetpp_connectortest_LDADD = \
	$(top_builddir)/src/cpp/augutilpp/libaugutilpp.la \
	$(top_builddir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_builddir)/src/c/augnet/libaugnet.la \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augsys/libaugsys.la
