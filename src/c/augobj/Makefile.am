EXTRA_DIST = \
	addrob.xml \
	blob.xml \
	longob.xml

modincludedir = $(includedir)/augobj
modinclude_HEADERS = \
	addrob.h \
	blob.h \
	longob.h

if ENABLE_RUBY
%.h: %.xml
	$(top_srcdir)/src/rb/augidl -f addrob <$< >$@
endif
