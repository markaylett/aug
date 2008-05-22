include $(top_srcdir)/src/cpp/lang.mk

modincludedir = $(includedir)/augnetpp
modinclude_HEADERS = \
	base64.hpp \
	channels.hpp \
	config.hpp \
	http.hpp \
	inet.hpp \
	mar.hpp \
	object.hpp \
	tcpconnect.hpp \
	writer.hpp

noinst_LTLIBRARIES = libaugnetpp.la

libaugnetpp_la_SOURCES = \
	base64.cpp \
	channels.cpp \
	config.cpp \
	http.cpp \
	inet.cpp \
	mar.cpp \
	object.cpp \
	tcpconnect.cpp \
	writer.cpp

libaugnetpp_la_LDFLAGS = \
	-no-undefined

libaugnetpp_la_LIBADD = \
	$(top_builddir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_builddir)/src/c/augnet/libaugnet.la \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augsys/libaugsys.la

check_PROGRAMS = \
	base64test \
	channelstest \
	maintest \
	tcpconnecttest

TESTS = \
	base64test \
	channelstest \
	maintest \
	tcpconnecttest

base64test_SOURCES = base64test.cpp
base64test_LDADD = \
	$(top_builddir)/src/cpp/augutilpp/libaugutilpp.la \
	$(top_builddir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_builddir)/src/c/augnet/libaugnet.la \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augsys/libaugsys.la

channelstest_SOURCES = channelstest.cpp
channelstest_LDADD = \
	$(top_builddir)/src/cpp/augutilpp/libaugutilpp.la \
	$(top_builddir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_builddir)/src/c/augnet/libaugnet.la \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augsys/libaugsys.la

maintest_SOURCES = maintest.cpp
maintest_LDADD = \
	$(top_builddir)/src/cpp/augutilpp/libaugutilpp.la \
	$(top_builddir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_builddir)/src/c/augnet/libaugnet.la \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augsys/libaugsys.la

tcpconnecttest_SOURCES = tcpconnecttest.cpp
tcpconnecttest_LDADD = \
	$(top_builddir)/src/cpp/augutilpp/libaugutilpp.la \
	$(top_builddir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_builddir)/src/c/augnet/libaugnet.la \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augsys/libaugsys.la
