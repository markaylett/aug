include $(top_srcdir)/src/c/lang.mk

moddir = $(prefix)/mod

if ENABLE_RUBY
RB_CFLAGS=`$(top_srcdir)/src/rb/rbflags -c`
RB_LDFLAGS=`$(top_srcdir)/src/rb/rbflags -l`
AM_CFLAGS = $(RB_CFLAGS)

noinst_HEADERS =

mod_LTLIBRARIES = modaugrb.la
modaugrb_la_SOURCES = \
	augrb.c
modaugrb_la_LDFLAGS = -module -export-dynamic -avoid-version -no-undefined $(RB_LDFLAGS)
modaugrb_la_LIBADD =
endif
