include $(top_srcdir)/src/c/lang.mk

bin_PROGRAMS = htdigest

htdigest_SOURCES = \
	htdigest.c

htdigest_LDADD = \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augsys/libaugsys.la
