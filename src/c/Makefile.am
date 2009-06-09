EXTRA_DIST = \
	augses.xml \
	lang.mk

SUBDIRS = \
	augext \
	augctx \
	augsys \
	augmar \
	augres \
	augutil \
	augserv \
	augnet \
	mar

include_HEADERS = \
	aug.h \
	augabi.h \
	augses.h \
	augmod.h \
	augtypes.h \
	augext.h \
	augctx.h \
	augsys.h \
	augmar.h \
	augutil.h \
	augserv.h \
	augnet.h

if WITH_RUBY
BUILT_SOURCES = \
	augses.h

%.h: %.xml
	$(top_srcdir)/src/rb/augidl -n $@ <$< >$@
endif
