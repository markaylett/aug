EXTRA_DIST = \
	MSG00001.bin \
	augres.h \
	augres.mc \
	augres.rc \
	augres.reg

dist_bin_SCRIPTS = \
	augres.dll

etcdir = $(prefix)/etc
etc_DATA = augres.reg

uninstall-local:
	rm -f $(DESTDIR)$(bindir)/augres.dll \
	$(DESTDIR)$(etcdir)/augres.reg

build: augres.o
	dllwrap --dllname augres.dll --driver-name gcc augres.o

augres.o: augres.rc
	windres -o augres.o augres.rc

augres.rc: augres.mc
	mc augres.mc
