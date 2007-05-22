include $(top_srcdir)/src/cpp/lang.mk

noinst_HEADERS = \
	buffer.hpp \
	clntconn.hpp \
	config.hpp \
	conn.hpp \
	engine.hpp \
	listener.hpp \
	object.hpp \
	rwtimer.hpp \
	serv.hpp \
	servs.hpp \
	servconn.hpp \
	ssl.hpp \
	sock.hpp \
	socks.hpp

noinst_LTLIBRARIES = libaugrtpp.la

libaugrtpp_la_SOURCES = \
	buffer.cpp \
	clntconn.cpp \
	config.cpp \
	conn.cpp \
	engine.cpp \
	listener.cpp \
	object.cpp \
	rwtimer.cpp \
	serv.cpp \
	servs.cpp \
	servconn.cpp \
	ssl.cpp \
	sock.cpp \
	socks.cpp

libaugrtpp_la_LDFLAGS = \
	-no-undefined

libaugrtpp_la_LIBADD = \
	$(top_builddir)/src/c/augrt/libaugrt.la
