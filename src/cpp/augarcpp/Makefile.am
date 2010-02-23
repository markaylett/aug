include $(top_srcdir)/src/cpp/lang.mk

modincludedir = $(includedir)/augarcpp
modinclude_HEADERS = \
	base.hpp \
	config.hpp \
	exception.hpp \
	factory.hpp \
	inner.hpp \
	inverse.hpp \
	names.hpp \
	node.hpp \
	outer.hpp \
	query.hpp \
	recur.hpp \
	typed.hpp \
	types.hpp \
	utility.hpp \
	wild.hpp

noinst_LTLIBRARIES = libaugarcpp.la

libaugarcpp_la_SOURCES = \
	base.cpp \
	config.cpp \
	exception.cpp \
	factory.cpp \
	inner.cpp \
	inverse.cpp \
	names.cpp \
	node.cpp \
	outer.cpp \
	query.cpp \
	recur.cpp \
	typed.cpp \
	types.cpp \
	utility.cpp \
	wild.cpp

libaugarcpp_la_LDFLAGS = \
	-no-undefined

libaugarcpp_la_LIBADD = \
	$(top_builddir)/src/c/augctx/libaugctx.la
