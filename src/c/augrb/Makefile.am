include $(top_srcdir)/src/c/lang.mk

moddir = $(prefix)/mod

if WITH_RUBY
AM_CFLAGS = $(RB_CFLAGS)

noinst_HEADERS = \
	object.h

mod_LTLIBRARIES = modaugrb.la
modaugrb_la_SOURCES = \
	augrb.c \
	object.c
modaugrb_la_LDFLAGS = -module -export-dynamic -avoid-version -no-undefined $(RB_LDFLAGS)
modaugrb_la_LIBADD =
endif
