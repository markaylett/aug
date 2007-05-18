include $(top_srcdir)/src/cpp/lang.mk

noinst_HEADERS = \
	config.hpp \
	serv.hpp

noinst_LTLIBRARIES = libaugrtpp.la

libaugrtpp_la_SOURCES = \
	config.cpp \
	serv.cpp

libaugrtpp_la_LDFLAGS = \
	-no-undefined

libaugrtpp_la_LIBADD = \
	$(top_builddir)/src/c/augrt/libaugrt.la
