include $(top_srcdir)/src/cpp/lang.mk

modincludedir = $(includedir)/augaspp
modinclude_HEADERS = \
	buffer.hpp \
	clntconn.hpp \
	config.hpp \
	conn.hpp \
	engine.hpp \
	listener.hpp \
	object.hpp \
	rwtimer.hpp \
	servconn.hpp \
	session.hpp \
	sessions.hpp \
	ssl.hpp \
	sock.hpp \
	socks.hpp

lib_LTLIBRARIES = libaugaspp.la

libaugaspp_la_SOURCES = \
	buffer.cpp \
	clntconn.cpp \
	config.cpp \
	conn.cpp \
	engine.cpp \
	listener.cpp \
	object.cpp \
	rwtimer.cpp \
	servconn.cpp \
	session.cpp \
	sessions.cpp \
	ssl.cpp \
	sock.cpp \
	socks.cpp

libaugaspp_la_LDFLAGS = \
	-no-undefined \
	-version-info 1:0:0

libaugaspp_la_LIBADD = \
	$(top_builddir)/src/c/augnet/libaugnet.la \
	$(top_builddir)/src/c/augserv/libaugserv.la \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augmar/libaugmar.la \
	$(top_builddir)/src/c/augsys/libaugsys.la \
	$(top_builddir)/src/c/augctx/libaugctx.la
