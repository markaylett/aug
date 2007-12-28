EXTRA_DIST = \
	addrob.xml \
	blob.xml \
	eventob.xml \
	longob.xml \
	varrayob.xml

modincludedir = $(includedir)/augob
modinclude_HEADERS = \
	addrob.h \
	blob.h \
	eventob.h \
	longob.h \
	types.h \
	varrayob.h

if ENABLE_RUBY
%.h: %.xml
	$(top_srcdir)/src/rb/aubidl -f $@ <$< >$@
endif
