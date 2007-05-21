include $(top_srcdir)/src/cpp/lang.mk

noinst_HEADERS = \
	buffer.hpp \
	clntconn.hpp \
	config.hpp \
	conn.hpp \
	listener.hpp \
	object.hpp \
	objects.hpp \
	rwtimer.hpp \
	serv.hpp \
	servs.hpp \
	servconn.hpp

noinst_LTLIBRARIES = libaugrtpp.la

libaugrtpp_la_SOURCES = \
	buffer.cpp \
	clntconn.cpp \
	config.cpp \
	conn.cpp \
	listener.cpp \
	object.cpp \
	objects.cpp \
	rwtimer.cpp \
	serv.cpp \
	servs.cpp \
	servconn.cpp

libaugrtpp_la_LDFLAGS = \
	-no-undefined

libaugrtpp_la_LIBADD = \
	$(top_builddir)/src/c/augrt/libaugrt.la
