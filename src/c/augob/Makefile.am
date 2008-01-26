EXTRA_DIST = \
	addrob.xml \
	blob.xml \
	errinfob.xml \
	eventob.xml \
	iterob.xml \
	longob.xml \
	rpcob.xml \
	seqob.xml

modincludedir = $(includedir)/augob
modinclude_HEADERS = \
	addrob.h \
	blob.h \
	errinfob.h \
	eventob.h \
	iterob.h \
	longob.h \
	rpcob.h \
	seqob.h \
	var.h

if ENABLE_RUBY
%.h: %.xml
	$(top_srcdir)/src/rb/aubidl -n $@ <$< >$@
endif
